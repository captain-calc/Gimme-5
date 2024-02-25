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
  this->frozen = false;
  reload();
  return;
}


void Timer::update()
{
  clock_t timestamp = clock();

  if (this->frozen)
    return;

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


void Timer::freeze_updates()
{
  this->frozen = true;
  return;
}


void Timer::thaw_updates()
{
  this->frozen = false;
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

  if (InGameHelp::must_show_help_for(InGameHelp::RUSH_GAMEPLAY))
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

    if (last_guess_had_four_correctly_positioned_letters())
      this->timer.freeze_updates();
    else
      this->timer.thaw_updates();

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


bool RushGameplay::last_guess_had_four_correctly_positioned_letters() const
{
  word_evaluation_t last_guess_evaluation;
  uint8_t count = 0;

  memcpy(
    last_guess_evaluation,
    this->guess_evaluations[(this->num_guesses ? this->num_guesses - 1: 0)],
    sizeof(word_evaluation_t)
  );

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (last_guess_evaluation[index] == POSITION_AND_LETTER_CORRECT)
      count++;
  }

  return (count >= 4);
}


void RushGameplay::show_help_screen() const
{
  const uint8_t NUM_STRINGS_ON_FIRST_PAGE = 9;
  const char* FIRST_PAGE_STRINGS[NUM_STRINGS_ON_FIRST_PAGE] = {
    "Gameplay:",
    "",
    "The object of the game is to guess a",
    "five-letter English word by entering other",
    "five-letter English words.",
    "",
    "The letters of each entered guess are",
    "then colored to indicate if that letter",
    "appears in the target word."
  };
  const uint8_t NUM_STRINGS_ON_SECOND_PAGE = 9;
  const char* SECOND_PAGE_STRINGS[NUM_STRINGS_ON_SECOND_PAGE] = {
    "Colors:",
    "  BLUE:        Letter is not in the target word.",
    "  ORANGE:   Letter is in the target word but",
    "                      in a different position.",
    "  GREEN:      Letter is in the target word in",
    "                      that position.",
    "",
    "In short, the rules of Original apply to Rush,",
    "but with one twist."
  };
  const uint8_t NUM_STRINGS_ON_THIRD_PAGE = 13;
  const char* THIRD_PAGE_STRINGS[NUM_STRINGS_ON_THIRD_PAGE] = {
    "The Rush:",
    "",
    "Every 30 seconds, the game will change the",
    "target word. However, if your last guess had",
    "one or more GREEN letters in it, the game will",
    "pick a word with those letters and in those",
    "positions.",
    "",
    "BEWARE! If you leave out a GREEN letter, the",
    "target word no longer has to have that letter",
    "in it.",
    "",
    "(continued on next page)"
  };
  const uint8_t NUM_STRINGS_ON_FOURTH_PAGE = 11;
  const char* FOURTH_PAGE_STRINGS[NUM_STRINGS_ON_FOURTH_PAGE] = {
    "When you get four GREEN letters, the timer",
    "will stop.",
    "",
    "Controls:",
    "  [2nd]/[enter]  . . . . . . Enter guess",
    "  [del]  . . . . . . . . . . . . . . . . Delete last letter",
    "  [clear] . . . . . . . . . . . . . . Pause game",
    "",
    "Use the the buttons associated with the",
    "green A-Z letters to enter a five-letter",
    "word.",
  };
  const uint8_t NUM_PAGES = 4;

  bool transition_in = true;
  uint8_t page_num = 1;

  while (true)
  {
    Keypad::update_state();

    if (Keypad::is_down_repeating(kb_KeyLeft) && page_num > 1)
      page_num--;

    if (Keypad::is_down_repeating(kb_KeyRight) && page_num < NUM_PAGES)
      page_num++;

    if (Keypad::was_released_exclusive(kb_KeyClear))
      break;

    switch (page_num)
    {
      case 1:
        gui_DrawHelpScreen(FIRST_PAGE_STRINGS, NUM_STRINGS_ON_FIRST_PAGE, 67);
        break;

      case 2:
        gui_DrawHelpScreen(
          SECOND_PAGE_STRINGS, NUM_STRINGS_ON_SECOND_PAGE, 67
        );
        break;

      case 3:
        gui_DrawHelpScreen(THIRD_PAGE_STRINGS, NUM_STRINGS_ON_THIRD_PAGE, 67);
        break;

      case 4:
        gui_DrawHelpScreen(FOURTH_PAGE_STRINGS, NUM_STRINGS_ON_FOURTH_PAGE, 67);
        break;
    };

    gui_DrawPageNumberIndicator(NUM_PAGES, page_num);

    if (transition_in)
    {
      gui_TransitionIn();
      transition_in = false;
    }
    else
    {
      gfx_BlitBuffer();
    }
  }

  gui_TransitionOut();
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
