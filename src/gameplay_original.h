#ifndef GAMEPLAY_ORIGINAL_H
#define GAMEPLAY_ORIGINAL_H


#include "gameplay.h"


// ============================================================================
// CLASS DECLARATIONS
// ============================================================================


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


class OriginalGameplay : public Gameplay
{
  public:
    static const uint8_t ENCRYPTED_CODE_LENGTH = 12;

    OriginalGameplay();
    bool is_valid_code(IN char* code) const;

    void play_random_word();
    void play_valid_word_code(IN char* code);

  private:
    static const uint8_t ID = 0;
    static const uint8_t MAX_NUM_GUESSES = 5;
    static const uint8_t NUM_ENCRYPTION_KEYS = 16;
    const word_string_t ENCRYPTION_KEYS[NUM_ENCRYPTION_KEYS] = {
      { '0', 'c', 'b', '4', '3' }, { '2', '8', '7', '7', '7' },
      { 'c', '5', 'e', '3', 'c' }, { '4', 'f', 'd', 'f', '2' },
      { '8', '5', 'c', '5', 'b' }, { '9', '5', '5', 'c', '1' },
      { '3', 'b', '0', 'e', 'b' }, { '4', '2', '2', '1', 'd' },
      { '9', '6', 'c', '8', '1' }, { 'a', '7', 'd', '6', 'e' },
      { '4', '8', 'd', '9', '3' }, { '2', '7', 'e', '2', 'e' },
      { 'b', '1', 'b', '3', '6' }, { '0', '2', '2', 'f', '4' },
      { '7', 'f', '7', '0', '4' }, { '1', '4', '3', '0', '8' }
    };

    AlphabetTracker alphabet_tracker;
    Dictionary dictionary;
    Word target;
    Word guesses[MAX_NUM_GUESSES];
    word_evaluation_t guess_evaluations[MAX_NUM_GUESSES];
    uint8_t num_guesses;

    void draw_game_screen_background() const;
    void draw_game_screen_foreground(IN word_string_t current_guess) const;
    void draw_guesses() const;
    void draw_win_animation() const;
    void draw_lose_animation() const;
    void draw_word_code_screen() const;
    bool is_last_guess_correct() const;
    bool are_all_guesses_used() const;
    void generate_code(OUT char* code) const;
    void decrypt_word_code_into_word(IN char* code, OUT Word& word) const;
    pause_menu_code_t pause_menu() const;
    void show_help_screen() const;
    void word_not_in_dictionary_notification() const;

    void play(IN Word& target_word);
    void reset_guesses();
    void add_guess(IN word_string_t guess);
};


#endif
