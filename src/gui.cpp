#include <sys/lcd.h>
#include <sys/timers.h>
#include <ti/screen.h>
#include <assert.h>
#include <graphx.h>
#include <string.h>

#include "gfx/gfx.h"
#include "gui.h"
#include "keypad.h"


// ============================================================================
// DEFINITIONS AND DATA STRUCTURE DECLARATIONS
// ============================================================================


typedef struct
{
  const char* title;
  uint8_t ypos;
  uint8_t highlight_color;
  uint8_t main_color;
  uint8_t inner_shadow_color;
  uint8_t text_color;
} button_attributes_t;


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static unsigned int log10(IN unsigned int value);
static void draw_filled_rectangle(IN rectangle_t& rectangle);
static void draw_filled_rounded_rectangle(
  IN rectangle_t& rectangle, IN uint8_t corner_radius
);
static void draw_button(IN button_attributes_t& attributes);
static void draw_gameplay_option(
  IN char* title, IN point_t& origin, MOD Container& container
);


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


GuiText::GuiText()
{
  this->background_color = TRANSPARENT;
  this->transparent_color = TRANSPARENT;

  set_font(DEFAULT);
  return;
}


uint8_t GuiText::get_font_height() const
{
  return GFX_DEFAULT_FONT_HEIGHT * vertical_scale;
}


void GuiText::draw_string(IN char* string)
{
  const char* character = string;

  while (*character != '\0')
  {
    draw_character(*character);
    character++;
  }

  return;
}


void GuiText::draw_centered_string(IN char* string)
{
  // The font scale must be set before the string's width is calculated.
  // gfx_SetTextScale(this->horizontal_scale, this->vertical_scale);

  set_xpos((LCD_WIDTH - gfx_GetStringWidth(string)) / 2);
  draw_string(string);
  return;
}


void GuiText::draw_character(IN char character)
{
  const uint24_t XPOS = gfx_GetTextX();
  const uint8_t YPOS = gfx_GetTextY();

  uint24_t shadow_xpos = XPOS + z_index;
  uint8_t shadow_ypos = YPOS + z_index;

  assert(shadow_xpos + gfx_GetCharWidth(character) < LCD_WIDTH);
  assert(shadow_ypos + get_font_height() < LCD_HEIGHT);

  if (z_index > 0)
  {
    gfx_SetTextFGColor(BLACK);
    gfx_SetTextXY(shadow_xpos, shadow_ypos);
    gfx_PrintChar(character);
  }

  gfx_SetTextXY(XPOS, YPOS);
  gfx_SetTextFGColor(this->foreground_color);
  gfx_PrintChar(character);
  return;
}


void GuiText::draw_unsigned_int(IN unsigned int integer)
{
  const uint24_t XPOS = gfx_GetTextX();
  const uint8_t YPOS = gfx_GetTextY();

  uint24_t shadow_xpos = XPOS + z_index;
  uint8_t shadow_ypos = YPOS + z_index;

  // Assertion checking integer with deliberately skipped.
  assert(shadow_ypos + get_font_height() < LCD_HEIGHT);

  if (z_index > 0)
  {
    gfx_SetTextFGColor(BLACK);
    gfx_SetTextXY(shadow_xpos, shadow_ypos);
    gfx_PrintUInt(integer, log10(integer));
  }

  gfx_SetTextXY(XPOS, YPOS);
  gfx_SetTextFGColor(this->foreground_color);
  gfx_PrintUInt(integer, log10(integer));
  return;
}


void GuiText::set_font(IN font_style_t font_style)
{
  switch (font_style)
  {
    case NORMAL_SIZE_WITH_SHADOW:
      this->foreground_color = WHITE;
      this->z_index = 1;
      this->horizontal_scale = 1;
      this->vertical_scale = 1;
      break;

    case DOUBLE_SIZE_WITH_SHADOW:
      this->foreground_color = WHITE;
      this->z_index = 2;
      this->horizontal_scale = 2;
      this->vertical_scale = 2;
      break;

    case MENU_TITLE:
      this->foreground_color = WHITE;
      this->z_index = 2;
      this->horizontal_scale = 2;
      this->vertical_scale = 3;
      break;

    case TILE_CHARACTER:
      this->foreground_color = WHITE;
      this->z_index = 1;
      this->horizontal_scale = 1;
      this->vertical_scale = 2;
      break;

    case GAME_COMPLETION:
      this->foreground_color = WHITE;
      this->z_index = 2;
      this->horizontal_scale = 3;
      this->vertical_scale = 2;
      break;

    case GAMEPLAY_OPTION_TITLE:
      this->foreground_color = WHITE;
      this->z_index = 2;
      this->horizontal_scale = 1;
      this->vertical_scale = 2;
      break;

    case DEFAULT:
    default:
      this->foreground_color = WHITE;
      this->z_index = 0;
      this->horizontal_scale = 1;
      this->vertical_scale = 1;
      break;
  }

  gfx_SetTextBGColor(this->background_color);
  gfx_SetTextTransparentColor(this->transparent_color);
  gfx_SetTextScale(this->horizontal_scale, this->vertical_scale);
  return;
}


void GuiText::set_xpos(IN uint24_t xpos)
{
  gfx_SetTextXY(xpos, gfx_GetTextY());
  return;
}


void GuiText::set_ypos(IN uint8_t ypos)
{
  gfx_SetTextXY(gfx_GetTextX(), ypos);
  return;
}


Rectangle::Rectangle()
{
  rectangle.xpos = 0;
  rectangle.ypos = 0;
  rectangle.width = 0;
  rectangle.height = 0;
  color = WHITE;
  return;
}


void Rectangle::draw() const
{
  gfx_SetColor(color);
  gfx_FillRectangle_NoClip(
    rectangle.xpos, rectangle.ypos, rectangle.width, rectangle.height
  );
  return;
}


uint24_t Rectangle::get_xpos() const
{
  return rectangle.xpos;
}


uint8_t Rectangle::get_ypos() const
{
  return rectangle.ypos;
}


void Rectangle::set_xpos(IN uint24_t xpos)
{
  rectangle.xpos = xpos;
  return;
}


void Rectangle::set_ypos(IN uint8_t ypos)
{
  rectangle.ypos = ypos;
  return;
}


void Rectangle::set_width(IN uint24_t width)
{
  assert(width <= LCD_WIDTH);

  rectangle.width = width;
  return;
}


void Rectangle::set_height(IN uint8_t height)
{
  assert(height <= LCD_HEIGHT);

  rectangle.height = height;
  return;
}


void Rectangle::center_horizontally_on_xpos(IN uint24_t xpos)
{
  assert((xpos - (rectangle.width / 2)) < LCD_WIDTH);

  rectangle.xpos = xpos - (rectangle.width / 2);
  return;
}


void Rectangle::center_vertically_on_ypos(IN uint8_t ypos)
{
  assert(ypos - (rectangle.height / 2));

  rectangle.ypos = ypos - (rectangle.height / 2);
  return;
}


void Rectangle::center_both_axes_on_point(IN point_t& point)
{
  center_horizontally_on_xpos(point.xpos);
  center_vertically_on_ypos(point.ypos);
  return;
}


void Rectangle::center_vertically_on_screen()
{
  center_vertically_on_ypos(LCD_HEIGHT / 2);
  return;
}


void Rectangle::center_horizontally_on_screen()
{
  center_horizontally_on_xpos(LCD_WIDTH / 2);
  return;
}


void Rectangle::center_both_axes_on_screen()
{
  center_vertically_on_screen();
  center_horizontally_on_screen();
  return;
}


void Rectangle::set_color(IN uint8_t color)
{
  this->color = color;
  return;
}


DecoratedRectangle::DecoratedRectangle()
{
  border_color = WHITE;
  border_radius = 0;
  border_thickness = 0;
  inset = false;
  z_index = 0;
  return;
}


void DecoratedRectangle::draw()
{
  rectangle_t shadow_rectangle = rectangle;
  rectangle_t highlight_rectangle = rectangle;
  rectangle_t border_rectangle = rectangle;

  if (inset)
  {
    shadow_rectangle.xpos -= z_index;
    shadow_rectangle.ypos -= z_index;
    highlight_rectangle.xpos += z_index;
    highlight_rectangle.ypos += z_index;
  }
  else
  {
    shadow_rectangle.xpos += z_index;
    shadow_rectangle.ypos += z_index;
  }

  if (border_radius > 0)
  {
    if (inset)
    {
      gfx_SetColor(WHITE);
      draw_filled_rounded_rectangle(highlight_rectangle, border_radius);
    }

    if (z_index > 0)
    {
      gfx_SetColor(BLACK);
      draw_filled_rounded_rectangle(shadow_rectangle, border_radius);
    }

    if (border_thickness > 0)
    {
      gfx_SetColor(border_color);
      draw_filled_rounded_rectangle(border_rectangle, border_radius);
      shrink_towards_center_by_num_pixels(2 * border_thickness);
    }

    gfx_SetColor(color);
    draw_filled_rounded_rectangle(rectangle, border_radius);
  }
  else
  {
    if (inset)
    {
      gfx_SetColor(WHITE);
      draw_filled_rectangle(highlight_rectangle);
    }

    if (z_index > 0)
    {
      gfx_SetColor(BLACK);
      draw_filled_rectangle(shadow_rectangle);
    }

    if (border_thickness > 0)
    {
      gfx_SetColor(border_color);
      draw_filled_rectangle(border_rectangle);
      shrink_towards_center_by_num_pixels(2 * border_thickness);
    }

    gfx_SetColor(color);
    draw_filled_rectangle(rectangle);
  }

  return;
}


void DecoratedRectangle::set_border_color(IN uint8_t border_color)
{
  this->border_color = border_color;
  return;
}


void DecoratedRectangle::set_border_radius(IN uint8_t border_radius)
{
  this->border_radius = border_radius;
  return;
}


void DecoratedRectangle::set_border_thickness(IN uint8_t border_thickness)
{
  this->border_thickness = border_thickness;
  return;
}


void DecoratedRectangle::make_inset()
{
  inset = true;
  return;
}


void DecoratedRectangle::set_z_index(IN uint8_t z_index)
{
  this->z_index = z_index;
  return;
}


void DecoratedRectangle::shrink_towards_center_by_num_pixels(
  IN uint8_t num_pixels
)
{
  assert(rectangle.width >= num_pixels);
  assert(rectangle.height >= num_pixels);

  point_t point = {
    .xpos = rectangle.xpos + (rectangle.width / 2),
    .ypos = (uint8_t)(rectangle.ypos + (rectangle.height / 2))
  };

  rectangle.width -= num_pixels;
  rectangle.height -= num_pixels;
  center_both_axes_on_point(point);
  return;
}


Container::Container()
{
  this->set_color(BG_COLOR);
  this->set_border_radius(6);
  this->set_border_thickness(1);
  this->set_border_color(WHITE);
  this->set_z_index(3);
  return;
}


InputField::InputField(IN uint8_t ypos, IN uint24_t width)
{
  rectangle.set_ypos(ypos);
  rectangle.set_width(width);
  rectangle.set_height(12);
  rectangle.center_horizontally_on_screen();
  rectangle.set_color(BLUE);
  rectangle.set_border_radius(6);
  rectangle.make_inset();
  rectangle.set_z_index(1);
  return;
}


void InputField::draw_input(IN char* input)
{
  GuiText text;

  rectangle.draw();
  text.set_font(GuiText::DEFAULT);
  text.set_ypos(rectangle.get_ypos() + 3);
  text.draw_centered_string(input);
  return;
}


Notification::Notification(IN char* message)
{
  assert(strlen(message) <= MAX_MESSAGE_LENGTH);

  memcpy(this->message, message, strlen(message));

  // This must be set before gfx_GetStringWidth() is called.
  this->text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);

  this->container.xpos = 7;
  this->container.ypos = 17;
  this->container.width = gfx_GetStringWidth(message) + 32;
  this->container.height = 19;
  this->animation_index = 0;
  return;
}


void Notification::draw()
{
  rectangle_t inner_border = this->container;
  rectangle_t content = this->container;

  inner_border.xpos -= 2;
  inner_border.ypos -= 2;
  content.xpos -= 1;
  content.ypos -= 1;
  content.width -= 2;
  content.height -= 2;

  gfx_SetColor(BLACK);
  draw_filled_rounded_rectangle(this->container, 6);
  gfx_SetColor(WHITE);
  draw_filled_rounded_rectangle(inner_border, 6);
  gfx_SetColor(ORANGE);
  draw_filled_rounded_rectangle(content, 6);

  this->text.set_xpos(this->container.xpos + 21);
  this->text.set_ypos(this->container.ypos + 4);
  this->text.draw_string(message);

  gfx_TransparentSprite_NoClip(spr_notification_icon, 0, 10);
  return;
}


void Notification::blit()
{
  const uint24_t DELAY = CLOCKS_PER_SEC / 1000;

  static clock_t timestamp = clock();

  gfx_BlitRectangle(
    gfx_buffer,
    0,
    10,
    spr_notification_icon->width,
    spr_notification_icon->height
  );

  if (
    this->animation_index < this->container.width
    && clock() > (timestamp + DELAY)
  )
  {
    gfx_BlitRectangle(
      gfx_buffer,
      spr_notification_icon->width + this->animation_index,
      15,
      2,
      22
    );

    this->animation_index += 2;
    timestamp = clock() + DELAY;
  }

  return;
}


void gui_StartGraphics()
{
  gfx_Begin();
  gfx_SetPalette(global_palette, sizeof_global_palette, 0);
  gfx_SetDrawBuffer();
  return;
}


void gui_EndGraphics()
{
  gfx_End();
  return;
}


void gui_DrawHelpScreen(
  IN char** strings, IN uint8_t num_strings, IN uint8_t ypos
)
{
  const uint8_t LINE_HEIGHT = GFX_DEFAULT_FONT_HEIGHT + 3;

  GuiText text;
  uint8_t text_ypos = ypos;

  gui_DrawCheckeredBackground();
  gui_DrawMenuTitle("Help");
  text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);

  for (uint8_t index = 0; index < num_strings; index++)
  {
    text.set_xpos(5);
    text.set_ypos(text_ypos);
    text.draw_string(strings[index]);
    text_ypos += LINE_HEIGHT;
  }

  return;
}


void gui_DrawButton(IN char* title, IN uint8_t ypos)
{
  button_attributes_t attributes = {
    .title = title,
    .ypos = ypos,
    .highlight_color = LIGHT_BLUE,
    .main_color = MEDIUM_BLUE,
    .inner_shadow_color = BLUE,
    .text_color = WHITE
  };

  draw_button(attributes);
  return;
}


void gui_DrawButtonSelected(IN char* title, IN uint8_t ypos)
{
  button_attributes_t attributes = {
    .title = title,
    .ypos = ypos,
    .highlight_color = LIGHT_ORANGE,
    .main_color = ORANGE,
    .inner_shadow_color = DARK_ORANGE,
    .text_color = WHITE
  };

  draw_button(attributes);
  return;
}


void gui_DrawMenuTitle(IN char* title)
{
  const uint8_t TITLE_YPOS = 15;
  const uint24_t LINE_WIDTH = 256;
  const uint24_t LINE_XPOS = (LCD_WIDTH - LINE_WIDTH) / 2;

  GuiText text;
  uint8_t line_ypos;

  text.set_font(GuiText::MENU_TITLE);
  text.set_ypos(TITLE_YPOS);
  text.draw_centered_string(title);

  line_ypos = TITLE_YPOS + text.get_font_height() + 4;

  gfx_SetColor(BLACK);
  gfx_FillRectangle_NoClip(LINE_XPOS + 2, line_ypos + 2, LINE_WIDTH, 2);

  gfx_SetColor(WHITE);
  gfx_FillRectangle_NoClip(LINE_XPOS, line_ypos, LINE_WIDTH, 2);
  return;
}


void gui_Notification(IN char* message)
{
  GuiText text;
  text.set_font(GuiText::NORMAL_SIZE_WITH_SHADOW);

  const uint24_t CONTAINER_WIDTH = gfx_GetStringWidth(message) + 30;

  rectangle_t container = {
    .xpos = 7,
    .ypos = 17,
    .width = CONTAINER_WIDTH + 2,
    .height = 19
  };

  gfx_SetColor(BLACK);
  draw_filled_rounded_rectangle(container, 6);
  container.xpos -= 2;
  container.ypos -= 2;
  gfx_SetColor(WHITE);
  draw_filled_rounded_rectangle(container, 6);
  container.xpos += 1;
  container.ypos += 1;
  container.width -= 2;
  container.height -= 2;
  gfx_SetColor(ORANGE);
  draw_filled_rounded_rectangle(container, 6);


  text.set_xpos(28);
  text.set_ypos(21);
  text.draw_string(message);

  gfx_TransparentSprite_NoClip(spr_notification_icon, 0, 10);

  for (uint24_t index = 0; index < CONTAINER_WIDTH; index++)
  {
     gfx_BlitRectangle(
      gfx_buffer,
      0,
      10,
      spr_notification_icon->width + index,
      spr_notification_icon->height
    );

    delay(1);
  }

  Keypad::block_until_any_key_released();
  return;
}


void gui_DrawPageNumberIndicator(
  IN uint8_t num_pages, IN uint8_t current_page_num
)
{
  const uint24_t HORIZONTAL_SPACING = 5;

  point_t dot = {
    .xpos = (
      LCD_WIDTH - (num_pages * (spr_page_dot->width + HORIZONTAL_SPACING))
    ) / 2,
    .ypos = 219
  };

  for (uint8_t page_num = 1; page_num <= num_pages; page_num++)
  {
    if (page_num == current_page_num)
      gfx_TransparentSprite_NoClip(spr_page_dot, dot.xpos, dot.ypos);
    else
      gfx_TransparentSprite_NoClip(spr_page_dot_slot, dot.xpos, dot.ypos);

    dot.xpos += spr_page_dot->width + HORIZONTAL_SPACING;
  }

  return;
}


void gui_DrawGameplayOption(IN char* title, IN point_t& origin)
{
  Container container;

  draw_gameplay_option(title, origin, container);
  return;
}


void gui_DrawGameplayOptionSelected(IN char* title, IN point_t& origin)
{
  Container container;

  container.set_color(ORANGE);
  container.set_z_index(10);
  draw_gameplay_option(title, origin, container);
  return;
}


void gui_DrawCheckeredBackground()
{
  gfx_FillScreen(BG_COLOR);
  gfx_SetColor(BLUE);

  for (uint8_t ypos = 0; ypos < LCD_HEIGHT; ypos += 16)
  {
    for (uint24_t xpos = 0; xpos < LCD_WIDTH; xpos += 16)
    {
      if (xpos % 32 == ypos % 32)
        gfx_FillRectangle_NoClip(xpos, ypos, 16, 16);
    }
  }

  return;
}


void gui_TransitionIn()
{
  for (uint8_t ypos = LCD_HEIGHT - 1; ypos > 0; ypos--)
  {
    gfx_BlitLines(gfx_buffer, ypos, 1);
    delay(1);
  }

  gfx_BlitLines(gfx_buffer, 0, 1);
  return;
}


void gui_TransitionOut()
{
  gfx_FillScreen(BG_COLOR);
  gfx_SetColor(BLUE);

  for (uint8_t ypos = 0; ypos < LCD_HEIGHT; ypos += 20)
  {
    for (uint24_t xpos = 0; xpos < LCD_WIDTH; xpos += 20)
    {
      if (xpos % 40 == ypos % 40)
        gfx_FillRectangle_NoClip(xpos, ypos, 20, 20);
    }
  }

  for (uint8_t ypos = LCD_HEIGHT - 1; ypos > 0; ypos--)
  {
    gfx_BlitLines(gfx_buffer, ypos, 1);
    delay(1);
  }

  gfx_BlitLines(gfx_buffer, 0, 1);
  delay(50);
  return;
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static unsigned int log10(IN unsigned int value)
{
  unsigned int operand = value;
  unsigned int log = 1;

  while (operand > 9)
  {
    operand /= 10;
    log++;
  }

  return log;
}


static void draw_filled_rectangle(IN rectangle_t& rectangle)
{
  gfx_FillRectangle_NoClip(
    rectangle.xpos, rectangle.ypos, rectangle.width, rectangle.height
  );
  return;
}


static void draw_filled_rounded_rectangle(
  IN rectangle_t& rectangle, IN uint8_t corner_radius
)
{
  gfx_FillCircle_NoClip(
    rectangle.xpos + corner_radius,
    rectangle.ypos + corner_radius,
    corner_radius
  );

  gfx_FillCircle_NoClip(
    rectangle.xpos + rectangle.width - corner_radius,
    rectangle.ypos + corner_radius,
    corner_radius
  );

  if (corner_radius <= rectangle.height)
  {
    gfx_FillCircle_NoClip(
      rectangle.xpos + corner_radius,
      rectangle.ypos + rectangle.height - corner_radius,
      corner_radius
    );

    gfx_FillCircle_NoClip(
      rectangle.xpos + rectangle.width - corner_radius,
      rectangle.ypos + rectangle.height - corner_radius,
      corner_radius
    );

    gfx_FillRectangle_NoClip(
      rectangle.xpos,
      rectangle.ypos + corner_radius,
      rectangle.width + 1,
      rectangle.height - (2 * corner_radius)
    );
  }

  gfx_FillRectangle_NoClip(
    rectangle.xpos + corner_radius,
    rectangle.ypos,
    rectangle.width - (2 * corner_radius),
    rectangle.height + 1
  );

  return;
}


static void draw_button(IN button_attributes_t& attributes)
{
  // This must be called before gfx_GetStringWidth().
  gfx_SetTextScale(1, 2);

  const uint24_t message_length = gfx_GetStringWidth(attributes.title);
  const uint24_t message_horizontal_padding = 20;
  const uint8_t message_vertical_padding = 6;
  const uint8_t border_thickness = 2;
  const uint8_t corner_radius = 9;

  uint8_t button_height = (
    GFX_DEFAULT_FONT_HEIGHT
    + (2 * message_vertical_padding)
    + (2 * border_thickness)
  );
  uint24_t button_width = 80 + (2 * message_horizontal_padding);
  uint24_t button_xpos = (LCD_WIDTH / 2) - (button_width / 2);
  uint8_t button_ypos = attributes.ypos;

  rectangle_t shadow_rectangle = {
    .xpos = button_xpos + 3,
    .ypos = (uint8_t)(button_ypos + 3),
    .width = button_width,
    .height = (uint8_t)(button_height + 2)
  };

  rectangle_t border_rectangle = {
    .xpos = button_xpos,
    .ypos = button_ypos,
    .width = button_width,
    .height = button_height
  };

  rectangle_t inner_rectangle = {
    .xpos = button_xpos,
    .ypos = (uint8_t)(button_ypos + border_thickness),
    .width = button_width,
    .height = button_height,
  };

  rectangle_t inner_shadow_rectangle = {
    .xpos = button_xpos + 2,
    .ypos = (uint8_t)(button_ypos + border_thickness + (button_height / 2)),
    .width = button_width - 4,
    .height = (uint8_t)(button_height - (button_height / 2) - 1),
  };

  gfx_SetColor(BLACK);
  draw_filled_rounded_rectangle(shadow_rectangle, corner_radius);
  gfx_SetColor(attributes.highlight_color);
  draw_filled_rounded_rectangle(border_rectangle, corner_radius);
  gfx_SetColor(attributes.main_color);
  draw_filled_rounded_rectangle(inner_rectangle, corner_radius);
  gfx_SetColor(attributes.inner_shadow_color);
  draw_filled_rounded_rectangle(inner_shadow_rectangle, 5);

  gfx_SetTextBGColor(TRANSPARENT);
  gfx_SetTextFGColor(BLACK);
  gfx_SetTextTransparentColor(TRANSPARENT);
  gfx_PrintStringXY(
    attributes.title,
    button_xpos + (button_width / 2) - (message_length / 2) - 1,
    button_ypos + message_vertical_padding + border_thickness - 1
  );

  gfx_SetTextFGColor(attributes.highlight_color);
  gfx_PrintStringXY(
    attributes.title,
    button_xpos + (button_width / 2) - (message_length / 2) + 1,
    button_ypos + message_vertical_padding + border_thickness + 1
  );

  gfx_SetTextFGColor(attributes.text_color);
  gfx_PrintStringXY(
    attributes.title,
    button_xpos + (button_width / 2) - (message_length / 2),
    button_ypos + message_vertical_padding + border_thickness
  );

  return;
}


static void draw_gameplay_option(
  IN char* title, IN point_t& origin, MOD Container& container
)
{
  const uint24_t CONTAINER_WIDTH = 86;
  const uint8_t CONTAINER_HEIGHT = 120;
  const uint8_t TITLE_HEIGHT = 30;
  const uint8_t SPACING_AROUND_ICON_BACKGROUND = 5;

  GuiText text;
  DecoratedRectangle icon_background;

  container.set_ypos(origin.ypos);
  container.set_width(CONTAINER_WIDTH);
  container.set_height(CONTAINER_HEIGHT);
  container.center_horizontally_on_xpos(origin.xpos);
  container.set_border_color(WHITE);
  container.draw();

  icon_background.set_ypos(origin.ypos + TITLE_HEIGHT);
  icon_background.set_width(
    CONTAINER_WIDTH - (2 * SPACING_AROUND_ICON_BACKGROUND)
  );
  icon_background.set_height(
    CONTAINER_HEIGHT - TITLE_HEIGHT - SPACING_AROUND_ICON_BACKGROUND
  );
  icon_background.center_horizontally_on_xpos(origin.xpos);
  icon_background.set_color(BLACK);
  icon_background.set_border_radius(6);
  icon_background.draw();

  text.set_font(GuiText::GAMEPLAY_OPTION_TITLE);
  text.set_xpos(
    container.get_xpos() + ((CONTAINER_WIDTH - gfx_GetStringWidth(title)) / 2)
  );
  text.set_ypos(container.get_ypos() + 8);
  text.draw_string(title);
  return;
}
