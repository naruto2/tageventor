## Linux ##
This was written based on Ubuntu 9.04. If you detect anything wrong or missing for other distributions then please contribute them!

### Build dependencies for tagEventor ###
To develop against PCSCD, including building tagEventor you will need the packages:
  * libpcsclite1 - the library
  * libpcsclite-dev - the header files

### Build dependencies for gtagEventor ###
If you want to compile the GUI version called gtagEventor then you will need Gtk2-0, GLib and GObjects.
  * libgtk2.0-dev
  * libnotify-dev - for notifications of events that occur in a pop-up window

Projects are included for Code::Blocks IDE so you might want to get that IDE for build and debug.

### Run Time dependencies ###
You'll need the "pcsc-lite" package on your machine which runes the "pcscd" daemon, if not already there (it comes with most Linux distributions and Mac OS X).

It maybe called "pcscd". From the terminal type "ps -ef | grep pcsc" to see if it's there. Best to try that with a reader plugged-in.

The ACS reader will need the **libccid** driver installed. This seems to come with and be loaded by pcscd.

## Mac OS X ##
These were written based on Mac OS X 10.4 (Tiger). I believe they are also accurate for 10.5 and 10.6. If you discover otherwise then please contribute the improvements.

### Build dependencies for tagEventor ###
To develop against PCSCD, including building tagEventor you will need the PCSD framework.

### Build dependencies for gtagEventor ###
To build gtagEventor on Mac OS X you need these additional frameworks (libraries) GLib, GObjects, Cairo and GTK+

To install these go to the [[Gtk Mac OX X project page ](http://www.gtk-osx.org/)]
  * Download the framework, chose open it in DiskMounter.
  * Double-click the Package icon and walk through the install steps

This makes the Frameworks available to Xcode and for 'make'.

Then you should be able to build using 'make' or Xcode or Code::Blocks.

The Xcode projects are not updated yet to include building for Gtk+

### Run Time dependencies ###
You'll need the "pcscd" daemon. This seems to come with Mac OS X.

To check it's running: from a terminal shell prompt type "ps -ef | grep pcsc" to see if it's there. Best to try that with a reader plugged-in, to be sure.

The ACS reader will need the **libccid** driver installed. This seems to come with and be loaded by pcscd.