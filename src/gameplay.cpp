#include <sys/lcd.h>
#include <sys/timers.h>
#include <assert.h>
#include <debug.h>
#include <keypadc.h>
#include <string.h>
#include <time.h>

#include "ccdbg/ccdbg.h"
#include "gfx/gfx.h"
#include "gameplay.h"
#include "gui.h"
#include "ingamehelp.h"
#include "keypad.h"


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static void draw_character_tile(
  IN evaluation_code_t evaluation_code, IN point_t& origin
);
static void draw_character_and_tile(
  IN char character, IN evaluation_code_t evaluation_code, IN point_t& origin
);


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


AlphabetTracker::AlphabetTracker()
{
  initialize();
  return;
}


void AlphabetTracker::initialize()
{
  memset(letters_not_in_target, '\0', 26);
  return;
}


void AlphabetTracker::include_guess(
  IN Word& guess, IN word_evaluation_t evaluation
)
{
  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (evaluation[index] == POSITION_AND_LETTER_INCORRECT)
    {
      letters_not_in_target[guess[index] - 'A'] = guess[index];
    }
  }

  return;
}


void AlphabetTracker::draw() const
{
  const uint24_t HORIZONTAL_SPACING = 8;
  const uint24_t LETTER_WIDTH = 8;
  const uint8_t LETTERS_PER_LINE = 13;
  const uint24_t LINE_WIDTH = (
    LETTERS_PER_LINE * (LETTER_WIDTH + HORIZONTAL_SPACING)
  ) - HORIZONTAL_SPACING;

  uint24_t xpos = (LCD_WIDTH - LINE_WIDTH) / 2;
  uint8_t ypos = 160;
  char letter;

  DecoratedRectangle rectangle;
  GuiText text;

  rectangle.set_ypos(ypos - 5);
  rectangle.set_width(LINE_WIDTH + 30);
  rectangle.set_height((2 * GFX_DEFAULT_FONT_HEIGHT) + 6 + 10);
  rectangle.center_horizontally_on_screen();
  rectangle.set_color(DARK_MED_BLUE);
  rectangle.set_border_color(WHITE);
  rectangle.set_border_radius(6);
  rectangle.set_border_thickness(1);
  rectangle.draw();

  text.set_font(GuiText::DEFAULT);
  text.set_ypos(ypos);

  for (uint8_t index = 0; index < 26; index++)
  {
    if (index == LETTERS_PER_LINE)
    {
      xpos = (LCD_WIDTH - LINE_WIDTH) / 2;
      ypos += GFX_DEFAULT_FONT_HEIGHT + 6;
      text.set_ypos(ypos);
    }

    letter = 'A' + index;

    if (letters_not_in_target[index] != letter)
    {
      text.set_xpos(xpos);
      text.draw_character(letter);
    }

    xpos += (LETTER_WIDTH + HORIZONTAL_SPACING);
  }

  return;
}


Gameplay::Gameplay(uint8_t id): id(id)
{
  return;
}


uint8_t Gameplay::get_id() const
{
  return id;
}


void Gameplay::draw_word_input(IN word_string_t word) const
{
  const uint24_t HORIZONTAL_SPACING = 6;
  const uint24_t INPUT_WIDTH = (
    (WORD_LENGTH * (spr_character_slot->width + HORIZONTAL_SPACING))
    - HORIZONTAL_SPACING
  );

  point_t tile_origin = {
    .xpos = (LCD_WIDTH - INPUT_WIDTH) / 2,
    .ypos = 205
  };
  char letter;

  gfx_SetColor(DARK_MED_BLUE);
  gfx_FillRectangle_NoClip(
    tile_origin.xpos,
    tile_origin.ypos,
    INPUT_WIDTH + 1,
    spr_character_slot->height + 1
  );

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    letter = word[index];

    if (letter == '\0')
    {
      gfx_Sprite_NoClip(
        spr_character_slot, tile_origin.xpos, tile_origin.ypos
      );
    }
    else
    {
      draw_character_and_tile(letter, UNEVALUATED, tile_origin);
    }

    gfx_SetColor(WHITE);
    gfx_FillRectangle_NoClip(
      tile_origin.xpos,
      tile_origin.ypos + spr_character_slot->height + 2,
      spr_character_slot->width,
      2
    );

    tile_origin.xpos += spr_character_slot->width + HORIZONTAL_SPACING;
  }
  return;
}


void Gameplay::draw_guess_evaluation(
  IN word_evaluation_t evaluation, IN point_t& origin
) const
{
  const uint24_t HORIZONTAL_SPACING = 2;
  const uint24_t WIDTH = (
    (WORD_LENGTH * (spr_character_slot->width + HORIZONTAL_SPACING))
    - HORIZONTAL_SPACING
  );

  assert(origin.xpos >= (WIDTH / 2));

  point_t tile_origin = {
    .xpos = origin.xpos - (WIDTH / 2),
    .ypos = origin.ypos
  };

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    draw_character_tile(evaluation[index], tile_origin);
    tile_origin.xpos += spr_character_slot->width + HORIZONTAL_SPACING;
  }

  return;
}


void Gameplay::draw_evaluated_guess(
  IN Word& word, IN word_evaluation_t evaluation, IN point_t& origin
) const
{
  const uint24_t HORIZONTAL_SPACING = 2;
  const uint24_t WIDTH = (
    (WORD_LENGTH * (spr_character_slot->width + HORIZONTAL_SPACING))
    - HORIZONTAL_SPACING
  );

  assert(origin.xpos >= (WIDTH / 2));

  point_t tile_origin = {
    .xpos = origin.xpos - (WIDTH / 2),
    .ypos = origin.ypos
  };
  char letter;

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    letter = word[index];

    if (letter == '\0')
    {
      gfx_Sprite_NoClip(
        spr_character_slot, tile_origin.xpos, tile_origin.ypos
      );
    }
    else
    {
      draw_character_and_tile(letter, evaluation[index], tile_origin);
    }

    tile_origin.xpos += spr_character_slot->width + HORIZONTAL_SPACING;
  }

  return;
}


pause_menu_code_t Gameplay::pause_menu() const
{
  const uint8_t NUM_OPTIONS = 2;
  const char* BUTTON_TITLES[NUM_OPTIONS] = { "Resume", "Show Word" };

  GuiText text;
  Container container;
  Container header;
  uint8_t option_index = 0;
  uint8_t button_ypos;
  bool first_draw = true;
  bool draw_help = false;

  gfx_SetColor(BLACK);

  for (uint8_t ypos = 0; ypos < LCD_HEIGHT; ypos += 2)
    gfx_HorizLine_NoClip(0, ypos, LCD_WIDTH);

  container.set_width(200);
  container.set_height(140);
  container.center_both_axes_on_screen();
  container.draw();
  header.set_width(150);
  header.set_height(30);
  header.center_horizontally_on_screen();
  header.center_vertically_on_ypos(container.get_ypos());
  header.set_color(DARK_MED_BLUE);
  header.set_border_radius(14);
  header.set_z_index(0);
  header.draw();
  gfx_SetColor(DARK_MED_BLUE);
  gfx_FillRectangle_NoClip(
    header.get_xpos() - 1, header.get_ypos() + 14, 151, 15
  );
  text.set_font(GuiText::DOUBLE_SIZE_WITH_SHADOW);
  text.set_ypos(header.get_ypos() + 7);
  text.draw_centered_string("PAUSED");

  while (true)
  {
    button_ypos = container.get_ypos() + 45;

    for (uint8_t index = 0; index < NUM_OPTIONS; index++)
    {
      if (option_index == index)
        gui_DrawButtonSelected(BUTTON_TITLES[index], button_ypos);
      else
        gui_DrawButton(BUTTON_TITLES[index], button_ypos);

      button_ypos += 35;
    }

    if (draw_help)
    {
      text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);
      text.set_ypos(button_ypos + 5);
      text.draw_centered_string("[clear]: Quit game");
    }

    if (first_draw)
    {
      gfx_BlitBuffer();
      first_draw = false;
    }
    else
    {
      gfx_SwapDraw();
    }

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
    else if (Keypad::was_released_exclusive(kb_KeyMode))
    {
      draw_help = true;
    }
    else if (Keypad::was_released_exclusive(kb_KeyClear))
    {
      return QUIT_GAME;
    }
    else if (
      Keypad::was_released_exclusive(kb_Key2nd)
      || Keypad::was_released_exclusive(kb_KeyEnter)
    )
    {
      if (option_index == 0)
      {
        return RESUME_GAME;
      }
      else if (option_index == 1)
      {
        return SHOW_WORD;
      }
    }
  }

  // Something is very wrong if this executes.
  assert(false);
  return NO_DECISION;
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static void draw_character_tile(
  IN evaluation_code_t evaluation_code, IN point_t& origin
)
{
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
  return;
}


static void draw_character_and_tile(
  IN char character, IN evaluation_code_t evaluation_code, IN point_t& origin
)
{
  GuiText text;

  draw_character_tile(evaluation_code, origin);
  text.set_font(GuiText::TILE_CHARACTER);
  text.set_xpos(origin.xpos + ((20 - gfx_GetCharWidth(character)) / 2));
  text.set_ypos(origin.ypos + 3);
  text.draw_character(character);
  return;
}
