#include <fileioc.h>

#include "ingamehelp.h"


#define HELP_APPVAR ("GIMME5S")


// ============================================================================
// VERSION HISTORY FOR InGameHelp CLASS
// ============================================================================
//
// Version 0 (Pre-Release Beta)
//
//    enum HELP_FLAG : uint8_t
//    {
//      MAIN_MENU            = (1 << 0),
//      WORD_CODE_ENTRY_MENU = (1 << 1),
//      GAME                 = (1 << 2)
//    };
//
// Version 1 (Pre-Release Beta 2)
//
//    enum HELP_FLAG : uint8_t
//    {
//      MAIN_MENU            = (1 << 0),
//      WORD_CODE_ENTRY_MENU = (1 << 1),
//      ORIGINAL_GAMEPLAY    = (1 << 2),
//      RUSH_GAMEPLAY        = (1 << 3)
//    };
//
//   `GAME` from v0 became `ORIGINAL_GAMEPLAY` in v1.
//
// Version 2 (Release)
//
//    enum HELP_FLAG : uint8_t
//    {
//      MAIN_MENU            = (1 << 0),
//      WORD_CODE_ENTRY_MENU = (1 << 1),
//      ORIGINAL_GAMEPLAY    = (1 << 2),
//      RUSH_GAMEPLAY        = (1 << 3),
//      ANAGRAM_GAMEPLAY     = (1 << 4)
//    };
//
//   The same parser can be used for for v0, v1, and v2 because each new
// version simply reduced the number of unused bits in `shown_help_flags`.
//


// ============================================================================
// DEFINITIONS
// ============================================================================


typedef struct
{
  uint8_t version;
  uint8_t shown_help_flags;
} appvar_data_t;


// ============================================================================
// CLASS InGameHelp FUNCTION DEFINITIONS
// ============================================================================


InGameHelp InGameHelp::instance;
uint8_t InGameHelp::version;
uint8_t InGameHelp::shown_help_flags;


bool InGameHelp::must_show_help_for(IN HELP_FLAG help_flag)
{
  bool must_show = !(shown_help_flags & help_flag);

  shown_help_flags |= help_flag;
  return must_show;
}


InGameHelp::InGameHelp()
{
  if (!load_from_appvar())
    load_defaults();

  if (this->LATEST_VERSION != this->version)
    load_defaults();

  return;
}


InGameHelp::~InGameHelp()
{
  const appvar_data_t DATA = {
    .version = this->version,
    .shown_help_flags = this->shown_help_flags
  };

  ti_var_t slot;

  if ((slot = ti_Open(HELP_APPVAR, "w")))
  {
    ti_Write(&DATA, sizeof DATA, 1, slot);
    ti_Close(slot);
  }

  return;
}


bool InGameHelp::load_from_appvar()
{
  ti_var_t slot;
  appvar_data_t data;

  if ((slot = ti_Open(HELP_APPVAR, "r")))
  {
    ti_Read(&data, sizeof data, 1, slot);
    ti_Close(slot);

    this->version = data.version;
    this->shown_help_flags = data.shown_help_flags;
    return true;
  }

  return false;
}


void InGameHelp::load_defaults()
{
  this->version = this->LATEST_VERSION;
  this->shown_help_flags = 0;
  return;
}
