#include "keypad.h"


#define only_one_bit_set(byte) \
( (byte & (byte - 1) || !byte) ? 0 : 1 )


// ============================================================================
// STATIC FUNCTION DECLARATIONS
// ============================================================================


static uint8_t bit_to_idx(uint8_t byte);


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


void Keypad::update_state()
{
  for (uint8_t index = 1; index < 8; index++)
  {
    Keypad::previous_state[index] = kb_Data[index];
  }

  kb_Scan();

  for (uint8_t index = 1; index < 8; index++)
  {
    Keypad::state[index] = kb_Data[index];
  }

  if (Keypad::timestamp_delta)
  {
    Keypad::prev_timestamp = Keypad::timestamp + Keypad::timestamp_delta;
    Keypad::timestamp_delta = 0;
  }

  Keypad::timestamp = clock();

  return;
}


bool Keypad::is_any_key_down()
{
  uint8_t sum = 0;

  for (uint8_t index = 1; index < 8; index++)
    sum |= Keypad::state[index];

  return (bool)sum;
}


bool Keypad::is_down(IN kb_lkey_t key)
{
  return (Keypad::state[key >> 8] & (key % 256));
}


bool Keypad::is_down_exclusive(IN kb_lkey_t key)
{
  for (uint8_t index = 1; index < 8; index++)
  {
    if (index != (key >> 8))
    {
      if (Keypad::state[index])
        return false;
    }
    else if (Keypad::state[index] != (key % 256))
    {
      return false;
    }
  }

  return is_down(key);
}


bool Keypad::was_down(IN kb_lkey_t key)
{
  return (Keypad::previous_state[key >> 8] & (key % 256));
}


bool Keypad::was_down_exclusive(IN kb_lkey_t key)
{
  for (uint8_t index = 1; index < 8; index++)
  {
    if (index != (key >> 8))
    {
      if (Keypad::previous_state[index])
        return false;
    }
    else if (Keypad::previous_state[index] != (key % 256))
    {
      return false;
    }
  }

  return was_down(key);
}


bool Keypad::is_down_repeating(IN kb_lkey_t key)
{
  const uint24_t INITIAL_PAUSE = CLOCKS_PER_SEC / 2;
  const uint24_t DELAY = CLOCKS_PER_SEC / 12;

  if (was_down_exclusive(key))
  {
    if (is_down_exclusive(key) && Keypad::timestamp >= Keypad::prev_timestamp)
    {
      Keypad::timestamp_delta = DELAY;
      return true;
    }
  }
  else if (is_down_exclusive(key))
  {
    Keypad::timestamp_delta = INITIAL_PAUSE;
    return true;
  }

  return false;
}


bool Keypad::was_any_key_released()
{
  uint8_t sum = 0;

  for (uint8_t index = 1; index < 8; index++)
    sum |= Keypad::previous_state[index];

  return (!is_any_key_down() && (bool)sum);
}


bool Keypad::was_released(IN kb_lkey_t key)
{
  return (!is_down(key) && was_down(key));
}


bool Keypad::was_released_exclusive(IN kb_lkey_t key)
{
  return (!is_any_key_down() && was_down_exclusive(key));
}


void Keypad::block_until_any_key_released()
{
  do {
    Keypad::update_state();
  } while (!was_any_key_released());

  return;
}


kb_lkey_t Keypad::get_only_key_down()
{
  kb_lkey_t key = 0;

  for (uint8_t index = 1; index < 8; index++)
  {
    if (key == 0 && only_one_bit_set(Keypad::state[index]))
    {
      key = (index << 8) | Keypad::state[index];
    }
    else if (Keypad::state[index])
    {
      return 0;
    }
  }

  return key;
}


bool Keypad::get_character_from_keymap(
  IN keymap_t keymap, OUT char& character
)
{
  kb_lkey_t key = get_only_key_down();

  if (is_down_repeating(key))
  {
    character = keymap[(key >> 8) - 1][bit_to_idx(key % 256)];
    return true;
  }

  return false;
}


Keypad Keypad::instance;
uint8_t Keypad::previous_state[8] = { 0 };
uint8_t Keypad::state[8] = { 0 };
clock_t Keypad::prev_timestamp = 0;
clock_t Keypad::timestamp = clock();
clock_t Keypad::timestamp_delta = 0;


Keypad::Keypad()
{
  return;
}


bool keypad_GetHexAsciiCharacter(OUT char& character)
{
  keymap_t KEYMAP = {
    "\0\0\0\0\0\0\0\0", "\0\0\0\0\0DA\0", "\x30\x31\x34\x37\0EB\0",
    "\0\x32\x35\x38\0FC\0", "\0\x33\x36\x39\0\0\0\0", "\0\0\0\0\0\0\0\0",
    "\0\0\0\0"
  };

  bool key_down = Keypad::get_character_from_keymap(KEYMAP, character);

  return (key_down && character != '\0');
}


bool keypad_GetUppercaseAsciiLetter(OUT char& letter)
{
  keymap_t KEYMAP = {
    "\0\0\0\0\0\0\0\0", "\0XSNIDA\0", "\0YTOJEB\0", "\0ZUPKFC\0",
    "\0\0VQLG\0\0", "\0\0WRMH\0\0", "\0\0\0\0"
  };

  bool key_down = Keypad::get_character_from_keymap(KEYMAP, letter);

  return (key_down && letter != '\0');
}


// ============================================================================
// STATIC FUNCTION DEFINITIONS
// ============================================================================


static uint8_t bit_to_idx(uint8_t byte)
{
  uint8_t result = 0;
  if (byte & 0b11110000) result |= 4;
  if (byte & 0b11001100) result |= 2;
  if (byte & 0b10101010) result |= 1;
  return result;
}
