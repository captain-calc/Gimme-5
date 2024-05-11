#include <assert.h>
#include <string.h>

#include "word.h"


// ============================================================================
// CLASS Word FUNCTION DEFINITIONS
// ============================================================================


Word::Word()
{
  memset(letters, '\0', WORD_LENGTH);
  return;
}


Word::Word(IN word_string_t string)
{
  memcpy(letters, string, WORD_LENGTH);
  return;
}


Word::~Word()
{
  return;
}


void Word::operator =(IN word_string_t string)
{
  memcpy(letters, string, WORD_LENGTH);
  return;
}


char Word::operator [](IN uint8_t index) const
{
  assert(index < WORD_LENGTH);

  return letters[index];
}


bool Word::operator <(IN Word& word) const
{
  int8_t result = strncmp(letters, word.letters, WORD_LENGTH);

  return (result < 0 ? true : false);
}


bool Word::operator >(IN Word& word) const
{
  int8_t result = strncmp(letters, word.letters, WORD_LENGTH);

  return (result > 0 ? true : false);
}


bool Word::operator ==(IN Word& word) const
{
  int8_t result = strncmp(letters, word.letters, WORD_LENGTH);

  return (result == 0 ? true : false);
}


bool Word::operator !=(IN Word& word) const
{
  if ((*this) == word)
    return false;

  return true;
}


bool Word::is_empty() const
{
  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (letters[index] != '\0')
      return false;
  }

  return true;
}


bool Word::is_similar_to_word(IN Word& word) const
{
  uint8_t index = 0;
  uint8_t num_letters_shared = 0;

  while (index < WORD_LENGTH)
  {
    if (letters[index] == word.letters[index])
    {
      num_letters_shared++;

      if (num_letters_shared == 3)
        return true;

      if (index >= 1 && (letters[index - 1] == word.letters[index - 1]))
        return true;
    }

    index++;
  }

  return false;
}


bool Word::is_anagram_of_word(IN Word& word) const
{
  word_string_t this_word_signature;
  word_string_t given_word_signature;

  generate_letter_signature(this_word_signature);
  word.generate_letter_signature(given_word_signature);

  int8_t result = strncmp(
    this_word_signature, given_word_signature, WORD_LENGTH
  );

  return (result == 0 ? true : false);
}


void Word::copy_into_string(OUT word_string_t string) const
{
  memcpy(string, letters, WORD_LENGTH);
  return;
}


void Word::generate_letter_signature(OUT word_string_t signature) const
{
  char letter;
  int8_t subindex;

  memcpy(signature, letters, WORD_LENGTH);

  for (uint8_t index = 1; index < WORD_LENGTH; index++)
  {
    letter = signature[index];
    subindex = index - 1;

    while (subindex >= 0 && signature[subindex] > letter)
    {
      signature[subindex + 1] = signature[subindex];
      subindex--;
    }

    signature[subindex + 1] = letter;
  }

  return;
}


// ============================================================================
// CLASS WordPattern FUNCTION DEFINITIONS
// ============================================================================


WordPattern::WordPattern()
{
  word_string_t default_pattern_string;
  memset(default_pattern_string, this->WILDCARD_CHARACTER, WORD_LENGTH);
  set_pattern(default_pattern_string);
  return;
}


void WordPattern::set_pattern(IN word_string_t pattern_string)
{
  memcpy(this->pattern_string, pattern_string, WORD_LENGTH);

  assert(is_valid_pattern());
  return;
}


bool WordPattern::is_valid_pattern() const
{
  char character;

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    character = this->pattern_string[index];

    if (character != this->WILDCARD_CHARACTER)
    {
      if (character < 'A' || character > 'Z')
        return false;
    }
  }

  return true;
}


bool WordPattern::is_all_wildcards() const
{
  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (pattern_string[index] != this->WILDCARD_CHARACTER)
      return false;
  }

  return true;
}


bool WordPattern::matches_word(IN Word& word) const
{
  word_string_t string;
  char character;

  word.copy_into_string(string);

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    character = pattern_string[index];

    if (
      character != this->WILDCARD_CHARACTER
      && character != string[index]
    )
    {
      return false;
    }
  }

  return true;
}


// ============================================================================
// PUBLIC FUNCTION DEFINITIONS
// ============================================================================


void word_EvaluateFirstWordBySecondWord(
  IN Word& guess, IN Word& target, OUT word_evaluation_t evaluation
)
{
  word_string_t target_string;
  uint8_t subindex;

  target.copy_into_string(target_string);
  memset(evaluation, UNEVALUATED, WORD_LENGTH);

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (target_string[index] == guess[index])
    {
      evaluation[index] = POSITION_AND_LETTER_CORRECT;
      target_string[index] = '\0';
    }
  }

  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    subindex = 0;

    while (subindex < WORD_LENGTH && evaluation[index] == UNEVALUATED)
    {
      if (target_string[subindex] == guess[index])
      {
        evaluation[index] = LETTER_CORRECT;
        target_string[subindex] = '\0';
      }

      subindex++;
    }

    if (evaluation[index] == UNEVALUATED)
    {
      evaluation[index] = POSITION_AND_LETTER_INCORRECT;
    }
  }

  return;
}


bool word_DoesEvaluationShowAllCorrect(IN word_evaluation_t evaluation)
{
  for (uint8_t index = 0; index < WORD_LENGTH; index++)
  {
    if (evaluation[index] != POSITION_AND_LETTER_CORRECT)
    {
      return false;
    }
  }

  return true;
}
