#include <sys/lcd.h>
#include <ti/screen.h>
#include <graphx.h>
#include <time.h>

#include "gfx/gfx.h"
#include "dictionary.h"
#include "gameplay_original.h"
#include "gameplay_rush.h"
#include "gui.h"
#include "ingamehelp.h"
#include "keypad.h"


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static bool ensure_dictionary_is_accessible();
static int main_menu();
static void show_main_menu_help();
static void word_code_entry_menu();
static void draw_word_code_entry_menu(
  IN char* user_input, MOD bool& full_redraw
);
static void invalid_code_notification();
static void show_word_code_entry_menu_help();
static void gameplay_menu();


// ============================================================================
// MAIN FUNCTION
// ============================================================================


int main(void)
{
  int exit_code;

  if (!ensure_dictionary_is_accessible())
    return 1;

  srand(time(NULL));

  gui_StartGraphics();
  gui_DrawCheckeredBackground();
  gfx_BlitBuffer();
  exit_code = main_menu();
  gui_EndGraphics();

  return exit_code;
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static bool ensure_dictionary_is_accessible()
{
  if (!dictionary_IsAppvarPresent())
  {
    os_ClrHomeFull();
    os_PutStrFull("The dictionary appvar:");
    os_NewLine();
    os_PutStrFull(DICTIONARY_APPVAR);
    os_PutStrFull(".8xv, cannot be");
    os_NewLine();
    os_PutStrFull("cannot opened/found.");
    os_NewLine();
    os_NewLine();
    os_PutStrFull("Transfer the dictionary");
    os_NewLine();
    os_PutStrFull("appvar to your calculator.");
    Keypad::block_until_any_key_released();
    return false;
  }

  if (!dictionary_ArchiveAppvar())
  {
    os_ClrHomeFull();
    os_PutStrFull("The dictionary appvar");
    os_NewLine();
    os_PutStrFull(DICTIONARY_APPVAR);
    os_PutStrFull("could not be archived.");
    os_NewLine();
    os_NewLine();
    os_PutStrFull("If your Archive is full,");
    os_NewLine();
    os_PutStrFull("try deleting some");
    os_NewLine();
    os_PutStrFull("archived files and try");
    os_NewLine();
    os_PutStrFull("again.");
    Keypad::block_until_any_key_released();
    return false;
  }

  return true;
}


static int main_menu()
{
  const uint8_t NUM_OPTIONS = 2;
  const char* BUTTON_TITLES[] = { "Play Game", "Enter Code" };

  uint8_t option_index = 0;
  uint8_t button_ypos;
  bool full_redraw = true;

  if (InGameHelp::must_show_help_for(InGameHelp::MAIN_MENU))
    show_main_menu_help();

  while (true)
  {
    Keypad::update_state();

    if (Keypad::is_down_repeating(kb_KeyUp))
    {
      if (option_index > 0)
        option_index--;
      else
        option_index = NUM_OPTIONS - 1;
    }
    else if (Keypad::is_down_repeating(kb_KeyDown))
    {
      if (option_index + 1 < NUM_OPTIONS)
        option_index++;
      else
        option_index = 0;
    }

    else if (Keypad::was_down_exclusive(kb_KeyClear))
    {
      break;
    }
    else if (Keypad::was_released_exclusive(kb_KeyMode))
    {
      gui_TransitionOut();
      show_main_menu_help();
      full_redraw = true;
    }
    else if (
      Keypad::was_released_exclusive(kb_Key2nd)
      || Keypad::was_released_exclusive(kb_KeyEnter)
    )
    {
      if (option_index == 0)
      {
        gameplay_menu();
        full_redraw = true;
      }
      else if (option_index == 1)
      {
        word_code_entry_menu();
        full_redraw = true;
      }
      else
      {
        break;
      }
    }

    if (full_redraw)
    {
      gui_DrawCheckeredBackground();
      gfx_ScaledTransparentSprite_NoClip(
        spr_logo, (LCD_WIDTH / 2) - (1.5 * spr_logo->width), 5, 3, 2
      );
      gfx_ScaledTransparentSprite_NoClip(
        spr_all_stars_logo,
        (LCD_WIDTH / 2) - spr_all_stars_logo->width,
        94,
        2,
        1
      );
    }

    button_ypos = 145;

    for (uint8_t index = 0; index < NUM_OPTIONS; index++)
    {
      if (option_index == index)
        gui_DrawButtonSelected(BUTTON_TITLES[index], button_ypos);
      else
        gui_DrawButton(BUTTON_TITLES[index], button_ypos);

      button_ypos += 35;
    }

    if (full_redraw)
    {
      gui_TransitionIn();
      full_redraw = false;
    }
    else
    {
      gfx_SwapDraw();
    }
  }

  return 0;
}


static void show_main_menu_help()
{
  const uint8_t NUM_STRINGS_ON_FIRST_PAGE = 13;
  const char* FIRST_PAGE_STRINGS[NUM_STRINGS_ON_FIRST_PAGE] = {
    "Thank you for downloading Gimme 5: All Stars!",
    "This help menu will give you a quick tour of",
    "how to navigate the program.",
    "",
    "Help Menu Controls",
    "  [left]/[right] . . . . . . . . . Flip pages",
    "  [clear] . . . . . . . . . . . . . . Close help menu",
    "",
    "Press [mode] at any time to access a help",
    "menu. Press [clear] to exit menus and",
    "games.",
    "",
    "Press [right] to go to the next page."
  };
  const uint8_t NUM_STRINGS_ON_SECOND_PAGE = 11;
  const char* SECOND_PAGE_STRINGS[NUM_STRINGS_ON_SECOND_PAGE] = {
    "Main Menu Controls:",
    "  [up]/[down] . . . . . . . . . Change selected button",
    "  [2nd]/[enter]  . . . . . . Press button",
    "  [clear] . . . . . . . . . . . . . . Exit menu",
    "",
    "",
    "Author:  Captain Calc",
    "Version:  Pre-Release Beta",
    "",
    "",
    "Have fun!"
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
        gui_DrawHelpScreen(FIRST_PAGE_STRINGS, NUM_STRINGS_ON_FIRST_PAGE, 67);
        break;

      case 2:
        gui_DrawHelpScreen(
          SECOND_PAGE_STRINGS, NUM_STRINGS_ON_SECOND_PAGE, 67
        );
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


static void word_code_entry_menu()
{
  OriginalGameplay original_gameplay;
  char input[20] = { '\0' };
  char letter;
  uint8_t num_characters = 0;
  uint8_t max_num_characters = original_gameplay.ENCRYPTED_CODE_LENGTH;
  bool transition_in = true;
  bool full_redraw = true;

  gui_TransitionOut();

  if (InGameHelp::must_show_help_for(InGameHelp::WORD_CODE_ENTRY_MENU))
    show_word_code_entry_menu_help();

  while (true)
  {
    Keypad::update_state();

    if (
      keypad_GetHexAsciiCharacter(letter)
      && num_characters < max_num_characters
    )
    {
      input[num_characters] = letter;
      num_characters++;
    }
    else if (Keypad::is_down_repeating(kb_KeyDel) && num_characters > 0)
    {
      num_characters--;
      input[num_characters] = '\0';
    }
    else if (
      Keypad::was_released_exclusive(kb_Key2nd)
      || Keypad::was_released_exclusive(kb_KeyEnter)
    )
    {
      if (original_gameplay.is_valid_code(input))
      {
        original_gameplay.play_valid_word_code(input);
        break;
      }
      else
      {
        invalid_code_notification();
        full_redraw = true;
      }
    }
    else if (Keypad::was_released_exclusive(kb_KeyMode))
    {
      gui_TransitionOut();
      show_word_code_entry_menu_help();
      full_redraw = true;
      transition_in = true;
    }
    else if (Keypad::was_released_exclusive(kb_KeyClear))
    {
      break;
    }

    draw_word_code_entry_menu(input, full_redraw);

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


static void draw_word_code_entry_menu(
  IN char* user_input, MOD bool& full_redraw
)
{
  const uint8_t CONTAINER_YPOS = 96;
  const uint8_t CONTAINER_HEIGHT = 75;

  GuiText text;
  Container code_input_container;
  InputField input_field(CONTAINER_YPOS + CONTAINER_HEIGHT - 27, 180);

  code_input_container.set_ypos(CONTAINER_YPOS);
  code_input_container.set_width(256);
  code_input_container.set_height(CONTAINER_HEIGHT);
  code_input_container.center_horizontally_on_screen();

  if (full_redraw)
  {
    gui_DrawCheckeredBackground();
    gui_DrawMenuTitle("Word Code Entry");
    code_input_container.draw();

    text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);
    text.set_ypos(CONTAINER_YPOS + 10);
    text.draw_centered_string("Enter a word code into");
    text.set_ypos(CONTAINER_YPOS + 21);
    text.draw_centered_string("the field below.");

    full_redraw = false;
  }

  input_field.draw_input(user_input);
  return;
}


static void invalid_code_notification()
{
  Notification notification("Invalid code!");

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


static void show_word_code_entry_menu_help()
{
  const uint8_t NUM_STRINGS = 12;
  const char* STRINGS[NUM_STRINGS] = {
    "Controls:",
    "  [2nd]/[enter]  . . . . . . Enter code",
    "  [del]  . . . . . . . . . . . . . . . . Delete last character",
    "  [clear] . . . . . . . . . . . . . . Exit menu",
    "",
    "Use the numeric keypad and the buttons",
    "associated with the green A-F letters to",
    "enter the 12-character word code.",
    "",
    "If you get an \"Invalid code\" error,",
    "verify that you have entered the code",
    "correctly."
  };

  gui_DrawHelpScreen(STRINGS, NUM_STRINGS, 67);
  gui_TransitionIn();
  Keypad::block_until_any_key_released();
  gui_TransitionOut();
  return;
}


static void gameplay_menu()
{
  OriginalGameplay original_gameplay;
  RushGameplay rush_gameplay;
  AnagramGameplay anagram_gameplay;
  Gameplay* gameplay_options[] = {
    &rush_gameplay, &original_gameplay, &anagram_gameplay
  };
  const uint8_t NUM_OPTIONS = 3;
  uint8_t option_index = 1;
  bool transition_in = true;
  point_t origin;

  gui_TransitionOut();

  while (true)
  {
    Keypad::update_state();

    if (Keypad::was_released_exclusive(kb_KeyClear))
    {
      break;
    }
    else if (Keypad::is_down_repeating(kb_KeyLeft))
    {
      if (option_index > 0)
        option_index--;
      else
        option_index = NUM_OPTIONS - 1;
    }
    else if (Keypad::is_down_repeating(kb_KeyRight))
    {
      if (option_index + 1 < NUM_OPTIONS)
        option_index++;
      else
        option_index = 0;
    }
    else if (
      Keypad::was_released_exclusive(kb_Key2nd)
      || Keypad::was_released_exclusive(kb_KeyEnter)
    )
    {
      gameplay_options[option_index]->play_random_word();
      transition_in = true;
    }

    gui_DrawCheckeredBackground();
    gui_DrawMenuTitle("Gameplay");

    origin.xpos = (LCD_WIDTH / 6) + 2;
    origin.ypos = 72;

    if (option_index == 0)
    {
      origin.ypos = 66;
      gui_DrawGameplayOptionSelected("Rush", origin);
    }
    else
      gui_DrawGameplayOption("Rush", origin);

    gfx_ScaledTransparentSprite_NoClip(
      spr_gameplay_rush_icon,
      origin.xpos - spr_gameplay_rush_icon->width,
      origin.ypos + 32,
      2,
      2
    );

    origin.xpos = LCD_WIDTH / 2;
    origin.ypos = 72;

    if (option_index == 1)
    {
      origin.ypos = 66;
      gui_DrawGameplayOptionSelected("Original", origin);
    }
    else
      gui_DrawGameplayOption("Original", origin);

    gfx_ScaledTransparentSprite_NoClip(
      spr_gameplay_original_icon,
      origin.xpos - spr_gameplay_original_icon->width,
      origin.ypos + 20,
      2,
      2
    );

    origin.xpos = ((LCD_WIDTH / 6) * 5) - 2;
    origin.ypos = 72;

    if (option_index == 2)
    {
      origin.ypos = 66;
      gui_DrawGameplayOptionSelected("Anagram-7", origin);
    }
    else
      gui_DrawGameplayOption("Anagram-7", origin);

    gfx_ScaledTransparentSprite_NoClip(
      spr_gameplay_anagram_seven_icon,
      origin.xpos - spr_gameplay_anagram_seven_icon->width + 1,
      origin.ypos + 22,
      2,
      2
    );

    if (transition_in)
    {
      gui_TransitionIn();
      transition_in = false;
    }
    else
      gfx_BlitBuffer();
  }

  gui_TransitionOut();
  return;
}
