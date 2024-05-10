#ifndef INGAME_HELP_H
#define INGAME_HELP_H


#include <stdbool.h>
#include <stdint.h>

#include "typehints.h"


class InGameHelp
{
  public:
    enum HELP_FLAG : uint8_t
    {
      MAIN_MENU            = (1 << 0),
      WORD_CODE_ENTRY_MENU = (1 << 1),
      ORIGINAL_GAMEPLAY    = (1 << 2),
      RUSH_GAMEPLAY        = (1 << 3),
      ANAGRAM_GAMEPLAY     = (1 << 4)
    };

    static bool must_show_help_for(IN HELP_FLAG help_flag);

  private:
    const uint8_t LATEST_VERSION = 2;
    static InGameHelp instance;
    static uint8_t version;
    static uint8_t shown_help_flags;

    InGameHelp();
    ~InGameHelp();
    bool load_from_appvar();
    void load_defaults();
};


#endif
