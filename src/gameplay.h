#ifndef GAME_H
#define GAME_H


#include <graphx.h>

#include "dictionary.h"
#include "gui.h"
#include "word.h"


enum pause_menu_code_t : uint8_t
{
  NO_DECISION = 0,
  RESUME_GAME = 0,
  QUIT_GAME   = (1 << 0),
  REVEAL      = (1 << 1)
};


class AlphabetTracker
{
  public:
    AlphabetTracker();
    void initialize();
    void include_guess(IN Word& guess, IN word_evaluation_t evaluation);
    void draw() const;

  private:
    char letters_not_in_target[26];
};


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
    pause_menu_code_t pause_menu() const;

  private:
    uint8_t id;
};


class AnagramGameplay : public Gameplay
{
  public:
    AnagramGameplay(): Gameplay(2) { return; }
    void play_random_word();
};



#endif
