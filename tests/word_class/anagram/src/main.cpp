#include "../../../../src/word.h"
#include "../../../test_utils.h"


static bool test(void);


int main(void)
{
  testutil_PrintTestSetup();
  testutil_PrintTestResults(test());
  return 0;
}


static bool test(void)
{
  Word word("LOOPS");
  Word word_two("SLOOP");

  return word.is_anagram_of_word(word_two);
}
