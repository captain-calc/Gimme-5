#include <sys/lcd.h>
#include <sys/timers.h>
#include <assert.h>
#include <string.h>

#include "ccdbg/ccdbg.h"
#include "gfx/gfx.h"
#include "gameplay.h"
#include "gameplay_anagram.h"
#include "gui.h"
#include "ingamehelp.h"
#include "keypad.h"



// ============================================================================
// DEFINITIONS
// ============================================================================


#define NUM_ANAGRAMS (163)
static const word_string_t ANAGRAM_LIST[NUM_ANAGRAMS] = {
  {'B', 'A', 'S', 'T', 'E', }, {'B', 'A', 'T', 'E', 'S', },
  {'B', 'E', 'A', 'S', 'T', }, {'B', 'E', 'A', 'T', 'S', },
  {'B', 'E', 'T', 'A', 'S', }, {'B', 'A', 'L', 'E', 'R', },
  {'B', 'L', 'A', 'R', 'E', }, {'A', 'L', 'T', 'E', 'R', },
  {'L', 'A', 'T', 'E', 'R', }, {'A', 'N', 'G', 'L', 'E', },
  {'G', 'L', 'E', 'A', 'N', }, {'G', 'R', 'O', 'A', 'N', },
  {'O', 'R', 'G', 'A', 'N', }, {'C', 'O', 'A', 'S', 'T', },
  {'C', 'O', 'A', 'T', 'S', }, {'T', 'A', 'C', 'O', 'S', },
  {'N', 'A', 'P', 'E', 'S', }, {'N', 'E', 'A', 'P', 'S', },
  {'P', 'A', 'N', 'E', 'S', }, {'B', 'L', 'A', 'S', 'E', },
  {'S', 'A', 'B', 'L', 'E', }, {'B', 'E', 'A', 'R', 'D', },
  {'B', 'R', 'E', 'A', 'D', }, {'R', 'O', 'B', 'E', 'S', },
  {'S', 'O', 'B', 'E', 'R', }, {'C', 'A', 'R', 'E', 'D', },
  {'C', 'E', 'D', 'A', 'R', }, {'R', 'A', 'C', 'E', 'D', },
  {'L', 'A', 'C', 'K', 'S', }, {'S', 'L', 'A', 'C', 'K', },
  {'C', 'A', 'T', 'E', 'R', }, {'C', 'R', 'A', 'T', 'E', },
  {'R', 'E', 'A', 'C', 'T', }, {'T', 'R', 'A', 'C', 'E', },
  {'C', 'L', 'A', 'S', 'P', }, {'S', 'C', 'A', 'L', 'P', },
  {'C', 'O', 'L', 'D', 'S', }, {'S', 'C', 'O', 'L', 'D', },
  {'C', 'O', 'R', 'E', 'D', }, {'C', 'R', 'E', 'D', 'O', },
  {'D', 'E', 'C', 'O', 'R', }, {'C', 'O', 'R', 'S', 'E', },
  {'S', 'C', 'O', 'R', 'E', }, {'L', 'U', 'C', 'R', 'E', },
  {'U', 'L', 'C', 'E', 'R', }, {'C', 'U', 'T', 'E', 'R', },
  {'T', 'R', 'U', 'C', 'E', }, {'D', 'I', 'E', 'T', 'S', },
  {'E', 'D', 'I', 'T', 'S', }, {'T', 'I', 'D', 'E', 'S', },
  {'P', 'A', 'D', 'R', 'E', }, {'P', 'A', 'R', 'E', 'D', },
  {'R', 'I', 'D', 'E', 'S', }, {'S', 'I', 'R', 'E', 'D', },
  {'N', 'A', 'R', 'E', 'S', }, {'N', 'E', 'A', 'R', 'S', },
  {'S', 'N', 'A', 'R', 'E', }, {'H', 'A', 'T', 'E', 'R', },
  {'H', 'E', 'A', 'R', 'T', }, {'F', 'E', 'A', 'R', 'S', },
  {'S', 'A', 'F', 'E', 'R', }, {'F', 'R', 'I', 'E', 'S', },
  {'F', 'R', 'I', 'S', 'E', }, {'S', 'E', 'R', 'I', 'F', },
  {'F', 'R', 'E', 'R', 'E', }, {'R', 'E', 'F', 'E', 'R', },
  {'L', 'A', 'G', 'E', 'R', }, {'L', 'A', 'R', 'G', 'E', },
  {'R', 'E', 'G', 'A', 'L', }, {'H', 'E', 'A', 'R', 'S', },
  {'S', 'H', 'A', 'R', 'E', }, {'S', 'H', 'E', 'A', 'R', },
  {'P', 'H', 'A', 'S', 'E', }, {'S', 'H', 'A', 'P', 'E', },
  {'L', 'I', 'D', 'O', 'S', }, {'S', 'O', 'L', 'I', 'D', },
  {'S', 'T', 'I', 'L', 'E', }, {'T', 'I', 'L', 'E', 'S', },
  {'L', 'I', 'N', 'K', 'S', }, {'S', 'L', 'I', 'N', 'K', },
  {'L', 'I', 'A', 'R', 'S', }, {'R', 'A', 'I', 'L', 'S', },
  {'P', 'A', 'L', 'M', 'S', }, {'P', 'S', 'A', 'L', 'M', },
  {'L', 'E', 'A', 'P', 'S', }, {'P', 'A', 'L', 'E', 'S', },
  {'P', 'L', 'E', 'A', 'S', }, {'S', 'A', 'L', 'T', 'S', },
  {'S', 'L', 'A', 'T', 'S', }, {'S', 'L', 'A', 'T', 'E', },
  {'S', 'T', 'A', 'L', 'E', }, {'S', 'T', 'E', 'A', 'L', },
  {'T', 'A', 'L', 'E', 'S', }, {'T', 'E', 'A', 'L', 'S', },
  {'S', 'T', 'I', 'L', 'L', }, {'T', 'I', 'L', 'L', 'S', },
  {'M', 'I', 'L', 'E', 'S', }, {'S', 'L', 'I', 'M', 'E', },
  {'S', 'M', 'I', 'L', 'E', }, {'S', 'I', 'L', 'T', 'S', },
  {'S', 'L', 'I', 'T', 'S', }, {'P', 'O', 'O', 'L', 'S', },
  {'S', 'P', 'O', 'O', 'L', }, {'S', 'T', 'O', 'O', 'L', },
  {'T', 'O', 'O', 'L', 'S', }, {'S', 'O', 'L', 'V', 'E', },
  {'V', 'O', 'L', 'E', 'S', }, {'M', 'A', 'N', 'S', 'E', },
  {'M', 'E', 'A', 'N', 'S', }, {'N', 'A', 'M', 'E', 'S', },
  {'M', 'E', 'A', 'T', 'S', }, {'S', 'T', 'E', 'A', 'M', },
  {'T', 'A', 'M', 'E', 'S', }, {'T', 'E', 'A', 'M', 'S', },
  {'M', 'I', 'T', 'E', 'R', }, {'M', 'I', 'T', 'R', 'E', },
  {'R', 'E', 'M', 'I', 'T', }, {'T', 'I', 'M', 'E', 'R', },
  {'O', 'N', 'S', 'E', 'T', }, {'S', 'T', 'O', 'N', 'E', },
  {'T', 'O', 'N', 'E', 'S', }, {'T', 'R', 'O', 'V', 'E', },
  {'V', 'O', 'T', 'E', 'R', }, {'P', 'L', 'A', 'Y', 'S', },
  {'S', 'P', 'L', 'A', 'Y', }, {'P', 'E', 'N', 'A', 'L', },
  {'P', 'L', 'A', 'N', 'E', }, {'P', 'E', 'A', 'R', 'S', },
  {'R', 'E', 'A', 'P', 'S', }, {'S', 'P', 'A', 'R', 'E', },
  {'S', 'P', 'E', 'A', 'R', }, {'S', 'T', 'R', 'A', 'P', },
  {'T', 'A', 'R', 'P', 'S', }, {'T', 'R', 'A', 'P', 'S', },
  {'P', 'R', 'A', 'T', 'E', }, {'T', 'A', 'P', 'E', 'R', },
  {'P', 'L', 'A', 'T', 'E', }, {'P', 'L', 'E', 'A', 'T', },
  {'P', 'I', 'N', 'T', 'O', }, {'P', 'I', 'T', 'O', 'N', },
  {'P', 'O', 'I', 'N', 'T', }, {'P', 'O', 'S', 'E', 'R', },
  {'P', 'R', 'O', 'S', 'E', }, {'R', 'O', 'P', 'E', 'S', },
  {'S', 'P', 'O', 'R', 'E', }, {'S', 'P', 'O', 'T', 'S', },
  {'S', 'T', 'O', 'P', 'S', }, {'T', 'R', 'A', 'D', 'E', },
  {'T', 'R', 'E', 'A', 'D', }, {'S', 'T', 'E', 'E', 'R', },
  {'T', 'E', 'R', 'S', 'E', }, {'T', 'R', 'E', 'E', 'S', },
  {'T', 'I', 'E', 'R', 'S', }, {'T', 'I', 'R', 'E', 'S', },
  {'T', 'R', 'I', 'E', 'S', }, {'S', 'A', 'T', 'I', 'N', },
  {'S', 'T', 'A', 'I', 'N', }, {'S', 'E', 'V', 'E', 'R', },
  {'V', 'E', 'E', 'R', 'S', }, {'V', 'E', 'R', 'S', 'E', },
  {'S', 'T', 'A', 'K', 'E', }, {'S', 'T', 'E', 'A', 'K', },
  {'T', 'A', 'K', 'E', 'S', }
};


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static void load_random_anagram_into_word(OUT Word& word);


// ============================================================================
// CLASS FUNCTION DEFINITIONS
// ============================================================================


AnagramGameplay::AnagramGameplay(): Gameplay(ID)
{
  return;
}


void AnagramGameplay::play_random_word()
{
  Word guess;
  word_string_t input = { '\0' };
  char letter;
  uint8_t num_letters = 0;
  bool transition_in = true;
  bool full_redraw = true;
  bool partial_redraw = false;
  bool guess_submitted = false;
  uint8_t pause_menu_decision = NO_DECISION;

  gui_TransitionOut();
  reset_guesses();
  pick_next_target_word();

  if (InGameHelp::must_show_help_for(InGameHelp::ANAGRAM_GAMEPLAY))
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

      if (pause_menu_decision == QUIT_GAME)
        break;

      if (pause_menu_decision == SHOW_WORD)
        guess_submitted = true;

      pause_menu_decision = NO_DECISION;
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
        guess_submitted = true;
      }
      else
      {
        word_not_in_dictionary_notification();
        full_redraw = true;
      }
    }

    if (are_all_rounds_complete())
    {
      results_screen();
      break;
    }

    if (full_redraw)
      draw_game_screen_background();

    if (partial_redraw || full_redraw)
    {
      draw_game_screen_foreground(input);
      partial_redraw = false;
    }

    if (transition_in)
    {
      gui_TransitionIn();
      transition_in = false;
    }

    gfx_BlitBuffer();
    full_redraw = false;

    if (guess_submitted)
    {
      add_guess(input);
      memset(input, '\0', WORD_LENGTH);
      num_letters = 0;

      if (guess_submitted && is_guess_correct())
      {
        this->guess_was_correct[this->num_rounds_completed - 1] = true;
        draw_guess_correct_animation();
      }
      else
      {
        this->guess_was_correct[this->num_rounds_completed - 1] = false;
        draw_guess_incorrect_animation();
      }

      full_redraw = true;
      transition_in = true;
      guess_submitted = false;

      pick_next_target_word();
    }
  }

  gui_TransitionOut();
  return;
}


void AnagramGameplay::draw_game_screen_background() const
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

  gfx_SetColor(DARK_MED_BLUE);
  gfx_FillRectangle_NoClip(33, 0, 254, LCD_HEIGHT);
  return;
}


static void draw_icon(
  IN char character, IN evaluation_code_t evaluation_code, IN point_t& origin
)
{
  GuiText text;
  gfx_sprite_t* sprite;

  switch (evaluation_code)
  {
    case LETTER_CORRECT:
      sprite = spr_character_tile_orange;
      break;

    case POSITION_AND_LETTER_CORRECT:
      sprite = spr_character_tile_green;
      break;

    case UNEVALUATED:
    case POSITION_AND_LETTER_INCORRECT:
    default:
      sprite = spr_character_tile_blue;
      break;
  }

  gfx_SetColor(BLACK);
  gfx_HorizLine(origin.xpos + 1, origin.ypos + sprite->height, sprite->width);
  gfx_VertLine(origin.xpos + sprite->width, origin.ypos + 1, sprite->height);
  gfx_Sprite_NoClip(sprite, origin.xpos, origin.ypos);
  text.set_font(GuiText::TILE_CHARACTER);
  text.set_xpos(origin.xpos + ((20 - gfx_GetCharWidth(character)) / 2));
  text.set_ypos(origin.ypos + 3);
  text.draw_character(character);
  return;
}


void AnagramGameplay::draw_guess_correctness_icons() const
{
  point_t origin = {
    .xpos = (uint24_t)(LCD_WIDTH - spr_character_slot->width - 10),
    .ypos = 10
  };

  for (uint8_t index = 0; index < this->MAX_NUM_ROUNDS; index++)
  {
    if (index < this->num_rounds_completed)
    {
      if (this->guess_was_correct[index])
        draw_icon('1' + index, POSITION_AND_LETTER_CORRECT, origin);
      else
        draw_icon('1' + index, LETTER_CORRECT, origin);
    }
    else
    {
      draw_icon('1' + index, UNEVALUATED, origin);
    }

    origin.ypos += spr_character_slot->height + 5;
  }

  return;
}


void AnagramGameplay::draw_game_screen_foreground(
  IN word_string_t current_guess
) const
{
  GuiText text;
  word_string_t target_string;

  gfx_SetColor(DARK_MED_BLUE);
  gfx_FillRectangle_NoClip(33, 0, 254, 200);

  this->target.copy_into_string(target_string);
  text.set_font(GuiText::DOUBLE_SIZE_WITH_SHADOW);
  text.set_ypos(60);
  text.draw_centered_string(target_string);
  draw_word_input(current_guess);
  draw_guess_correctness_icons();
  return;
}


void AnagramGameplay::draw_guess_correct_animation() const
{
  const uint24_t RECTANGLE_WIDTH = 200;
  const uint24_t RECTANGLE_XPOS = (LCD_WIDTH - RECTANGLE_WIDTH) / 2;

  GuiText text;

  gfx_SetColor(GREEN);
  gfx_FillRectangle_NoClip(RECTANGLE_XPOS, 0, RECTANGLE_WIDTH, LCD_HEIGHT);

  text.set_font(GuiText::GAME_COMPLETION);
  text.set_ypos(100);
  text.draw_centered_string("CORRECT!");
  gui_TransitionIn();

  delay(1000);
  return;
}


void AnagramGameplay::draw_guess_incorrect_animation() const
{
  const uint24_t RECTANGLE_WIDTH = 200;
  const uint24_t RECTANGLE_XPOS = (LCD_WIDTH - RECTANGLE_WIDTH) / 2;

  GuiText text;

  gfx_SetColor(DARK_ORANGE);
  gfx_FillRectangle_NoClip(RECTANGLE_XPOS, 0, RECTANGLE_WIDTH, LCD_HEIGHT);

  text.set_font(GuiText::GAME_COMPLETION);
  text.set_ypos(100);
  text.draw_centered_string("WRONG!");
  gui_TransitionIn();

  delay(1000);
  return;
}


bool AnagramGameplay::is_guess_correct() const
{
  Word guess;

  if (this->num_rounds_completed == 0)
    return false;

  guess = this->guesses[this->num_rounds_completed - 1];

  if (guess != this->target && this->target.is_anagram_of_word(guess))
    return true;

  return false;
}


bool AnagramGameplay::is_guess_incorrect() const
{
  return !is_guess_correct();
}


bool AnagramGameplay::are_all_rounds_complete() const
{
  return (this->num_rounds_completed == this->MAX_NUM_ROUNDS);
}


void AnagramGameplay::show_help_screen() const
{
  const uint8_t NUM_STRINGS_ON_FIRST_PAGE = 10;
  const char* FIRST_PAGE_STRINGS[NUM_STRINGS_ON_FIRST_PAGE] = {
    "Gameplay:",
    "",
    "This game's objective is to create anagrams",
    "of given words. Each game consists of five",
    "rounds. Every round you will be given a",
    "random five-letter word. You must enter a",
    "valid anagram of the given word to win.",
    "",
    "Note: Some given words may have more than",
    "one valid anagram."
  };
  const uint8_t NUM_STRINGS_ON_SECOND_PAGE = 13;
  const char* SECOND_PAGE_STRINGS[NUM_STRINGS_ON_SECOND_PAGE] = {
    "Your progress is tracked on the right side",
    "of the screen using numeric tiles. A green",
    "tile indicates a correct answer and an",
    "orange tile indicates a wrong answer.",
    "",
    "Controls:",
    "  [2nd]/[enter]  . . . . . . Enter guess",
    "  [del]  . . . . . . . . . . . . . . . . Delete last letter",
    "  [clear] . . . . . . . . . . . . . . Pause game",
    "",
    "Use the the buttons associated with the",
    "green A-Z letters to enter a five-letter",
    "word."
  };
  const uint8_t NUM_PAGES = 2;

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
        gui_DrawHelpScreen(FIRST_PAGE_STRINGS, NUM_STRINGS_ON_FIRST_PAGE);
        break;

      case 2:
        gui_DrawHelpScreen(SECOND_PAGE_STRINGS, NUM_STRINGS_ON_SECOND_PAGE);
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


void AnagramGameplay::reset_guesses()
{
  Word empty_word;
  this->num_rounds_completed = 0;

  for (uint8_t index = 0; index < this->MAX_NUM_ROUNDS; index++)
  {
    this->guesses[index] = empty_word;
    this->guess_was_correct[index] = false;
  }

  return;
}


void AnagramGameplay::add_guess(IN word_string_t guess)
{
  if (this->num_rounds_completed < this->MAX_NUM_ROUNDS)
  {
    this->guesses[this->num_rounds_completed] = guess;
    this->num_rounds_completed++;
  }

  return;
}


void AnagramGameplay::pick_next_target_word()
{
  Word next_target;

  do {
    load_random_anagram_into_word(next_target);
  } while (next_target == this->target);

  this->target = next_target;
  return;
}


void AnagramGameplay::word_not_in_dictionary_notification()
{
  Notification notification("Word not in dictionary!");
  notification.draw();

  while (true)
  {
    Keypad::update_state();

    if (Keypad::was_any_key_released())
      break;

    notification.blit();
  }

  return;
}


void AnagramGameplay::results_screen()
{
  const uint24_t HORIZONTAL_SPACING = 5;
  const uint24_t ICON_SEQUENCE_WIDTH = (
    (5 * spr_character_slot->width) + (4 * HORIZONTAL_SPACING)
  );
  GuiText text;

  draw_game_screen_background();

  text.set_font(GuiText::DOUBLE_SIZE_WITH_SHADOW);
  text.set_ypos(40);
  text.draw_centered_string("GAME");
  text.set_ypos(70);
  text.draw_centered_string("COMPLETE!");

  point_t origin = {
    .xpos = (uint24_t)((LCD_WIDTH / 2) - (ICON_SEQUENCE_WIDTH / 2)),
    .ypos = 120
  };

  for (uint8_t index = 0; index < this->MAX_NUM_ROUNDS; index++)
  {
    if (index < this->num_rounds_completed)
    {
      if (this->guess_was_correct[index])
        draw_icon('1' + index, POSITION_AND_LETTER_CORRECT, origin);
      else
        draw_icon('1' + index, LETTER_CORRECT, origin);
    }
    else
    {
      draw_icon('1' + index, UNEVALUATED, origin);
    }

    origin.xpos += spr_character_slot->width + HORIZONTAL_SPACING;
  }

  delay(500);
  gui_TransitionIn();
  Keypad::block_until_any_key_released();
  return;
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static void load_random_anagram_into_word(OUT Word& word)
{
  word = ANAGRAM_LIST[rand() % NUM_ANAGRAMS];
  return;
}
