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


static const char* ANAGRAM_LIST[163] = {
  "BASTE",
  "BATES",
  "BEAST",
  "BEATS",
  "BETAS",
  "BALER",
  "BLARE",
  "ALTER",
  "LATER",
  "ANGLE",
  "GLEAN",
  "GROAN",
  "ORGAN",
  "COAST",
  "COATS",
  "TACOS",
  "NAPES",
  "NEAPS",
  "PANES",
  "BLASE",
  "SABLE",
  "BEARD",
  "BREAD",
  "ROBES",
  "SOBER",
  "CARED",
  "CEDAR",
  "RACED",
  "LACKS",
  "SLACK",
  "CATER",
  "CRATE",
  "REACT",
  "TRACE",
  "CLASP",
  "SCALP",
  "COLDS",
  "SCOLD",
  "CORED",
  "CREDO",
  "DECOR",
  "CORSE",
  "SCORE",
  "LUCRE",
  "ULCER",
  "CUTER",
  "TRUCE",
  "DIETS",
  "EDITS",
  "TIDES",
  "PADRE",
  "PARED",
  "RIDES",
  "SIRED",
  "NARES",
  "NEARS",
  "SNARE",
  "HATER",
  "HEART",
  "FEARS",
  "SAFER",
  "FRIES",
  "FRISE",
  "SERIF",
  "FRERE",
  "REFER",
  "LAGER",
  "LARGE",
  "REGAL",
  "HEARS",
  "SHARE",
  "SHEAR",
  "PHASE",
  "SHAPE",
  "LIDOS",
  "SOLID",
  "STILE",
  "TILES",
  "LINKS",
  "SLINK",
  "LIARS",
  "RAILS",
  "PALMS",
  "PSALM",
  "LEAPS",
  "PALES",
  "PLEAS",
  "SALTS",
  "SLATS",
  "SLATE",
  "STALE",
  "STEAL",
  "TALES",
  "TEALS",
  "STILL",
  "TILLS",
  "MILES",
  "SLIME",
  "SMILE",
  "SILTS",
  "SLITS",
  "POOLS",
  "SPOOL",
  "STOOL",
  "TOOLS",
  "SOLVE",
  "VOLES",
  "MANSE",
  "MEANS",
  "NAMES",
  "MEATS",
  "STEAM",
  "TAMES",
  "TEAMS",
  "MITER",
  "MITRE",
  "REMIT",
  "TIMER",
  "ONSET",
  "STONE",
  "TONES",
  "TROVE",
  "VOTER",
  "PLAYS",
  "SPLAY",
  "PENAL",
  "PLANE",
  "PEARS",
  "REAPS",
  "SPARE",
  "SPEAR",
  "STRAP",
  "TARPS",
  "TRAPS",
  "PRATE",
  "TAPER",
  "PLATE",
  "PLEAT",
  "PINTO",
  "PITON",
  "POINT",
  "POSER",
  "PROSE",
  "ROPES",
  "SPORE",
  "SPOTS",
  "STOPS",
  "TRADE",
  "TREAD",
  "STEER",
  "TERSE",
  "TREES",
  "TIERS",
  "TIRES",
  "TRIES",
  "SATIN",
  "STAIN",
  "SEVER",
  "VEERS",
  "VERSE",
  "STAKE",
  "STEAK",
  "TAKES"
};


AnagramCache::AnagramCache()
{
  this->num_anagrams = 163;

  for (uint8_t index = 0; index < this->MAX_NUM_ANAGRAMS; index++)
  {
    memcpy(
      this->anagram_list[index], ANAGRAM_LIST[index], sizeof(word_string_t)
    );
  }

  return;
}


void AnagramCache::load_random_anagram_into_word(OUT Word& word) const
{
  word = this->anagram_list[rand() % this->MAX_NUM_ANAGRAMS];
  return;
}


void AnagramCache::load_cache()
{
  return;
}


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
  anagram_cache.load_cache();
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
    anagram_cache.load_random_anagram_into_word(next_target);
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

  gui_TransitionOut();
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

  gui_TransitionIn();
  Keypad::block_until_any_key_released();
  return;
}
