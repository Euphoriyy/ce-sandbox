# ----------------------------
# Makefile Options
# ----------------------------

NAME = SANDBOX
ICON = icon.png
DESCRIPTION = "Sandbox Game for the TI-84 Plus CE"
COMPRESSED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

.PHONY: compile_commands.json

compile_commands.json:
	bear -- make all
	sed -i '/fforce-mangle/d' ./compile_commands.json
	sed -i '/profile-guided/d' ./compile_commands.json

# ----------------------------

include $(shell cedev-config --makefile)
