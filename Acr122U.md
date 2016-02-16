## ACS 122U Tag Reader/Writer Details ##
NOTE: When checking the reader connected using  'lsusb' this reader reports:-
```
Bus 002 Device 005: ID 072f:90cc Advanced Card Systems, Ltd ACR38 SmartCard Reader
```

This causes most tools (pcsc\_tools and others) to erroneously detect it as a ACS ACR38U reader. Then a few things go wrong, among them is that commands to read or poll it's content or status will block and not return. Also, this reader will NOT report tag insert/remove events and so calls to pcsclite function "SCardGetStatusChange" will never return the correct status, and never return on an event, only a timeout.
It MUST be polled.

But this is ACS re-using the USB ID between devices, it is in fact a ACS ACR122U reader with a SAM card installed. This can be checked by using this program or another to send the appropriate commands (APDUs) to the reader to get it to report it's firmware version.

This program with appropriate verbosity level will like this:
Firmware: ACR122U102
where 'ACR122U102' is the string reported from the reader. I suspect the '102' at the end may be a firmware version number and may change over time.

It parses the firmware string and will support any reader that reports a firmware string that contains one of the supported readers in a list.
At the moment this list contains just one entry "ACR122U", but it can easily be expanded in the code when newer ones are tested.
NOTE: "contains", the extra 102 string will not cause an issue and will still be supported even if that ´minor´ firmware rev code changes.