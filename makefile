# ----------------------------
# Makefile Options
# ----------------------------

NAME = GIMME5
VERSION = "1.0.0A"
ICON = icon.png
DESCRIPTION = "Gimme 5: All Stars v"$(VERSION)
COMPRESSED = YES
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=\"$(VERSION)\"
CXXFLAGS = -Wall -Wextra -Oz -DEXTERN__PROGRAM_VERSION=\"$(VERSION)\"

# ----------------------------

include $(shell cedev-config --makefile)
