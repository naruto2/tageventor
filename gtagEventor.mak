all:Debug Release
	@echo ""

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
	@echo ""

debug_binaries = bin/Debug/gtagEventor
release_binaries = bin/Release/gtagEventor
binaries = $(debug_binaries) $(release_binaries)

debug_objects = obj/Debug/gtagEventor.o \
                obj/Debug/rulesTable.o  \
                obj/Debug/aboutDialog.o \
                obj/Debug/rulesEditor.o \
                obj/Debug/rulesEditorHelp.o \
                obj/Debug/settingsDialog.o \
                obj/Debug/readersDialog.o \
                obj/Debug/systemTray.o

release_objects = obj/Release/gtagEventor.o \
                  obj/Release/rulesTable.o \
                  obj/Release/aboutDialog.o \
                  obj/Release/rulesEditor.o \
                  obj/Release/rulesEditorHelp.o \
                  obj/Release/settingsDialog.o \
                  obj/Release/readersDialog.o \
                  obj/Release/systemTray.o

debug_dependencies = $(debug_objects:.o=.d)
release_dependencies = $(release_objects:.o=.d)

#dependancy generation and use
include $(debug_dependencies)
include $(release_dependencies)

###### Build Flags
build_flags = -DBUILD_SYSTEM_TRAY \
              -DBUILD_CONTROL_PANEL \
              -DBUILD_ABOUT_DIALOG \
              -DBUILD_CONTROL_PANEL_HELP \
              -DBUILD_SETTINGS_DIALOG \
              -DBUILD_RULES_EDITOR \
              -DBUILD_RULES_EDITOR_HELP \
              -DBUILD_READERS_DIALOG \
              -DDEFAULT_LOCK_FILE_DIR='"/var/run/tagEventor"' \
              -DDAEMON_NAME='"tagEventord"'

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
                      -I /Library/Frameworks/Gtk.framework/Headers/ \
                      -DDEFAULT_COMMAND_DIR='"/Library/Application Support/gtagEventor"'
else
	linker = gcc
	link_flags =  `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` \
                      -l tagReader -l pcsclite
	os_cc_flags = `pkg-config --cflags --libs gtk+-2.0 gmodule-2.0` \
                      -I /usr/include/PCSC \
                      -DDEFAULT_COMMAND_DIR='"/etc/gtagEventor"'
endif

##### Get revision number of the version we're compiling
rev_string = $(shell, svnversion)
version_string = "0.0.0"

##### Compile flags
cc_flags = $(build_flags) $(icon_flags) -I . -I lib/source -Wall \
          -DPROGRAM_NAME='"gtagEventor"' $(os_cc_flags)

debug_cc_flags = $(cc_flags) -DDEBUG -g \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Debug"'

release_cc_flags = $(cc_flags) \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Release"'

cleanDebug:
	@rm -f $(debug_binaries) $(debug_objects) $(debug_dependencies)rm
	@echo "gtagEventor Debug files cleaned"

Debug: bin/Debug/gtagEventor

###### Debug version LINK
bin/Debug/gtagEventor: lib/Debug/libtagReader.a $(debug_objects)
	@$(linker)  $(debug_objects) -Llib/Debug $(link_flags) -o $@
	@echo gtagEventor Debug version BUILT $@
	@echo ""

lib/Debug/libtagReader.a:

########## Debug version DEPENDENCIES
obj/Debug/%.d: %.c
	@set -e; rm -f $@; \
	gcc -M $(debug_cc_flags) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@D)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

###### Debug version COMPILE
obj/Debug/gtagEventor.o: tagEventor.c
	@gcc -c $< $(debug_cc_flags) -o $@
	@echo "Compiling " $< "---->" $@

obj/Debug/gtagEventor.d: tagEventor.c
	@set -e; rm -f $@; \
	gcc -M $(debug_cc_flags) $< > $@.$$$$; \
	sed 's,tagEventor.o[ :]*,$(@D)/gtagEventor.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/Debug/%.o : %.c
	@gcc -c $< $(debug_cc_flags) -o $@
	@echo "Compiling " $<

cleanRelease:
	@rm -f $(release_binaries) $(release_objects) $(release_dependencies)
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

obj/Release/gtagEventor.d: tagEventor.c
	@set -e; rm -f $@; \
	gcc -M $(debug_cc_flags) $< > $@.$$$$; \
	sed 's,tagEventor.o[ :]*,$(@D)/gtagEventor.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

########## Release version DEPENDENCIES
obj/Release/%.d: %.c
	@set -e; rm -f $@; \
	gcc -M $(release_cc_flags) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@D)/$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

obj/Release/%.o : %.c
	@gcc -c $< $(release_cc_flags) -o $@
	@echo "Compiling " $<
