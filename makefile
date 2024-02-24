# ----------------------------
# Makefile Options
# ----------------------------

NAME = GIMME5
VERSION = "Pre-Release Beta"
ICON = icon.png
DESCRIPTION = "Gimme 5: All Stars "$(VERSION)
COMPRESSED = YES
ARCHIVED = NO

HAS_PRINTF = NO

CFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=$(VERSION)
CXXFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=$(VERSION)

# ----------------------------

include $(shell cedev-config --makefile)
