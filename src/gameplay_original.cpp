#include <sys/lcd.h>
#include <sys/timers.h>
#include <assert.h>
#include <debug.h>
#include <string.h>

#include "ccdbg/ccdbg.h"
#include "gfx/gfx.h"
#include "gameplay_original.h"
#include "gui.h"
#include "ingamehelp.h"
#include "keypad.h"


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static char low_nibble_to_ascii_hex_char(IN uint8_t low_nibble);
static void byte_to_hex_pair(IN char character, OUT char hex_pair[2]);
static uint8_t ascii_hex_pair_to_byte(IN char ascii_hex_pair[2]);


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


OriginalGameplay::OriginalGameplay(): Gameplay(ID)
{
  return;
}


bool OriginalGameplay::is_valid_code(IN char* code) const
{
  Word decrypted_word;

  if (strlen(code) != ENCRYPTED_CODE_LENGTH)
    return false;

  decrypt_word_code_into_word(code, decrypted_word);

  if (dictionary.contains_word(decrypted_word))
    return true;

  return false;
}


void OriginalGameplay::play_random_word()
{
  Word random_word;

  if (target.is_empty())
    dictionary.get_random_word(random_word);
  else
    dictionary.get_random_word_unlike_given_word(target, random_word);

  play(random_word);
  return;
}


void OriginalGameplay::play_valid_word_code(IN char* code)
{
  Word decrypted_word;

  decrypt_word_code_into_word(code, decrypted_word);
  play(decrypted_word);
  return;
}


void OriginalGameplay::draw_game_screen_background() const
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


void OriginalGameplay::draw_game_screen_foreground(
  IN word_string_t current_guess
) const
{
  draw_guesses();
  draw_word_input(current_guess);
  alphabet_tracker.draw();
  return;
}


void OriginalGameplay::draw_guesses() const
{
  point_t origin = {
    .xpos = (LCD_WIDTH / 2),
    .ypos = 15
  };

  for (uint8_t index = 0; index < MAX_NUM_GUESSES; index++)
  {
    draw_evaluated_guess(
      guesses[index], guess_evaluations[index], origin
    );

    origin.ypos += spr_character_slot->height + 5;
  }

  return;
}


void OriginalGameplay::draw_win_animation() const
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


void OriginalGameplay::draw_lose_animation() const
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


void OriginalGameplay::draw_word_code_screen() const
{
  GuiText text;
  char buffer[ENCRYPTED_CODE_LENGTH + 1] = { '\0' };
  point_t origin = {
    .xpos = (LCD_WIDTH / 2),
    .ypos = 15
  };

  draw_game_screen_background();

  for (uint8_t index = 0; index < MAX_NUM_GUESSES; index++)
  {
    draw_guess_evaluation(guess_evaluations[index], origin);
    origin.ypos += spr_character_slot->height + 5;
  }

  generate_code(buffer);

  text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);
  text.set_ypos(165);
  text.draw_centered_string("Word Code:");
  text.set_font(GuiText::DOUBLE_SIZE_WITH_SHADOW);
  text.set_ypos(180);
  text.draw_centered_string(buffer);

  gfx_BlitBuffer();
  return;
}


bool OriginalGameplay::is_last_guess_correct() const
{
  if (num_guesses == 0)
    return false;

  if (word_DoesEvaluationShowAllCorrect(guess_evaluations[num_guesses - 1]))
    return true;

  return false;
}


bool OriginalGameplay::are_all_guesses_used() const
{
  if (num_guesses == MAX_NUM_GUESSES)
    return true;

  return false;
}


void OriginalGameplay::generate_code(OUT char* code) const
{
  const uint8_t ENCRYPTION_KEY_INDEX = rand() % NUM_ENCRYPTION_KEYS;
  word_string_t encryption_key;
  word_string_t target_string;

  memcpy(
    encryption_key,
    ENCRYPTION_KEYS[ENCRYPTION_KEY_INDEX],
    WORD_LENGTH
  );
  target.copy_into_string(target_string);

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    byte_to_hex_pair(
      target_string[index] ^ encryption_key[index],
      code + 2 + (index * 2)
    );
  }

  code[0] = low_nibble_to_ascii_hex_char(get_id());
  code[1] = low_nibble_to_ascii_hex_char(ENCRYPTION_KEY_INDEX);
  return;
}


void OriginalGameplay::decrypt_word_code_into_word(
  IN char* code, OUT Word& word
) const
{
  uint8_t encryption_key_index = ascii_hex_pair_to_byte(code) % 16;
  word_string_t encryption_key;
  word_string_t word_string;
  char byte;

  memcpy(
    encryption_key,
    ENCRYPTION_KEYS[encryption_key_index],
    WORD_LENGTH
  );

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    byte = (char)ascii_hex_pair_to_byte(code + 2 + (2 * index));
    byte = byte ^ encryption_key[index];
    word_string[index] = byte;
  }

  word = word_string;

char word_string_buffer[7] = { '\0' };
memcpy(word_string_buffer, word_string, WORD_LENGTH);
CCDBG_PUTS(word_string_buffer);

  return;
}


void OriginalGameplay::show_help_screen() const
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
    "Letters that are not in the target word",
    "will be removed from the onscreen alphabet."
  };
  const uint8_t NUM_STRINGS_ON_THIRD_PAGE = 8;
  const char* THIRD_PAGE_STRINGS[NUM_STRINGS_ON_THIRD_PAGE] = {
    "Controls:",
    "  [2nd]/[enter]  . . . . . . Enter guess",
    "  [del]  . . . . . . . . . . . . . . . . Delete last letter",
    "  [clear] . . . . . . . . . . . . . . Pause game",
    "",
    "Use the the buttons associated with the",
    "green A-Z letters to enter a five-letter",
    "word."
  };
  const uint8_t NUM_PAGES = 3;

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


void OriginalGameplay::word_not_in_dictionary_notification() const
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


void OriginalGameplay::play(IN Word& target_word)
{
  Word guess;
  word_string_t input = { '\0' };
  char letter;
  uint8_t num_letters = 0;
  bool transition_in = true;
  bool full_redraw = true;
  uint8_t pause_menu_decision = NO_DECISION;

  gui_TransitionOut();
  reset_guesses();
  target = target_word;

  if (InGameHelp::must_show_help_for(InGameHelp::ORIGINAL_GAMEPLAY))
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
    }
    else if (Keypad::is_down_repeating(kb_KeyDel) && num_letters > 0)
    {
      num_letters--;
      input[num_letters] = '\0';
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
      }
      else
      {
        gui_Notification("Not a dictionary word!");
        full_redraw = true;
      }
    }

    if (full_redraw)
    {
      draw_game_screen_background();
      full_redraw = false;
    }

    draw_game_screen_foreground(input);

    if (transition_in)
    {
      gui_TransitionIn();
      transition_in = false;
    }

    gfx_BlitBuffer();

    if (is_last_guess_correct())
    {
      draw_win_animation();
      Keypad::block_until_any_key_released();
      draw_word_code_screen();
      Keypad::block_until_any_key_released();
      break;
    }
    else if (are_all_guesses_used() || (pause_menu_decision & REVEAL))
    {
      draw_lose_animation();
      Keypad::block_until_any_key_released();
      draw_word_code_screen();
      Keypad::block_until_any_key_released();
      break;
    }

    if (pause_menu_decision & QUIT_GAME)
      break;
  }

  gui_TransitionOut();
  return;
}


void OriginalGameplay::reset_guesses()
{
  Word empty_word;
  num_guesses = 0;

  for (uint8_t index = 0; index < MAX_NUM_GUESSES; index++)
  {
    guesses[index] = empty_word;
    memset(guess_evaluations[index], UNEVALUATED, sizeof(word_evaluation_t));
  }

  alphabet_tracker.initialize();
  return;
}


void OriginalGameplay::add_guess(IN word_string_t guess)
{
  if (num_guesses < MAX_NUM_GUESSES)
  {
    guesses[num_guesses] = guess;
    word_EvaluateFirstWordBySecondWord(
      guess, target, guess_evaluations[num_guesses]
    );
    alphabet_tracker.include_guess(
      guesses[num_guesses], guess_evaluations[num_guesses]
    );
    num_guesses++;
  }

  return;
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static char low_nibble_to_ascii_hex_char(IN uint8_t low_nibble)
{
  const char* ASCII_HEX_CHARACTERS = { "0123456789ABCDEF" };

  return ASCII_HEX_CHARACTERS[low_nibble];
}


static void byte_to_hex_pair(IN char character, OUT char hex_pair[2])
{
  hex_pair[0] = low_nibble_to_ascii_hex_char(character / 16);
  hex_pair[1] = low_nibble_to_ascii_hex_char(character % 16);
  return;
}


static uint8_t ascii_hex_pair_to_byte(IN char ascii_hex_pair[2])
{
  uint8_t byte = 0;

  if (ascii_hex_pair[0] >= 'A')
    byte = 16 * (ascii_hex_pair[0] - 'A' + 10);
  else
    byte = 16 * (ascii_hex_pair[0] - '0');

  if (ascii_hex_pair[1] >= 'A')
    byte += (ascii_hex_pair[1] - 'A' + 10);
  else
    byte += (ascii_hex_pair[1] - '0');

  return byte;
}
