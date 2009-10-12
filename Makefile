all: Debug Release

Debug: lib/Debug/libtagReader.a bin/Debug/tagEventor

Release: lib/Release/libtagReader.a bin/Release/tagEventor

###### Build Flags
# The following flags can be used to build the (in development) additional GUI components
# NOTE that for now it's 100% Gtk+ specific and I haven't looked at Gtk+ on non-Linux platforms
# Add these flags to the cflags on the gcc command line with the -D option
# BUILD_SYSTEM_TRAY
# BUILD_CONTROL_PANEL (requires BUILD_SYSTEM_TRAY to be defined to be useful)
# BUILD_ABOUT_DIALOG (can be used with both/either BUILD_SYSTEM_TRAY or BUILD_CONTROL_PANEL)
# BUILD_CONTROL_PANEL_HELP (requires BUILD_CONTROL_PANEL to be defined to be useful)

###### Debug version
#TODO define list of objects some time
bin/Debug/tagEventor: obj/Debug/tagEventor.o obj/Debug/rulesTable.o obj/Debug/aboutDialog.o obj/Debug/rulesEditor.o obj/Debug/rulesEditorHelp.o obj/Debug/systemTray.o
	gcc obj/Debug/tagEventor.o obj/Debug/rulesTable.o `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -Llib/Debug -l tagReader -o $@
	@echo tagEventor Debug version BUILT \(./bin/Debug/tagEventor\)

# The way I have defined the headers and code, tagEventor should never need any Gtk+ stuff
obj/Debug/tagEventor.o: tagEventor.c tagReader.h
	gcc -c tagEventor.c -DDEBUG -Wall -I . -I /usr/include/PCSC -o $@

obj/Debug/rulesTable.o: rulesTable.c
	gcc -c rulesTable.c -DDEBUG -Wall -I . -o $@

obj/Debug/aboutDialog.o: aboutDialog.c
	gcc -c aboutDialog.c -DDEBUG `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Debug/rulesEditor.o: rulesEditor.c
	gcc -c rulesEditor.c -DDEBUG `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Debug/rulesEditorHelp.o: rulesEditorHelp.c
	gcc -c rulesEditorHelp.c -DDEBUG `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Debug/systemTray.o: systemTray.c
	gcc -c systemTray.c -DDEBUG `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

###### Debug version of library
lib/Debug/libtagReader.a: obj/Debug/tagReader.o
	ar rcs $@ $<
	@echo libtagReader BUILT \(.lib/Debug/libtagReader\)

obj/Debug/tagReader.o: tagReader.c  tagReader.h
	gcc -c tagReader.c -Wall -I . -I /usr/include/PCSC -o $@

########## Release version
bin/Release/tagEventor: obj/Release/tagEventor.o obj/Release/rulesTable.o obj/Release/aboutDialog.o obj/Release/rulesEditor.o obj/Release/rulesEditorHelp.o obj/Release/systemTray.o
	gcc obj/Release/tagEventor.o obj/Release/rulesTable.o `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -l pcsclite  -Llib/Release -l tagReader -o $@
	@echo tagEventor Release version BUILT \(./bin/Release/tagEventor\)

# The way I have defined the headers and code, tagEventor should never need any Gtk+ stuff
obj/Release/tagEventor.o: tagEventor.c
	gcc -c tagEventor.c -Wall -I . -I /usr/include/PCSC -o $@

obj/Release/rulesTable.o: rulesTable.c
	gcc -c rulesTable.c -Wall -I . -o $@

obj/Release/aboutDialog.o: aboutDialog.c
	gcc -c aboutDialog.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Release/rulesEditor.o: rulesEditor.c
	gcc -c rulesEditor.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Release/rulesEditorHelp.o: rulesEditorHelp.c
	gcc -c rulesEditorHelp.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

obj/Release/systemTray.o: systemTray.c
	gcc -c systemTray.c `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` -DICON_DIR="/usr/share/app-install/" -Wall -I . -o $@

###### Release version of library
lib/Release/libtagReader.a: obj/Release/tagReader.o
	ar rcs $@ $<
	@echo libtagReader BUILT \(.lib/Release/libtagReader\)

obj/Release/tagReader.o: tagReader.c
	gcc -c tagReader.c -Wall -I . -I /usr/include/PCSC -o $@

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean:
	rm -f  *~ *.o *.a obj/Debug/*.o lib/Debug/*.a bin/Debug/* obj/Release/*.o lib/Release/*.a bin/Release/* scripts/*~
