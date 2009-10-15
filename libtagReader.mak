debug_archive = lib/Debug/libtagReader.a
release_archive = lib/Release/libtagReader.a
archives = $(debug_archive) $(release_archive)

all: Debug Release

debug_objects = lib/Debug/tagReader.o

release_objects = lib/Release/tagReader.o

flags = -Wall -I . -I /usr/include/PCSC

debug_flags = $(flags) -DDEBUG -g

release_flags = $(flags)

###### Debug version of library
cleanDebug:
	@rm -f $(debug_objects) $(debug_archive)
	@echo "libtagReader Debug files cleaned"

Debug: $(debug_archive)

lib/Debug/libtagReader.a: $(debug_objects)
	@ar rcs $@ $<
	@echo libtagReader Debug version BUILT \(./lib/Debug/libtagReader\)
	@echo ""

lib/Debug/tagReader.o: tagReader.c  tagReader.h
	@gcc -c tagReader.c $(debug_flags) -o $@
	@echo "Compiling " $<

###### Release version of library
cleanRelease:
	@rm -f $(release_objects) $(release_archive)
	@echo "libtagReader Release files cleaned"

Release: $(release_archive)

lib/Release/libtagReader.a: $(release_objects)
	@ar rcs $@ $<
	@echo libtagReader Release version BUILT \(./lib/Release/libtagReader\)
	@echo ""

lib/Release/tagReader.o: tagReader.c
	@gcc -c tagReader.c $(release_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
