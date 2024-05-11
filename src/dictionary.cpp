#include <assert.h>
#include <fileioc.h>
#include <string.h>

#include "ccdbg/ccdbg.h"
#include "dictionary.h"


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


bool dictionary_IsAppvarPresent()
{
  ti_var_t slot;

  if ((slot = ti_Open(DICTIONARY_APPVAR, "r")))
  {
    ti_Close(slot);
    return true;
  }

  return false;
}


bool dictionary_ArchiveAppvar()
{
  bool is_archived = false;
  ti_var_t slot;

  if ((slot = ti_Open(DICTIONARY_APPVAR, "r")))
  {
    if (ti_IsArchived(slot) == 0)
    {
      if (ti_SetArchiveStatus(true, slot) != 0)
        is_archived = true;
    }
    else
    {
      is_archived = true;
    }

    ti_Close(slot);
  }

  return is_archived;
}


// ============================================================================
// CLASS Dictionary FUNCTION DEFINITIONS
// ============================================================================


Dictionary::Dictionary()
{
  ti_var_t slot;

  this->dictionary_loaded = false;

  if ((slot = ti_Open(DICTIONARY_APPVAR, "r")))
  {
    ti_Read(&num_words, sizeof num_words, 1, slot);
    this->word_list = (word_string_t*)ti_GetDataPtr(slot);

    ti_Close(slot);
    this->dictionary_loaded = true;
  }

  return;
}


Dictionary::~Dictionary()
{
  return;
}


char* Dictionary::operator[](IN uint24_t index) const
{
  assert(this->dictionary_loaded);
  assert(index < this->num_words);

  return this->word_list[index];
}


bool Dictionary::is_loaded() const
{
  return dictionary_loaded;
}


uint24_t Dictionary::get_num_words() const
{
  return this->num_words;
}


bool Dictionary::contains_word(IN Word& word) const
{
  assert(this->dictionary_loaded);

  Word current_word;
  int24_t low = 0;
  int24_t mid;
  int24_t high = num_words - 1;

  while (low <= high)
  {
    mid = (high + low) / 2;
    current_word = *(this->word_list + mid);

    if (current_word < word)
    {
      low = mid + 1;
    }
    else if (current_word > word)
    {
      high = mid - 1;
    }
    else
    {
      return true;
    }
  }

  return false;
}


void Dictionary::get_random_word(OUT Word& random_word) const
{
  assert(this->dictionary_loaded);

  random_word = this->word_list[rand() % this->num_words];
  return;
}


void Dictionary::get_random_word_unlike_given_word(
  IN Word& given_word, OUT Word& random_word
) const
{
  assert(this->dictionary_loaded);

  Word word;

  do {
    word = this->word_list[rand() % this->num_words];
  } while (word.is_similar_to_word(given_word));

  random_word = word;
  return;
}


void Dictionary::get_random_word_that_fits_pattern(
  IN WordPattern& pattern, OUT Word& random_word
) const
{
  assert(this->dictionary_loaded);

  const uint24_t STARTING_INDEX = rand() % this->num_words;
  const uint8_t MAX_NUM_WORD_CHOICES = 50;

  word_string_t word_choices[MAX_NUM_WORD_CHOICES];
  Word word;
  uint24_t index = STARTING_INDEX;
  uint8_t num_word_choices = 0;

  while (num_word_choices < MAX_NUM_WORD_CHOICES && index < this->num_words)
  {
    word = this->word_list[index];

    if (pattern.matches_word(word))
    {
      memcpy(
        word_choices[num_word_choices], this->word_list[index], WORD_LENGTH
      );
      num_word_choices++;
    }

    index++;

    if (index == this->num_words)
      index = 0;

    if (index == STARTING_INDEX)
      break;
  }

  random_word = word_choices[rand() % num_word_choices];
  return;
}
