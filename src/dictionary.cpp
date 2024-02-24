#include <assert.h>
#include <fileioc.h>

#include "ccdbg/ccdbg.h"
#include "dictionary.h"


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


Dictionary::Dictionary()
{
  ti_var_t slot;

  dictionary_loaded = false;

  if ((slot = ti_Open(DICTIONARY_APPVAR, "r")))
  {
    ti_Read(&num_words, sizeof num_words, 1, slot);
    word_list = (word_string_t*)ti_GetDataPtr(slot);

    ti_Close(slot);
    dictionary_loaded = true;
  }

  return;
}


Dictionary::~Dictionary()
{
  return;
}


bool Dictionary::is_loaded() const
{
  return dictionary_loaded;
}


bool Dictionary::contains_word(IN Word& word) const
{
  assert(dictionary_loaded);

  Word current_word;
  int24_t low = 0;
  int24_t mid;
  int24_t high = num_words - 1;

  while (low <= high)
  {
    mid = (high + low) / 2;
    current_word = *(word_list + mid);

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
  assert(dictionary_loaded);

  random_word = word_list[rand() % num_words];
  return;
}


void Dictionary::get_random_word_unlike_given_word(
  IN Word& given_word, OUT Word& random_word
) const
{
  assert(dictionary_loaded);

  Word word;

  do {
    word = word_list[rand() % num_words];
  } while (word.is_similar_to_word(given_word));

  random_word = word;
  return;
}


void Dictionary::get_random_word_that_fits_pattern(
  IN WordPattern& pattern, OUT Word& random_word
) const
{
  assert(dictionary_loaded);

  const uint24_t STARTING_INDEX = rand() % num_words;

  Word word;
  uint24_t index = STARTING_INDEX;

  while (true)
  {
    word = word_list[index];

    if (pattern.matches_word(word))
      break;

    index++;

    if (index == num_words)
      index = 0;

    if (index == STARTING_INDEX)
      break;
  }

  random_word = word;
  return;
}
