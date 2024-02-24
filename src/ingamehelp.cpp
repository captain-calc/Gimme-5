#include <fileioc.h>

#include "ingamehelp.h"


#define HELP_APPVAR ("GIMME5S")


typedef struct
{
  uint8_t version;
  uint8_t shown_help_flags;
} appvar_data_t;


InGameHelp InGameHelp::instance;
uint8_t InGameHelp::shown_help_flags;


bool InGameHelp::must_show_help_for(IN HELP_FLAG help_flag)
{
  bool must_show = !(shown_help_flags & help_flag);

  shown_help_flags |= help_flag;
  return must_show;
}


InGameHelp::InGameHelp()
{
  shown_help_flags = get_shown_help_flags_from_appvar();
  return;
}


InGameHelp::~InGameHelp()
{
  const appvar_data_t DATA = {
    .version = VERSION,
    .shown_help_flags = shown_help_flags
  };

  ti_var_t slot;

  if ((slot = ti_Open(HELP_APPVAR, "w")))
  {
    ti_Write(&DATA, sizeof DATA, 1, slot);
    ti_Close(slot);
  }

  return;
}


uint8_t InGameHelp::get_shown_help_flags_from_appvar()
{
  ti_var_t slot;
  appvar_data_t data;

  if ((slot = ti_Open(HELP_APPVAR, "r")))
  {
    ti_Read(&data, sizeof data, 1, slot);
    ti_Close(slot);
  }

  return data.shown_help_flags;
}
