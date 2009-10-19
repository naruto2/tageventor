all: Debug Release

release_binary = bin/Release/tagEventor
debug_binary = bin/Debug/tagEventor

debug_library = lib/Debug/libtagReader.a
release_library = lib/Release/libtagReader.a

debug_objects = obj/Debug/tagEventor.o obj/Debug/rulesTable.o
release_objects = obj/Release/tagEventor.o obj/Release/rulesTable.o

##### Get revision number of the version we're compiling
rev_string = $(shell, svnversion)
version_string = "0.0.0"

##### Compile Flags
cc_flags = -Wall -I . -I /usr/include/PCSC -I lib \
           -DPROGRAM_NAME="tagEventor" \
           -DDEFAULT_COMMAND_DIR='"/etc/tagEventor"'

debug_cc_flags = $(cc_flags) -DDEBUG -g \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Debug"'

release_cc_flags = $(cc_flags) \
                 -DVERSION_STRING='$(version_string) $(rev_string) " Release"'

os = $(shell uname)
ifeq ($(os),Darwin)
	debug_link_flags =   -Llib/Debug   -l tagReader -framework PCSC
	release_link_flags = -Llib/Release -l tagReader -framework PCSC
else
	debug_link_flags =   -Llib/Debug   -l tagReader -l pcsclite
	release_link_flags = -Llib/Release -l tagReader -l pcsclite
endif

cleanDebug:
	@rm -f $(debug_objects) $(debug_binary)
	@echo "tagEventor Debug files cleaned"

Debug: $(debug_binary)

###### Debug version LINK
$(debug_binary): $(debug_objects) $(debug_library)
	@gcc $(debug_objects) $(debug_link_flags) -o $@
	@echo tagEventor Debug version $(rev_string) BUILT $@
	@echo ""

$(debug_library):

###### Debug version COMPILE
obj/Debug/%.o : %.c
	@gcc -c $< $(debug_cc_flags) -o $@
	@echo "Compiling " $<

cleanRelease:
	@rm -f $(release_objects) $(release_binary)
	@echo "tagEventor Release files cleaned"

########## Release version LINK
Release: $(release_binary)

$(release_binary): $(release_library) $(release_objects)
	@gcc $(release_objects) $(release_link_flags) -o $@
	@echo tagEventor Release version $(rev_string) BUILT $@
	@echo ""

$(release_library):

########## Release version COMPILE
obj/Release/%.o : %.c
	@gcc -c $< $(release_cc_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
