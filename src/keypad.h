#ifndef KEYPAD_H
#define KEYPAD_H


#include <keypadc.h>
#include <time.h>

#include "typehints.h"


typedef const char* keymap_t[7];

class Keypad
{
  public:
    static void update_state();
    static bool is_any_key_down();
    static bool is_down(IN kb_lkey_t key);
    static bool is_down_exclusive(IN kb_lkey_t key);
    static bool was_down(IN kb_lkey_t key);
    static bool was_down_exclusive(IN kb_lkey_t key);
    static bool is_down_repeating(IN kb_lkey_t key);
    static bool was_any_key_released();
    static bool was_released(IN kb_lkey_t key);
    static bool was_released_exclusive(IN kb_lkey_t key);
    static void block_until_any_key_released();
    static kb_lkey_t get_only_key_down();
    static bool get_character_from_keymap(
      IN keymap_t keymap, OUT char& character
    );

  private:
    static Keypad instance;
    static uint8_t previous_state[8];
    static uint8_t state[8];
    static clock_t prev_timestamp;
    static clock_t timestamp;
    static clock_t timestamp_delta;

    Keypad();
};

bool keypad_GetHexAsciiCharacter(OUT char& character);
bool keypad_GetUppercaseAsciiLetter(OUT char& letter);


#endif
