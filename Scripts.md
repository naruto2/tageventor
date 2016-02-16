# Rules #

The long-term idea is that the UI of gtagEventor allows the user to define rules for events, and that it can invoke system commands and UI events, and that specific events can be configured for specific tags.

Short term a primitive system of script execution and rule matching, combined with a "catchall" generic script for any tag is used to get something working.

Also, to help people experimenting, it allows users to have scripts in local folders executed before the ones in the system folders, so they can easily experiment and do so without installing the application or being administrator on the computer.

Four built-in rules get evaluated, trying to find a matching script to execute for a tag event.

They are evaluated in this order until a matching script is found, then it is executed in a child process:

  1. any tag UID, look for a script with a name matching the tag UID, in the directory "./scripts" (i.e. below the folder where you were in the terminal when you executed tagEventor)
  1. any tag UID, look for a script named "generic", in the directory "./scripts" (i.e. below the folder where you were in the terminal when you executed tagEventor)
  1. any tag UID, look for a script with a name matching the tag UID, in the system directory where tagEventor is installed (see InstallationDirectories)
  1. any tag, look for a script named "generic", in the system directory where tagEventor is installed (see InstallationDirectories)

### "generic" script ###
A simple "generic" script is included that puts up a UI message of a tag being detected, with it's UID (on Linux uses x11message, on Mac OS X it uses growlnotify).

Feel free to copy or modify this in your own folders, or create a specific script for a specific action and then name the script matching the tag UID that you want to produce that action.