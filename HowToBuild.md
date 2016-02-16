Before trying to build (OK, go ahead and try it and then come back here when it fails....!) you might want to check the SoftwareDependencies page to make sure you have what's needed for Linux and Mac OS X building.

### Build from the command line ###
Type "make" at the command prompt in the source directory.
You can use "make clean" then "make" to ensure you have a clean build.

You should get appropriate output from the make file and be left with tagEventor and gtagEventor (Gtk/GNOME GUI version) executables in the ./bin/Debug and ./bin/Release subdirectories.

### Build from the command line on Mac OS X ###
Most people on Mac OS X will not have a Gtk port installed and so for now only building of the command line program is supported directly on Mac OS X.

To avoid errors in the build trying to build the Gtk version, use:
  * make -f tagEventor.mak

### Build Details ###
The project has three somewhat independant builds, each with it's own makefile:
  * Entire project (Makefile)        - build all the sub-projects
  * libReader      (lib/source/Makefile)   - the library for talking to the reader
  * tagEventor     (tagEventor.mak)  - the command line application or daemon
  * gtagEventor    (gtagEventor.mak) - the Gtk/Gnome GUI version of tagEventor

### Targets ###
The makefiles take the following targets:
  * "" (default target) or  "all" - equivalent to "Debug" and "Release"
  * Debug                                  - build the Debug   version
  * Release                                - build the Release version
  * cleanDebug                         - only clean out the generated files of Debug   version
  * cleanRelease                       - only clean out the generated files of Release version

The overall project makefile (Makefile) also accepts the "binary" target which builds "Release" then creates a binary "tarball" for binary instalation.

### Build from Code::Blocks IDE ###
There are projects included for the libReader library, tagEventor command-line program and gtagEventor Gtk/Gnome GUI version for the [Code::Blocks](http://www.codeblocks.org/) Integrated Development Environment (IDE).

Three Code::Blocks projects (.cbp files) exist, unsurprisingly named:
  * libReader    (libReader.cbp)
  * tagEventor   (tagEventor.cbp)
  * gtagEventor  (gtagEventor.cbp)

Open them in [Code::Blocks](http://www.codeblocks.org/), select the target (Debug or Release) and hit "build" or "fresh build"...

The make files are used directly by the [Code::Blocks](http://www.codeblocks.org/) build system, and they are compatible with it, so if you open the corresponding project in Code::Blocks and hit "build" you are doing "make $target" ($target = Debug | Release).

If you do a fresh build [Code::Blocks](http://www.codeblocks.org/) does "make clean$target" followed by "make $target".

### Where is gtagEventor.c ? ###
The **gtagEventor** project reuses the tagEventor.c file but recompiles it with different flags to produce **gtagEventor.o** and then links with the rest of the files.