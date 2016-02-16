## PCSC Library header file including ##

### Background ###
It is assumed that a library .a archive and it's corresponding .h external header files for applications to be able to link to it are stored in system library directories, such as /usr/lib and /usr/include.

For header associated with libraries found in the system include directories such as "stdio.h" it is normal to include it with the following pre-processor directive:

#include <stdio.h>

as "stdio.h" is in the standard header file directory (/usr/include/) expected by the GCC compiler.

In the case of pcsc-lite, the header files are in
/usr/include/PCSC/

These can be found by the compiler when source files include them thus

#include <PCSC/winscard.h>

as the "PCSC" directory is under the standard header file directory (/usr/include/) expected by the GCC compiler.

### What the Bible says ###
Kernigan & Ritchie's book on the C language, section "4.11.1 File Inclusion" states:

"Any source line of the form
#include "filename"
or
#include 

&lt;filename&gt;


is replaced by the contents of the file filename.

If the filename is quoted, searching for the file typically begins where the source program was found;
if it is not found there, or if the name is enclosed in < and >, searching follows an implementation-defined rule to find the file."

and also in "A.12.4 File Inclusion
A control line of the form
# include 

&lt;filename&gt;


causes the replacement of that line by the entire contents of the file filename. The characters in
the name filename must not include > or newline, and the effect is undefined if it contains any
of ", ', \, or /**. The named file is searched for in a sequence of implementation-defined
places.
Similarly, a control line of the form
# include "filename"
searches first in association with the original source file (a deliberately implementation dependent
phrase), and if that search fails, then as in the first form."**

Thus for applications compiling against a library, it is normal for the application to include it's own header files using #include "", and the library's header files using #include <>.

Thus, applications compiling against a library, such as pcsc-lite, that has its header files in a subdirectory of /usr/include are expected to include the header file thus:
#include <PCSC/winscard.h>

### PCSC Lite Implementation ###
However, when doing so the following compile error occurs

"gcc -c tagReader.c -Wall -I . -o obj/Debug/tagReader.o
In file included from tagReader.h:20,
> from tagReader.c:26:
/usr/include/PCSC/winscard.h:19:22: error: pcsclite.h: No such file or directory
In file included from tagReader.h:20,
> from tagReader.c:26:"

Inspection reveals that this is because winscard.h uses
"#include <pcsclite.h>"
without the PCSC directory prefix

This error can be avoided by modifying "winscard.h" to include "pcsclite.h" thus

"#include <PCSC/pcsclite.h>

which also then requires modifying the line in pcsclite.h "#include <wintypes.h>"
to be "#include <PCSC/wintypes.h>"

These modifications leave the #include pattern of PCSC thus:
(include files from the same library in the same directory shown with <<<<<<<<<<<<<)

ifdhandler.h:
  1. nclude <PCSC/pcsclite.h>          <<<<<<<<<<<<<<<<<<<
pcsclite.h:
  1. nclude <PCSC/wintypes.h>          <<<<<<<<<<<<<<<<<<<
  1. nclude <PCSC/winscard.h>          <<<<<<<<<<<<<<<<<<<
reader.h:
  1. nclude <inttypes.h>
winscard.h:
  1. nclude <PCSC/pcsclite.h>          <<<<<<<<<<<<<<<<<<<
wintypes.h:
  1. nclude <windows.h>

allowing compilation from an application directory to work as expected, with no PCSC specific modifications made to the Makefile.

### Other library implementations ###
Looking at other library implementations (I chose at random libglade, in /usr/include/libglade-2.0/glade/) we see
glade-build.h:
  1. nclude <glib.h>
  1. nclude <glib-object.h>
  1. nclude <gmodule.h>
  1. nclude <glade/glade-xml.h>       <<<<<<<<<<<<<<<<<<<<
  1. nclude <gtk/gtk.h>
  1. nclude <glade/glade-parser.h>    <<<<<<<<<<<<<<<<<<<<
glade.h:
  1. nclude <glib.h>
  1. nclude <glade/glade-init.h>	   <<<<<<<<<<<<<<<<<<<<
  1. nclude <glade/glade-xml.h>       <<<<<<<<<<<<<<<<<<<<
glade-init.h:
  1. nclude <glib.h>
glade-parser.h:
  1. nclude <glib.h>
  1. nclude <gdk/gdk.h>
glade-xml.h:
  1. nclude <glib.h>
  1. nclude <gtk/gtk.h>

Which matches the proposed changes above, and avoids the need for the application programmer to know each library header file subdirectory and to modify Makefiles, this knowledge is retained in the source files only.

### The Workaround ###
I've added the switch "-I /usr/include/PCSC" in the make file to make sure it always finds the PCSC header files, no matter how included.