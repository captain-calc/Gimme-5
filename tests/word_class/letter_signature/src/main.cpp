#include <string.h>

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
  Word word("STONE");
  word_string_t signature;

  word.generate_letter_signature(signature);

  if (strncmp("ENOST", signature, WORD_LENGTH) == 0)
  {
    return true;
  }
  
  return false;
}
