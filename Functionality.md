NOTE: As a system may have multiple readers connected to it, a number of instances of the program (in foreground or daemon) may run at the same time, providing there is only one per reader as specified with the command line option ´-n´.

On start-up tagEventor it will detect reader and connect to it.
It will check is correct type of reader or abort with an error.
If correct type and SAM detected it will get SAM Serial number, and SAM ID.

Then it will enter an infinte loop to poll the reader (once per second at the moment) for tags.

NOTE: This reader hardware seems to support reading upto two tags simultaneously, and this program is written to handle that and report multiple tag events in one loop, and report multiple tags detected in the status. The code is written to handle more tags simultaneously in fact and the maximum is defined as a macro in tagReader.h called MAX\_NUM\_TAGS

It will output the initial tag status (how many tags and their unique IDs).

From then on it is silent until a tag event (tag(s) placed/removed) is detected.
On each such event it will optionally output:-
  * the event type: tag(s) placed/removed
  * the tags involved unique ID (UID)
  * the new tag status, with the UID of each tag currently on the reader
then it will look for a script to execute for that event
  * if running in foreground it will look for a script with the name of the tag´s unique ID number in the current directory where the user run it from. In daemon mode this is NOT done.
  * if no script found above, it will look for a script of the same name in the tagEventor command directory (/etc/tagEventor) and then execute it.
  * if no specific script for that tag is found then it will look for the ´generic´ script (of that exact name) in the /etc/tagEventor directory.

In all cases the script will be passed these parameters:
  * $1 = UID (unique ID of the tag, as later we may use wildcard naming)
  * $2 = Event Type ("IN" for new tag placed on reader, "OUT" for tag removed from reader)

There are a couple of sample scripts in the "scripts" subdirectory when downloaded or installed.
The "generic" script is run according to the rules defined, when a specific tag script is not found.
In it I simply show a message on the screen (using x11message on Linux and growlnotify on Mac OS X)