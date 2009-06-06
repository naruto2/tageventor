README.TXT
==========

tagEventor by Andrew Mackenzie for Autelic (http://www.autelic.org)
Copyright (C) 1999-2003, Andrew Mackenzie <andrew@autelic.org>
No Warantee implied or given.

Dependencies
============
libpcsclite to link executable.
pcscd installed and daemon running on local machine.
libccid driver installed, should be loaded by pcscd.

Hardware Supported
==================
This first version was written to work with touchatag (previously tikitag) tag reader and tags. This is an OEM of the ACS ACR122U reader and the tags are MiFare Ultalight tags.

NOTE: This reader usually reports 'lsusb' as:-
Bus 002 Device 005: ID 072f:90cc Advanced Card Systems, Ltd ACR38 SmartCard Reader

This causes most tools (pcsc_tools and others) to erroneously detect it as a ACS ACR38U reader. Then a few things go wrong, among them is that commands to read or poll it's content or status will block and not return. Also, this reader will NOT report tag insert/remove events and so calls to pcsclite function "SCardGetStatusChange" will never return the correct status, and never return on an event, only a timeout.
It MUST be polled.

But this is ACS re-using the USB ID between devices, it is in fact a ACS ACR122U reader with a SAM card installed. This can be checked by using this program or another to send the appropriate commands (APDUs) to the reader to get it to report it's firmware version.

This program with appropriate verbosity level will like this:
Firmware: ACR122U102
where 'ACR122U102' is the string reported from the reader. I suspect the '102' at the end may be a firmware version number and may change over time.

It parses the firmware string and will support any reader that reports a firmware string that contains one of the supported readers in a list. 
At the moment this list contains just one entry "ACR122U", but it can easily be expanded in the code when newer ones are tested.
NOTE: "contains", the extra 102 string will not cause an issue and will still be supported even if that ´minor´ firmware rev code changes.

Invocation / Usage
==================
Usage: tagEventor <options>
	-n <reader number>   : default = 0
	-v <verbosity level> : default = 0 (silent), max = 3 
	-d start | stop : start or stop daemon, default = foreground
	-r <secs> : retry delay to connect to reader (seconds), default = 10
	-p <usecs> : tag polling delay (micro seconds), default = 1000000
	-h : print this message

- -n reader number: this is the number of the reader connected to the pcscd that manages the smart card resources. Default of 0 is chosen.

- -v verbosity level: sets the verbosity of output. Messages are output if the verbosity level meets or exceed the level of the message. i.e. 0 is no output, 3 is the maximum.

- -d start|stop: start or stop the daemon process for the given reader (default = 0) and remove the lock-file.

-r retry: is no reader is found the program loops trying to connect to it, waiting for this specified time in seconds between each try. Default is 10 seconds.

- p poll: this is the time in microseconds between each poll of the card reader. Default is 1000,000 (1Million), or 1 second.

-h: print the help or usage message

Functionality
=============
NOTE: As a system may have multiple readers connected to it, a number of instances of the program (in foreground or daemon) may run at the same time, providing there is only one per reader as specified with the command line option ´-n´.

On start-up tagEventor it will detect reader and connect to it.
It will check is correct type of reader or abort with an error.
If correct type and SAM detected it will get SAM Serial number, and SAM ID.

Then it will enter an infinte loop to poll the reader (once per second at the moment) for tags.

NOTE: This reader hardware seems to support reading upto two tags simultaneously, and this program is written to handle that and report multiple tag events in one loop, and report multiple tags detected in the status. The code is written to handle more tags simultaneously in fact and the maximum is defined as a macro in tagReader.h called MAX_NUM_TAGS

It will output the initial tag status (how many tags and their unique IDs).

From then on it is silent until a tag event (tag(s) placed/removed) is detected.
On each such event it will optionally output:-
   - the event type: tag(s) placed/removed
   - the tags involved unique ID (UID)
   - the new tag status, with the UID of each tag currently on the reader
then it will look for a script to execute for that event
   - if running in foreground it will look for a script with the name of the tag´s unique ID number in the current directory where the user run it from. In daemon mode this is NOT done.
   - if no script found above, it will look for a script of the same name in the tagEventor command directory (/etc/tagEventor) and then execute it.
   - if no specific script for that tag is found then it will look for the ´generic´ script (of that exact name) in the /etc/tagEventor directory.

In all cases the script will be passed three parameters
$1 = SAM (unique ID of the SAM chip in the smart card reader if exists, "NoSAM" otherwise
$2 = UID (unique ID of the tag, as later we may use wildcard naming)
$3 = Event Type (IN for new tag placed on reader, OUT for tag removed from reader)

Output
======
All errors (LOG_ERROR) and warnings (LOG_WARNING) are logged to syslog if in daemon mode, or output to STDERR in foreground mode. 

Informational messages (LOG_INFO) are logged to syslog (daemon mode) or output on STDOUT (foreground mode) if the currently set verbosity level is equal or higher than the message´s level. Default verbosity is 0, so no information messages are output.

See here for a list of messages output by level:

Level 0
-------
None

Level 1
-------
"Started daemon [command name] with PID=[pid] on reader [reader number], see /var/log/syslog",
"Daemon Started on reader number [reader number]
"Hangup signal received - will hangup and reconnect"
"SIGTERM received, exiting gracefully"
"Event: Tag [IN|OUT] - UID: [tag UID]"
"SAM Serial: [serial number of SAM in reader]
"SAM ID: [ID number of SAM in reader]

Level 2
-------
Current Tag state (number of tags present and what are their UIDs)
"Number of tags: [number of tags detected ]"
"Tag ID:   [tag UID]"

"Attempting to execute tag event script [tag UID as a string]"
"Successfully connected to pcscd server"
"Found [number of readers found] Readers, "
"Connected to reader: [reader number]"
"Disconnecting from reader [reader number]"
"Disconnecting from pcscd server"
"Tag Type: MIFARE_ULTRA"

Level 3
-------
Lower level commands sent to reader to read status, ATR, tags etc.
"APDU: [data sent as apdu in hex"
"Received: [response]
"Requesting Response Data ([number of response bytes]"
"Received: [data in hex received back]"

List of readers found on the system in pcscd, and some of their data:
"Reader [number]: [reader name string as reported by pcscd]"
"Firmware: [firmare version string]"
"ATR: [ATR returned by reader]"

WARNINGS
--------
"Will wait [retyr delay] seconds and retry connection" when a reader is not found

ERRORS
------
"Could not open lock file [lock file name], exiting"
"Check you have the necessary permission for it"
"Could not read PID from lock file [lock file name], exiting"
"Stopping daemon with PID = [pid] on readerNumber [reader number]"
"Could not open lock file [lock file name], check permissions or run as root, exiting"
"Could not lock file [lock file name]" ...
"Probably indicates a previous copy is still running or crashed"...
"Find PID using \"cat [lock file name]\" or \"ps -ef | grep [command name]\". Exiting.",      
 "Could not write PID to lock file [lock file name]"
"Error forking daemon [daemon name], exiting"
"Error creating new SID for daemon process [command name] with PID=[pid] on reader [reader number], see in /var/log/syslog"
"Failed to find a command to execute" -> none of the script options were found

Any error reported in a call to the PCSC library, converted to a string by the same library.

"APDU failed: SW1 = [SW1 in HEX]"

"Wrong reader index: [reader number]"
"Not enough memory for readers[]"
"Reader ([reader firmware string]) not supported"
