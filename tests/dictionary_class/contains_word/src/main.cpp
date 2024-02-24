#include "../../../../src/dictionary.h"
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
  Dictionary dictionary;
  Word word("ABBOT");
  
  return dictionary.contains_word(word);
}
