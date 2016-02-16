All errors (LOG\_ERROR) and warnings (LOG\_WARNING) are logged to syslog if in daemon mode, or output to STDERR in foreground mode.

Informational messages (LOG\_INFO) are logged to syslog (daemon mode) or output on STDOUT (foreground mode) if the currently set verbosity level is equal or higher than the message´s level. Default verbosity is 0, so no information messages are output.

See here for a list of messages output by level:

### Level 0 ###
None - So with default setting of verbosity 0, no output is generated.

### Level 1 ###
"Started daemon [name](command.md) with PID=[pid](pid.md) on reader [number](reader.md), see /var/log/syslog",
"Daemon Started on reader number [number](reader.md)
"Hangup signal received - will hangup and reconnect"
"SIGTERM received, exiting gracefully"
"Event: Tag [IN|OUT] - UID: [UID](tag.md)"
"SAM Serial: [number of SAM in reader](serial.md)
"SAM ID: [number of SAM in reader](ID.md)

### Level 2 ###
Current Tag state (number of tags present and what are their UIDs)
"Number of tags: [of tags detected ](number.md)"
"Tag ID:   [UID](tag.md)"

"Attempting to execute tag event script [UID as a string](tag.md)"
"Successfully connected to pcscd server"
"Found [of readers found](number.md) Readers, "
"Connected to reader: [number](reader.md)"
"Disconnecting from reader [number](reader.md)"
"Disconnecting from pcscd server"
"Tag Type: MIFARE\_ULTRA"

### Level 3 ###
Lower level commands sent to reader to read status, ATR, tags etc.
"APDU: [data sent as apdu in hex"
"Received: [response](response.md)
"Requesting Response Data ([of response bytes](number.md)"
"Received: [in hex received back](data.md)"

List of readers found on the system in pcscd, and some of their data:
"Reader [number](number.md): [name string as reported by pcscd](reader.md)"
"Firmware: [version string](firmare.md)"
"ATR: [returned by reader](ATR.md)"

## WARNINGS ##
"Will wait [delay](retyr.md) seconds and retry connection" when a reader is not found

## ERRORS ##
"Could not open lock file [file name](lock.md), exiting"
"Check you have the necessary permission for it"
"Could not read PID from lock file [file name](lock.md), exiting"
"Stopping daemon with PID = [pid](pid.md) on readerNumber [number](reader.md)"
"Could not open lock file [file name](lock.md), check permissions or run as root, exiting"
"Could not lock file [file name](lock.md)" ...
"Probably indicates a previous copy is still running or crashed"...
"Find PID using \"cat [file name](lock.md)\" or \"ps -ef | grep [name](command.md)\". Exiting.",
"Could not write PID to lock file [file name](lock.md)"
"Error forking daemon [name](daemon.md), exiting"
"Error creating new SID for daemon process [name](command.md) with PID=[pid](pid.md) on reader [number](reader.md), see in /var/log/syslog"
"Failed to find a command to execute" -> none of the script options were found

Any error reported in a call to the PCSC library, converted to a string by the same library.

"APDU failed: SW1 = [in HEX](SW1.md)"

"Wrong reader index: [number](reader.md)"
"Not enough memory for readers[.md](.md)"
"Reader ([firmware string](reader.md)) not supported"