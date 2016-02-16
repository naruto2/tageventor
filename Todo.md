Send other suggestions to [mailto:info@autelic.org](mailto:info@autelic.org)

# Build #
Modify make to build a binary package for Mac OS X that only contains tagEventor, as when Gtk is not present gtagEventor cannot be built, and also not many people will have Gtk on Mac and they just want tagEventor

## Tags Types ##
  * Each time we poll for tags, also poll for NFC Type 1 (Innovision Topaz, Jewel) and NFC Type 3 (Sony FeliCa) types. This will involve me getting some of those tags to test with first!

## Misc short-term stuff ##
  * Clean up the concepts of tags, tag events, and actions in all of program.

  * Get Icon theme and search path stuff to work PROPERLY

  * Make the Icon AMBER if it can connect to PCSCD but no readers found, red if cannot connect to PCSCD, and green if connected and a reader found.

  * Add the regexp matching of the tagID inside tagEventDispatch()

  * detect and decode NFC NDEF messages and the defined RTDs and act on them directly, by showing text, opening URL or whatever...

  * Write some text for the rules Editor Help Dialog and have it display it neatly, maybe with links to the wiki help pages or other sources.

## Mac Scripts ##
Look into having and executing AppleScript scripts or the like.

## Mac OS X GUI ##
Investigate Gtk+ on Mac OS X, or other ways to have the UI version also work. - this work is OnGoing -

## Man page ##
Write something useful in the man page, and then modify the install script to copy it to the appropriate location.

# Minor Improvements to Existing Functionality #
## Daemon code review ##
  * I'd really appreciate someone with experience on writing linux or unix daemons taking a look at the code in tagEventor.c for the daemon, for mutual exclusion using lock files as I'm not convinced it's bulletproof.
  * Also, I'd like it to check for the lockfile BEFORE it forks, so that an error message can be given to the user or script invoking it before it forks and fails. Then best to lock it again in the forked process to be sure.
## init scripts ##
  * The /etc/init.d script could be improved in a number of ways I think, such as catching a few corner cases that might cause failures, as well as making more standard compared to others. In doing that I think some of the logic in the application itself for daemonizing could be simplified and made more portable, as at the moment the 'C' code does what many others do in the init script.
## use reader LED colors ##
From reading the spec of the ACS reader it appears you can control the color of the LED (green, amber and red). I thought we could use this to give the user some feedback. Show that a tag was recognized and that action was launched or that an error was detected in the starting of the event/script. This might be especially useful as some events take a time to start and the user maybe tempted to place and remove the tag a few times until they see some action.

# Ease of acquiring #
## Install Packages for users ##
  * Build a Tiny Core Extension (TCE) for the Tiny Core Linux System, so tagEventor can be included in the tc-squared web-thin-client project which is based on Tiny Core
Create binary installation packages for supported systems
  * Debian .deb package for Ubuntu and other debian based Linux distributions
  * Packages for other Linux distribution packages
  * Add a Mac OS X Installer from the xcode project.

# New Functionality #
## Future Ideas ##
Generate compound events that are defined by TWO tags, such as "Andrew's tag" AND "e-mail tag", to be an event to read andrew's e-mail.

Think about if it's useful to define "states" as well as events, so that something is done or maintained WHILE a certain tag or tags are detected.

## Integration ##
Consider a better way of generating system events than scripts. Maybe one that's more tightly integrated with each OS it runs on.

## Read / Write tag contents ##
When a new tag is detected (for the session?) then read it's entire contents and make available to the user.
Possibly add the ability to edit it and then write it back to the same tag.

## Tree View of readers and tags ##
Make it possible to see what tag(s) is in which readers, and to see what contents are in each tag, all in a hierarchical tree view.

## NFC ##
Parse NFC NDEF messages and the RTD's and act upon the standardized content types as as URL directly in the application, without the need for a script.

## Security / authentication ##
  * It seems there is some mechanism with the embedded SAM card in the tag reader to do some authentication to the reader. Not sure how it applies, works or how to implement it. Investigation required.
  * I'd like to have the program also get the local PC MAC address being used, and the processor ID and maybe use them as additional security or to authenticate a client device.

## Login / Authentication ##
One of the common issues for inexpert or infrequent computer users (and hence a source of frustration for them and supports calls for companies) is loss of login and password info. So I suggest that for many users and uses a physical key in the form of an RFID tag (attached to a card, or keyring) or device (such as a mobile phone with NFC support) would be a good solution. Security would be derived mainly from possesion of it, and use on a known PC or tag reader.
  * Ability to use RFID key to Login / Keychain / Authenticate in new ways easing PC usage and reducing issues related to forgotten logins or passwords etc

# Supported Hardware #
  * Support more reader hardware (work is under way on this)

# New Platforms and Operating Systems #
  * Windows (XP, Vista, 7) port