#ifndef GAMEPLAY_RUSH_H
#define GAMEPLAY_RUSH_H


#include "gameplay.h"


class Timer
{
  public:
    Timer();

    void update();
    void reload();

    bool is_expired() const;
    void draw() const;
    void blit() const;

  private:
    const uint24_t WIDTH = 40;
    const uint8_t HEIGHT = 20;
    const uint24_t XPOS = LCD_WIDTH - WIDTH - 5;
    const uint8_t YPOS = 5;
    const uint8_t RELOAD_VALUE = 30;

    clock_t start_timestamp;
    uint8_t num_seconds_left;
};


class RushGameplay : public Gameplay
{
  public:
    RushGameplay();
    void play_random_word();

  private:
    static const uint8_t ID = 1;
    static const uint8_t MAX_NUM_GUESSES = 100;
    const uint8_t NUM_VISIBLE_GUESSES = 7;

    Timer timer;
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
    bool is_last_guess_correct() const;
    bool are_all_guesses_used() const;
    void show_help_screen() const;

    void reset_guesses();
    void add_guess(IN word_string_t guess);
    void swap_target_word();
    void word_not_in_dictionary_notification();
};


#endif
