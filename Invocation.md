The build will have left you a number of binaries in your local directory:-
Debug versions (debug symbols and some extra debugging output)
  * ./bin/Debug/tagEventor
  * ./bin/Debug/gtagEventor
Release versions
  * ./bin/Release/tagEventor
  * ./bin/Release/gtagEventor

Replace "Debug" with "Release" as you desire in the text below for the release version.

Type "./bin/Debug/tagEventor -h" to see the usage string.

Usage: tagEventor **options**
  * -n **reader number**   : default = AUTO, min = 0, max = 4.
  * -v **verbosity level** : default = 0 (silent), max = 3
  * -d start | stop : start or stop daemon
  * -p **msecs** : tag polling delay (milli seconds), default = 1,000
  * -h : print this message

The **tagEventor** command-line binary will start up in the foreground at the terminal and any output (depending on verbosity level set) will go to STDERR.

The **gtagEventor** binary (for Gtk2.0/Gnome) will start as an icon in the system tray. If started from the command line it won't disconnect from the terminal. You should use "gtagEventor&" for that.

## Install Directories ##
Linux
  * /etc/tagEventor for the command line version
  * /etc/gtagEveentor for the GtK UI version

Mac OS X
  * /Library/Application Support/tagEventor for the command line version
  * /Library/Application Support/tagEventor for the GtK UI version (you will need some GtK port to Mac!)

**NOTE: To start the daemon you will need permission for binary (see Install Directories above for where it will be) and hence you will probably need to start it as root, or with the "sudo" command!**

  * -n reader number: this is the number of the reader connected to the pcscd that manages the smart card resources. Default of AUTO is chosen. Note that this switch can be used many times to specify a number of readers, as well as AUTO. If AUTO is specified it will take priority over the other readers enabled. But the readers specified are remembered and the Settings GUI (Gtk/GNOME version) can be used to toggle them, and untoggle AUTO.

  * -v verbosity level: sets the verbosity of output. Messages are output if the verbosity level meets or exceed the level of the message. i.e. 0 is no output, 3 is the maximum.

  * -d start|stop: start or stop the daemon process for the given reader (default = 0) and remove the lock-file.

  * - p poll\_time: this is the time in milliseconds between each poll of the card reader. Default is 1,000, or 1 second.

  * -h: print the help or usage message