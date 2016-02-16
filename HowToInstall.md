To install, use the "install-sh" script.

  * 'cd' to the directory where you downloaded the source
  * type 'make' - just to be sure we have an updated set of binaries to install
  * type 'sudo ./install-sh'

You must be able to run as root to install on the local machine.
The install script checks this, warns you if you are not root and exists.

This script is now improved to:
  * if invoked as "sudo ./install-sh -" then it enters a menu driven interactive mode that will allow you to install the bits and pieces you want.
  * if invoked as "sudo ./install-sh" then it will install the "default" pieces and explain to you what they are, and show you what's installed where. "default" includes:
    * command\_line\_version - "tagEventor" command line version and scripts
    * gtk\_version - "gtagEventor" Gtk version and it's scripts
    * gtk\_autostart - A GNOME desktop shortcut that will start gtagEventor with every session
  * if invoked as "sudo ./install-sh $option" then it will install the option(s) you specify. Available values for $option are:
    * command\_line\_version - "tagEventor" command line version and scripts
    * daemon - a daemon that uses "tagEventor" command line version running in background, and that is started when the machine starts
    * gtk\_version - "gtagEventor" Gtk version and it's scripts
    * gtk\_autostart - A GNOME desktop shortcut that will start gtagEventor with every session
    * all - all of the above options