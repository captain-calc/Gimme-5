#ifndef GAME_H
#define GAME_H


#include <graphx.h>

#include "dictionary.h"
#include "gui.h"
#include "word.h"


// ============================================================================
// DEFINITIONS
// ============================================================================


enum pause_menu_code_t : uint8_t
{
  NO_DECISION = 0,
  OPTION_ONE,
  OPTION_TWO,
  QUIT_GAME
};


// ============================================================================
// CLASS DECLARATIONS
// ============================================================================


class Gameplay
{
  public:
    Gameplay(uint8_t id);
    virtual ~Gameplay() {}
    // void display_help();
    // gfx_sprite_t* get_gameplay_icon();
    virtual void play_random_word() = 0;

    uint8_t get_id() const;

  protected:
    void draw_word_input(IN word_string_t word) const;
    void draw_guess_evaluation(
      IN word_evaluation_t evaluation, IN point_t& origin
    ) const;
    void draw_evaluated_guess(
      IN Word& word, IN word_evaluation_t evaluation, IN point_t& origin
    ) const;
    pause_menu_code_t pause_menu(
      IN char** button_titles, IN uint8_t num_options
    ) const;

  private:
    uint8_t id;
};


#endif
