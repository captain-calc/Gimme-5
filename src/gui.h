#ifndef GUI_H
#define GUI_H


#include "typehints.h"


#define TRANSPARENT   (0)
#define BG_COLOR      (1)
#define BLUE          (2)
#define MEDIUM_BLUE   (3)
#define LIGHT_BLUE    (4)
#define WHITE         (5)
#define BLACK         (6)
#define GREEN         (7)
#define DARK_ORANGE   (8)
#define ORANGE        (9)
#define LIGHT_ORANGE (10)


#define GFX_DEFAULT_FONT_HEIGHT (8)


typedef struct
{
  uint24_t xpos;
  uint8_t ypos;
} point_t;


typedef struct
{
  uint24_t xpos;
  uint8_t ypos;
  uint24_t width;
  uint8_t height;
} rectangle_t;


class GuiText
{
  public:
    GuiText();

    enum font_style_t : uint8_t
    {
      DEFAULT = 0,
      NORMAL_SIZE_WITH_SHADOW,
      DOUBLE_SIZE_WITH_SHADOW,
      MENU_TITLE,
      TILE_CHARACTER,
      GAME_COMPLETION,
      GAMEPLAY_OPTION_TITLE
    };

    uint8_t get_font_height() const;

    void draw_string(IN char* string);
    void draw_centered_string(IN char* string);
    void draw_character(IN char character);
    void draw_unsigned_int(IN unsigned int integer);

    void set_font(IN font_style_t font_style);
    void set_xpos(IN uint24_t xpos);
    void set_ypos(IN uint8_t ypos);

  private:
    uint8_t background_color;
    uint8_t foreground_color;
    uint8_t transparent_color;

    uint8_t z_index;
    uint8_t horizontal_scale;
    uint8_t vertical_scale;
};


class Rectangle
{
  public:
    Rectangle();

    void draw() const;

    uint24_t get_xpos() const;
    uint8_t get_ypos() const;

    void set_xpos(IN uint24_t xpos);
    void set_ypos(IN uint8_t ypos);
    void set_width(IN uint24_t width);
    void set_height(IN uint8_t height);
    void center_on_point(IN point_t& point);
    void center_vertically_on_screen();
    void center_horizontally_on_screen();
    void center_both_axes_on_screen();

    void set_color(IN uint8_t color);

  protected:
    rectangle_t rectangle;
    uint8_t color;
};


class DecoratedRectangle : public Rectangle
{
  public:
    DecoratedRectangle();

    void draw();

    void set_border_color(IN uint8_t border_color);
    void set_border_radius(IN uint8_t border_radius);
    void set_border_thickness(IN uint8_t border_thickness);

    void make_inset();
    void set_z_index(IN uint8_t z_index);

    void shrink_towards_center_by_num_pixels(IN uint8_t num_pixels);

  private:
    uint8_t border_color;
    uint8_t border_radius;
    uint8_t border_thickness;

    bool inset;
    uint8_t z_index;
};


class Container : public DecoratedRectangle
{
  public:
    Container();
};


class InputField
{
  public:
    InputField(IN uint8_t ypos, IN uint24_t width);

    void draw_input(IN char* input);

  private:
    DecoratedRectangle rectangle;
};


class Notification
{
  public:
    Notification(IN char* message);

    void draw();
    void blit();

  private:
    static const uint8_t MAX_MESSAGE_LENGTH = 30;

    GuiText text;
    char message[MAX_MESSAGE_LENGTH + 1] = { '\0' };
    rectangle_t container;
    uint24_t animation_index;
};


void gui_StartGraphics();
void gui_EndGraphics();

void gui_DrawHelpScreen(
  IN char** strings, IN uint8_t num_strings, IN uint8_t ypos
);

void gui_DrawButton(IN char* title, IN uint8_t ypos);
void gui_DrawButtonSelected(IN char* title, IN uint8_t ypos);
void gui_DrawMenuTitle(IN char* title);
void gui_Notification(IN char* message);
void gui_DrawPageNumberIndicator(
  IN uint8_t num_pages, IN uint8_t current_page_num
);
void gui_DrawGameplayOption(IN char* title, IN point_t& origin);
void gui_DrawGameplayOptionSelected(IN char* title, IN point_t& origin);
void gui_DrawCheckeredBackground();

void gui_TransitionIn();
void gui_TransitionOut();


#endif
