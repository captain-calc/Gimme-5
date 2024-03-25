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
      RUSH_GAMEPLAY        = (1 << 3)
    };

    static bool must_show_help_for(IN HELP_FLAG help_flag);

  private:
    const uint8_t VERSION = 1;
    static InGameHelp instance;
    static uint8_t shown_help_flags;

    InGameHelp();
    ~InGameHelp();
    uint8_t get_shown_help_flags_from_appvar();
};


#endif
