#ifndef DICTIONARY_H
#define DICTIONARY_H


#include "typehints.h"
#include "word.h"


#define DICTIONARY_APPVAR ("GIMME5D")


bool dictionary_IsAppvarPresent();
bool dictionary_ArchiveAppvar();


class Dictionary
{
  public:
    Dictionary();
    ~Dictionary();

    bool is_loaded() const;
    bool contains_word(IN Word& word) const;
    void get_random_word(OUT Word& random_word) const;
    void get_random_word_unlike_given_word(
      IN Word& given_word, OUT Word& random_word
    ) const;
    void get_random_word_that_fits_pattern(
      IN WordPattern& pattern, OUT Word& random_word
    ) const;

  private:
    static const uint8_t MAX_NUM_PATTERN_MATCHES = 50;
    bool dictionary_loaded;
    word_string_t* word_list;
    uint24_t num_words;
};


#endif
