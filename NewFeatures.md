# New Features #
This list is in reverse chronological order (latest at the top) and only covers significant changes.

### Binary Install ###
You can now download and install a small binary package only. See the binaryInstall page for more details.

### Multiple Readers ###
Multiple Readers are now supported at the same time. tagEventor and gtagEventor will (in automatic mode) figure out how many (supported) readers are connected to the system, then attempt to connect to them all, and then poll them all for tags. Readers maybe added and removed while the application is running and it will adapt accordingly. it builds a unique tag list from all the tags on all the readers and generates events based on that. It supports upto 6 readers (PCSC-Lite max I believe) and multiple tags on each reader. I could only test it on two readers to date.