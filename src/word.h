#ifndef WORD_H
#define WORD_H


#include <stdint.h>

#include "typehints.h"

#define WORD_LENGTH (5)
typedef char word_string_t[WORD_LENGTH];

enum evaluation_code_t : uint8_t
{
  UNEVALUATED = 0,
  POSITION_AND_LETTER_INCORRECT,
  LETTER_CORRECT,
  POSITION_AND_LETTER_CORRECT
};

typedef evaluation_code_t word_evaluation_t[WORD_LENGTH];


class Word
{
  public:
    Word();
    Word(IN word_string_t string);
    ~Word();

    void operator =(IN word_string_t string);
    char operator [](IN uint8_t index) const;
    bool operator <(IN Word& word) const;
    bool operator >(IN Word& word) const;
    bool operator ==(IN Word& word) const;
    bool operator !=(IN Word& word) const;

    bool is_empty() const;
    bool is_similar_to_word(IN Word& word) const;
    bool is_anagram_of_word(IN Word& word) const;

    void copy_into_string(OUT word_string_t string) const;
    void generate_letter_signature(OUT word_string_t signature) const;

  private:
    word_string_t letters;
};


class WordPattern
{
  public:
    static const char WILDCARD_CHARACTER = '*';

    WordPattern();

    void set_pattern(IN word_string_t pattern_string);

    bool is_valid_pattern() const;
    bool is_all_wildcards() const;
    bool matches_word(IN Word& word) const;

  private:
    word_string_t pattern_string;
};


void word_EvaluateFirstWordBySecondWord(
  IN Word& first_word, IN Word& second_word, OUT word_evaluation_t evaluation
);
bool word_DoesEvaluationShowAllCorrect(IN word_evaluation_t evaluation);


#endif
