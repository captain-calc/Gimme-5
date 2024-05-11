#ifndef DICTIONARY_H
#define DICTIONARY_H


#include "typehints.h"
#include "word.h"


// ============================================================================
// DEFINITIONS
// ============================================================================


#define DICTIONARY_APPVAR ("GIMME5D")


// ============================================================================
// PUBLIC FUNCTION DECLARATIONS
// ============================================================================


bool dictionary_IsAppvarPresent();
bool dictionary_ArchiveAppvar();


// ============================================================================
// CLASS DECLARATIONS
// ============================================================================


class Dictionary
{
  public:
    Dictionary();
    ~Dictionary();

    char* operator[](IN uint24_t index) const;
    bool is_loaded() const;
    uint24_t get_num_words() const;
    bool contains_word(IN Word& word) const;
    void get_random_word(OUT Word& random_word) const;
    void get_random_word_unlike_given_word(
      IN Word& given_word, OUT Word& random_word
    ) const;
    void get_random_word_that_fits_pattern(
      IN WordPattern& pattern, OUT Word& random_word
    ) const;

  private:
    bool dictionary_loaded;
    word_string_t* word_list;
    uint24_t num_words;
};


#endif
