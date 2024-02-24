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
  Word word("ABACK");
  Word greater_word("BRAVE");
  return (word < greater_word);
}
