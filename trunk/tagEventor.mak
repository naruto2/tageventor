release_binaries = bin/Release/tagEventor
debug_binaries = bin/Debug/tagEventor
binaries = $(release_binaries) $(debug_binaries)

all: Debug Release

debug_objects = obj/Debug/tagEventor.o obj/Debug/rulesTable.o
release_objects = obj/Release/tagEventor.o obj/Release/rulesTable.o

flags = -Wall -I . -I /usr/include/PCSC -DPROGRAM_NAME="tagEventor"

debug_flags = $(flags) -DDEBUG

release_flags = $(flags)

link_flags = -l pcsclite  -Llib/Debug -l tagReader

cleanDebug:
	@rm -f $(debug_objects) $(debug_binaries)
	@echo "tagEventor Debug files cleaned"

Debug: bin/Debug/tagEventor

###### Debug version LINK
bin/Debug/tagEventor: lib/Debug/libtagReader.a $(debug_objects)
	@gcc $(debug_objects) $(link_flags) -o $@
	@echo tagEventor Debug version BUILT \(./bin/Debug/tagEventor\)
	@echo ""

lib/Debug/libtagReader.a:

###### Debug version COMPILE
obj/Debug/%.o : %.c
	@gcc -c $< $(debug_flags) -o $@
	@echo "Compiling " $<

########## Release version LINK
cleanRelease:
	@rm -f $(release_objects) $(release_binaries)
	@echo "tagEventor Release files cleaned"

Release: bin/Release/tagEventor

bin/Release/tagEventor: lib/Release/libtagReader.a $(release_objects)
	@gcc $(release_objects) $(link_flags) -o $@
	@echo tagEventor Release version BUILT \(./bin/Release/tagEventor\)
	@echo ""

lib/Release/libtagReader.a:

########## Release version COMPILE
obj/Release/%.o : %.c
	@gcc -c $< $(release_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
