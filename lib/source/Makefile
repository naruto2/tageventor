debug_archive = ../Debug/libtagReader.a
release_archive = ../Release/libtagReader.a

external_headers = tagReader.h

all: Debug Release

debug_objects = ../obj/Debug/tagReader.o ../obj/Debug/acr122UDriver.o
release_objects = ../obj/Release/tagReader.o ../obj/Release/acr122UDriver.o

os = $( shell uname )
ifeq ($(os),Darwin)
	architecture = $(shell arch)
	archiver = /usr/bin/libtool
	archiver_flags = -arch_only $(architecture) -o
	os_cc_flags =
#	os_cc_flags = -arch $(architecture)
else
	archiver = ar
	archiver_flags = rcsv
	os_cc_flags =
endif

cc_flags = $(os_cc_flags) -Wall -I . -I /usr/include/PCSC
debug_flags = $(os_cc_flags) $(cc_flags) -DDEBUG -g
release_flags = $(os_cc_flags) $(cc_flags)

###### Debug version of library
cleanDebug:
	@rm -f $(debug_objects) $(debug_archive)
	@rm -f ../$(external_headers)
	@echo "libtagReader Debug files cleaned"

Debug: $(debug_archive)

$(debug_archive): $(debug_objects)
	@echo "Creating library..."
	@$(archiver) $(archiver_flags) $@ $(debug_objects)
	@cp -f $(external_headers) ../
	@echo Library BUILT $@
	@echo ""

../obj/Debug/%.o: %.c
	@gcc -c $< $(debug_flags) -o $@
	@echo "Compiling " $<

###### Release version of library
cleanRelease:
	@rm -f $(release_objects) $(release_archive)
	@rm -f ../$(external_headers)
	@echo "libtagReader Release files cleaned"

Release: $(release_archive)

$(release_archive): $(release_objects)
	@echo "Creating library..."
	@$(archiver) $(archiver_flags) $@ $(release_objects)
	@cp -f $(external_headers) ../
	@echo Library BUILT $@
	@echo ""

../obj/Release/%.o: %.c
	@gcc -c $< $(release_flags) -o $@
	@echo "Compiling " $<

# Clean up all stray editor back-up files, any .o or .a left around in this directory
# Remove all built object files (.o and .a) and compiled and linked binaries
clean: cleanDebug cleanRelease
	@rm -f *~
	@echo ""
