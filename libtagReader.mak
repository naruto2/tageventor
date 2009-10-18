debug_archive = lib/Debug/libtagReader.a
release_archive = lib/Release/libtagReader.a
archives = $(debug_archive) $(release_archive)

all: Debug Release

debug_objects = lib/Debug/tagReader.o
release_objects = lib/Release/tagReader.o

os = $(shell uname)
ifeq ($(os),Darwin)
	architecture = $(shell arch)
	archiver = /usr/bin/libtool
	archiver_flags = -arch_only $(architecture) -o
	os_cc_flags =
#	os_cc_flags = -arch $(architecture)
else
	archiver = ar
	archiver_flags = rcs
	os_cc_flags =
endif

cc_flags = $(os_cc_flags) -Wall -I . -I /usr/include/PCSC
debug_flags = $(os_cc_flags) $(cc_flags) -DDEBUG -g
release_flags = $(os_cc_flags) $(cc_flags)

###### Debug version of library
cleanDebug:
	@rm -f $(debug_objects) $(debug_archive)
	@echo "libtagReader Debug files cleaned"

Debug: $(debug_archive)

$(debug_archive): $(debug_objects)
	@$(archiver) $(archiver_flags) $@ $<
	@echo libtagReader Debug version BUILT \(./lib/Debug/libtagReader\)
	@echo ""

lib/Debug/%.o: %.c
	@gcc -c $< $(debug_flags) -o $@
	@echo "Compiling " $<

###### Release version of library
cleanRelease:
	@rm -f $(release_objects) $(release_archive)
	@echo "libtagReader Release files cleaned"

Release: $(release_archive)

$(release_archive): $(release_objects)
	@$(archiver) $(archiver_flags) $@ $<
	@echo libtagReader Release version BUILT \(./lib/Release/libtagReader\)
	@echo ""

lib/Release/%.o: %.c
	@gcc -c $< $(release_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
	@echo ""
