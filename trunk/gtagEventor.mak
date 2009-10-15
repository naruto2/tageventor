debug_binaries = bin/Debug/gtagEventor
release_binaries = bin/Release/gtagEventor
binaries = $(debug_binaries) $(release_binaries)

debug_objects = obj/Debug/gtagEventor.o obj/Debug/rulesTable.o obj/Debug/aboutDialog.o obj/Debug/rulesEditor.o obj/Debug/rulesEditorHelp.o obj/Debug/settingsDialog.o obj/Debug/systemTray.o

release_objects = obj/Release/gtagEventor.o obj/Release/rulesTable.o obj/Release/aboutDialog.o obj/Release/rulesEditor.o obj/Release/rulesEditorHelp.o obj/Release/settingsDialog.o obj/Release/systemTray.o

all:Debug Release

###### Build Flags
# Add these flags to the cflags on the gcc command line with the -D option
# BUILD_SYSTEM_TRAY
# BUILD_CONTROL_PANEL (requires BUILD_SYSTEM_TRAY to be defined to be useful)
# BUILD_ABOUT_DIALOG (can be used with both/either BUILD_SYSTEM_TRAY or BUILD_CONTROL_PANEL)
# BUILD_CONTROL_PANEL_HELP (requires BUILD_CONTROL_PANEL to be defined to be useful)
# BUILD_SETTINGS_DIALOG
# BUILD_RULES_EDITOR
# BUILD_RULES_EDITOR_HELP
build_flags = -DBUILD_SYSTEM_TRAY -DBUILD_CONTROL_PANEL -DBUILD_ABOUT_DIALOG -DBUILD_CONTROL_PANEL_HELP -DBUILD_SETTINGS_DIALOG -DBUILD_RULES_EDITOR -DBUILD_RULES_EDITOR_HELP -DPROGRAM_NAME='"gtagEventor"'

flags = $(build_flags) -I . -Wall -DICON_DIR="/usr/share/app-install/"  -I /usr/include/PCSC `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0`

debug_flags = $(flags) -DDEBUG

release_flags = $(flags)

cleanDebug:
	@rm -f $(debug_binaries) $(debug_objects)
	@echo "gtagEventor Debug files cleaned"

Debug: bin/Debug/gtagEventor

###### Debug version LINK
bin/Debug/gtagEventor: lib/Debug/libtagReader.a $(debug_objects)
	@gcc $(debug_objects) `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -Llib/Debug -l tagReader -o $@
	@echo gtagEventor Debug version BUILT \(./bin/Debug/gtagEventor\)
	@echo ""

lib/Debug/libtagReader.a:

###### Debug version COMPILE
obj/Debug/gtagEventor.o : tagEventor.c
	@gcc -c $< $(debug_flags) -o $@
	@echo "Compiling " $< "---->" $@

obj/Debug/%.o : %.c
	@gcc -c $< $(debug_flags) -o $@
	@echo "Compiling " $<

cleanRelease:
	@rm -f $(release_binaries) $(release_objects)
	@echo "gtagEventor Release files cleaned"

Release: bin/Release/gtagEventor

########## Release version LINK
bin/Release/gtagEventor: lib/Release/libtagReader.a $(release_objects)
	@gcc $(release_objects) `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -Llib/Release -l tagReader -o $@
	@echo gtagEventor Release version BUILT \(./bin/Release/gtagEventor\)
	@echo ""

lib/Release/libtagReader.a:

########## Release version COMPILE
obj/Release/gtagEventor.o : tagEventor.c
	@gcc -c $< $(release_flags) -o $@
	@echo "Compiling " $<

obj/Release/%.o : %.c
	@gcc -c $< $(release_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
