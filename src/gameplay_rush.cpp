#include <sys/lcd.h>
#include <sys/timers.h>
#include <assert.h>
#include <debug.h>
#include <string.h>
#include <time.h>

#include "ccdbg/ccdbg.h"
#include "gfx/gfx.h"
#include "gameplay_rush.h"
#include "graphx.h"
#include "gui.h"
#include "ingamehelp.h"
#include "keypad.h"


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


Timer::Timer()
{
  reload();
  return;
}


void Timer::update()
{
  clock_t timestamp = clock();

  if ((timestamp - this->start_timestamp) > CLOCKS_PER_SEC)
  {
    if (this->num_seconds_left > 0)
      this->num_seconds_left--;

    this->start_timestamp = timestamp;
  }

  return;
}


void Timer::reload()
{
  this->start_timestamp = clock();
  this->num_seconds_left = RELOAD_VALUE;
  return;
}


bool Timer::is_expired() const
{
  return (this->num_seconds_left == 0);
}


void Timer::draw() const
{
  GuiText text;
  Container container;

  container.set_xpos(this->XPOS);
  container.set_ypos(this->YPOS);
  container.set_width(this->WIDTH);
  container.set_height(this->HEIGHT);
  container.set_color(BLUE);
  container.set_z_index(2);
  container.draw();

  text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);
  text.set_xpos(container.get_xpos() + 8);
  text.set_ypos(container.get_ypos() + 6);
  text.draw_unsigned_int(this->num_seconds_left);
  return;
}


void Timer::blit() const
{
  gfx_BlitRectangle(
    gfx_buffer, this->XPOS, this->YPOS, this->WIDTH + 2, this->HEIGHT + 2
  );
  return;
}


RushGameplay::RushGameplay(): Gameplay(ID)
{
  return;
}


void RushGameplay::play_random_word()
{
  Word guess;
  word_string_t input = { '\0' };
  char letter;
  uint8_t num_letters = 0;
  bool transition_in = true;
  bool full_redraw = true;
  bool partial_redraw = false;
  uint8_t pause_menu_decision = NO_DECISION;

  gui_TransitionOut();
  reset_guesses();
  dictionary.get_random_word(target);

  // TODO: Add help for RushGameplay and update in-game help flags.
  if (InGameHelp::must_show_help_for(InGameHelp::GAME))
    show_help_screen();

  while (true)
  {
    Keypad::update_state();

    if (Keypad::was_released_exclusive(kb_KeyMode))
    {
      gui_TransitionOut();
      show_help_screen();
      full_redraw = true;
      transition_in = true;
    }
    else if (Keypad::was_released_exclusive(kb_KeyClear))
    {
      pause_menu_decision = pause_menu();
      full_redraw = true;
    }
    else if (
      keypad_GetUppercaseAsciiLetter(letter) && num_letters < WORD_LENGTH
    )
    {
      input[num_letters] = letter;
      num_letters++;
      partial_redraw = true;
    }
    else if (Keypad::is_down_repeating(kb_KeyDel) && num_letters > 0)
    {
      num_letters--;
      input[num_letters] = '\0';
      partial_redraw = true;
    }
    else if (
      Keypad::was_released_exclusive(kb_Key2nd)
      || Keypad::was_released_exclusive(kb_KeyEnter)
    )
    {
      guess = input;

      if (dictionary.contains_word(guess))
      {
        add_guess(input);
        memset(input, '\0', WORD_LENGTH);
        num_letters = 0;
        partial_redraw = true;
      }
      else
      {
        word_not_in_dictionary_notification();
        full_redraw = true;
      }
    }

    this->timer.update();

    if (this->timer.is_expired())
    {
      swap_target_word();
      this->timer.reload();
    }

    if (full_redraw)
      draw_game_screen_background();

    if (partial_redraw || full_redraw)
    {
      draw_game_screen_foreground(input);
      partial_redraw = false;
    }

    this->timer.draw();

    if (transition_in)
    {
      gui_TransitionIn();
      transition_in = false;
    }

    gfx_BlitBuffer();
    full_redraw = false;

    if (is_last_guess_correct())
    {
      draw_win_animation();
      Keypad::block_until_any_key_released();
      //draw_results_screen();
      //Keypad::block_until_any_key_released();
      break;
    }
    else if (are_all_guesses_used() || (pause_menu_decision & REVEAL))
    {
      draw_lose_animation();
      Keypad::block_until_any_key_released();
      //draw_results_screen();
      //Keypad::block_until_any_key_released();
      break;
    }

    if (pause_menu_decision & QUIT_GAME)
      break;
  }

  gui_TransitionOut();
  return;
}


void RushGameplay::draw_game_screen_background() const
{
  gui_DrawCheckeredBackground();

  gfx_SetColor(WHITE);
  gfx_VertLine(32, 0, LCD_HEIGHT);
  gfx_VertLine(288, 0, LCD_HEIGHT);
  gfx_SetColor(BLACK);
  gfx_VertLine(30, 0, LCD_HEIGHT);
  gfx_VertLine(31, 0, LCD_HEIGHT);
  gfx_VertLine(289, 0, LCD_HEIGHT);
  gfx_VertLine(290, 0, LCD_HEIGHT);

  gfx_SetColor(BG_COLOR);
  gfx_FillRectangle_NoClip(33, 0, 254, LCD_HEIGHT);
  return;
}


void RushGameplay::draw_game_screen_foreground(
  IN word_string_t current_guess
) const
{
  draw_guesses();
  draw_word_input(current_guess);
  return;
}


void RushGameplay::draw_guesses() const
{
  const uint8_t VERTICAL_SPACING = (spr_character_slot->height + 5);
  const uint8_t STARTING_INDEX = (
    num_guesses > NUM_VISIBLE_GUESSES ? num_guesses - NUM_VISIBLE_GUESSES : 0
  );
  const uint8_t UPPER_LIMIT = STARTING_INDEX + NUM_VISIBLE_GUESSES;

  point_t origin = {
    .xpos = (LCD_WIDTH / 2),
    .ypos = 15
  };

  for (uint8_t index = STARTING_INDEX; index < UPPER_LIMIT; index++)
  {
    draw_evaluated_guess(
      guesses[index], guess_evaluations[index], origin
    );

    origin.ypos += VERTICAL_SPACING;
  }

  return;
}


void RushGameplay::draw_win_animation() const
{
  const uint24_t RECTANGLE_WIDTH = 200;
  const uint24_t RECTANGLE_XPOS = (LCD_WIDTH - RECTANGLE_WIDTH) / 2;

  GuiText text;
  word_string_t empty_string = { '\0' };

  gfx_SetColor(GREEN);
  gfx_FillRectangle_NoClip(RECTANGLE_XPOS, 0, RECTANGLE_WIDTH, LCD_HEIGHT);

  text.set_font(GuiText::GAME_COMPLETION);
  text.set_ypos(100);
  text.draw_centered_string("YOU");
  text.set_ypos(124);
  text.draw_centered_string("WIN!");
  delay(1000);
  gui_TransitionIn();

  delay(1500);
  draw_game_screen_background();
  draw_game_screen_foreground(empty_string);
  gui_TransitionIn();

  return;
}


void RushGameplay::draw_lose_animation() const
{
  const uint24_t RECTANGLE_WIDTH = 200;
  const uint24_t RECTANGLE_XPOS = (LCD_WIDTH - RECTANGLE_WIDTH) / 2;

  GuiText text;
  word_string_t target_string;

  gfx_SetColor(DARK_ORANGE);
  gfx_FillRectangle_NoClip(RECTANGLE_XPOS, 0, RECTANGLE_WIDTH, LCD_HEIGHT);

  text.set_font(GuiText::GAME_COMPLETION);
  text.set_ypos(100);
  text.draw_centered_string("GAME");
  text.set_ypos(124);
  text.draw_centered_string("OVER!");
  gui_TransitionIn();

  delay(1500);
  draw_game_screen_background();
  target.copy_into_string(target_string);
  draw_game_screen_foreground(target_string);
  gui_TransitionIn();

  return;
}


bool RushGameplay::is_last_guess_correct() const
{
  if (num_guesses == 0)
    return false;

  if (word_DoesEvaluationShowAllCorrect(guess_evaluations[num_guesses - 1]))
    return true;

  return false;
}


bool RushGameplay::are_all_guesses_used() const
{
  if (num_guesses == MAX_NUM_GUESSES)
    return true;

  return false;
}


void RushGameplay::show_help_screen() const
{
  // TODO
  return;
}


void RushGameplay::reset_guesses()
{
  Word empty_word;
  num_guesses = 0;

  for (uint8_t index = 0; index < MAX_NUM_GUESSES; index++)
  {
    guesses[index] = empty_word;
    memset(guess_evaluations[index], UNEVALUATED, sizeof(word_evaluation_t));
  }

  return;
}


void RushGameplay::add_guess(IN word_string_t guess)
{
  if (num_guesses < MAX_NUM_GUESSES)
  {
    guesses[num_guesses] = guess;
    word_EvaluateFirstWordBySecondWord(
      guess, target, guess_evaluations[num_guesses]
    );
    num_guesses++;
  }

  return;
}


void RushGameplay::swap_target_word()
{
  WordPattern pattern(target, guess_evaluations[num_guesses - 1]);

  if (!pattern.is_all_wildcards())
      dictionary.get_random_word_that_fits_pattern(pattern, target);

  return;
}


void RushGameplay::word_not_in_dictionary_notification()
{
  Notification notification("Word not in dictionary!");

  notification.draw();

  while (true)
  {
    Keypad::update_state();

    if (Keypad::was_any_key_released())
      break;

    this->timer.update();

    if (this->timer.is_expired())
    {
      swap_target_word();
      this->timer.reload();
    }

    notification.blit();
    this->timer.draw();
    this->timer.blit();
  }

  return;
}
