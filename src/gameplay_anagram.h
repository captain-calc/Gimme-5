#ifndef GAMEPLAY_ANAGRAM_H
#define GAMEPLAY_ANAGRAM_H


#include "gameplay.h"


class AnagramGameplay : public Gameplay
{
  public:
    AnagramGameplay();
    void play_random_word();

  private:
    static const uint8_t ID = 2;
    static const uint8_t MAX_NUM_ROUNDS = 5;

    Dictionary dictionary;
    Word target;
    Word guesses[MAX_NUM_ROUNDS];
    bool guess_was_correct[MAX_NUM_ROUNDS];
    uint8_t num_rounds_completed;

    void draw_guess_correctness_icons() const;
    void draw_game_screen_background() const;
    void draw_game_screen_foreground(IN word_string_t current_guess) const;
    void draw_guess_correct_animation() const;
    void draw_guess_incorrect_animation() const;
    bool is_guess_correct() const;
    bool is_guess_incorrect() const;
    bool are_all_rounds_complete() const;
    void show_help_screen() const;

    void reset_guesses();
    void add_guess(IN word_string_t guess);
    void pick_next_target_word();
    void word_not_in_dictionary_notification();
    void results_screen();
};


#endif
