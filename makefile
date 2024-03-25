# ----------------------------
# Makefile Options
# ----------------------------

NAME = GIMME5
VERSION = "Pre-Release Beta 2"
ICON = icon.png
DESCRIPTION = "Gimme 5: All Stars "$(VERSION)
COMPRESSED = YES
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=\"$(VERSION)\"
CXXFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=\"$(VERSION)\"

# ----------------------------

include $(shell cedev-config --makefile)
