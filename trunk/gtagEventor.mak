debug_binaries = bin/Debug/gtagEventor
release_binaries = bin/Release/gtagEventor
binaries = $(debug_binaries) $(release_binaries)

debug_objects = obj/Debug/gtagEventor.o obj/Debug/rulesTable.o obj/Debug/aboutDialog.o obj/Debug/rulesEditor.o obj/Debug/rulesEditorHelp.o obj/Debug/settingsDialog.o obj/Debug/systemTray.o

release_objects = obj/Release/gtagEventor.o obj/Release/rulesTable.o obj/Release/aboutDialog.o obj/Release/rulesEditor.o obj/Release/rulesEditorHelp.o obj/Release/settingsDialog.o obj/Release/systemTray.o

all:Debug Release
	@echo ""

###### Build Flags
build_flags = -DBUILD_SYSTEM_TRAY \
              -DBUILD_CONTROL_PANEL \
              -DBUILD_ABOUT_DIALOG \
              -DBUILD_CONTROL_PANEL_HELP \
              -DBUILD_SETTINGS_DIALOG \
              -DBUILD_RULES_EDITOR \
              -DBUILD_RULES_EDITOR_HELP \
              -DDEFAULT_COMMAND_DIR='"/etc/gtagEventor"'

icon_flags = -DICON_INSTALL_DIR='"/usr/share/gtagEventor/icons/"' \
             -DICON_NAME_CONNECTED='"gtagEventor"' \
             -DICON_NAME_NOT_CONNECTED='"gtagEventorNoReader"'

os = $( shell uname )
ifeq ($(os),Darwin)
	architecture = $(shell arch)
	linker = /usr/bin/gcc
	link_flags = -framework PCSC \
                     -L/Library/Frameworks/GLib.framework/Libraries -lglib-2.0.0 -lgobject-2.0.0 \
                     -L/Library/Frameworks/Cairo.framework/Libraries -lcairo.2 \
                     -L/Library/Frameworks/Gtk.framework/Libraries -lgtk-quartz-2.0.0 \
                     -arch $(architecture) -lc  $(common_link_flags)
	os_cc_flags = -I /Library/Frameworks/GLib.framework/Headers/ \
                      -I /Library/Frameworks/Cairo.framework/Headers/ \
                      -I /Library/Frameworks/Gtk.framework/Headers/
else
	linker = gcc
	link_flags =  `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` \
                      -l tagReader -l pcsclite 
	os_cc_flags = `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` \
                      -I /usr/include/PCSC
endif

##### Get revision number of the version we're compiling
rev_string = $(shell, svnversion)
version_string = "0.0.0"

##### Compile flahs
cc_flags = $(build_flags) $(icon_flags) -I . -I lib/ -Wall \
          -DPROGRAM_NAME='"gtagEventor"' $(os_cc_flags)

debug_cc_flags = $(cc_flags) -DDEBUG -g \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Debug"'

release_cc_flags = $(cc_flags) \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Release"'

cleanDebug:
	@rm -f $(debug_binaries) $(debug_objects)
	@echo "gtagEventor Debug files cleaned"

Debug: bin/Debug/gtagEventor

###### Debug version LINK
bin/Debug/gtagEventor: lib/Debug/libtagReader.a $(debug_objects)
	@$(linker)  $(debug_objects) -Llib/Debug $(link_flags) -o $@
	@echo gtagEventor Debug version BUILT $@
	@echo ""

lib/Debug/libtagReader.a:

###### Debug version COMPILE
obj/Debug/gtagEventor.o : tagEventor.c
	gcc -c $< $(debug_cc_flags) -o $@
	@echo "Compiling " $< "---->" $@

obj/Debug/%.o : %.c
	@gcc -c $< $(debug_cc_flags) -o $@
	@echo "Compiling " $<

cleanRelease:
	@rm -f $(release_binaries) $(release_objects)
	@echo "gtagEventor Release files cleaned"

Release: bin/Release/gtagEventor

########## Release version LINK
bin/Release/gtagEventor: lib/Release/libtagReader.a $(release_objects)
	@$(linker)  $(release_objects) -Llib/Release $(link_flags) -o $@
	@echo gtagEventor Release version BUILT $@
	@echo ""

lib/Release/libtagReader.a:

########## Release version COMPILE
obj/Release/gtagEventor.o : tagEventor.c
	@gcc -c $< $(release_cc_flags) -o $@
	@echo "Compiling " $< "---->" $@

obj/Release/%.o : %.c
	@gcc -c $< $(release_cc_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
	@echo ""
