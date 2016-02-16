TagEventor is a project from the non-profit Autelic Association (http://www.autelic.org). Read [Background](Background.md) for more info on Autelic.

The project goal is to enable radically simple computer usage by creating physical-object-based user interfaces.

It does this using commercially available (and relatively cheap now), standardized RFID technology in the form of USB contacted card/tag readers, and small, cheap tags.

The project was started using hardware (reader & tags) from the "touchatag" company, which has clients for Windows and Mac, and run their own web service to enable many interesting web-based applications (see http://www.touchatag.com). This means the ACR122 USB reader is supported, but we are working on adding support for other readers.

However, no simple, lightweight Linux client was available from "touchatag" so this project created one.

Also, the "touchatag" web service focus meant that some functionalities I'd like to enable (like login to PC, or keychain use, or local actions when no Internet available, or using a tag to connect to a Wi-Fi network) were not possible.

The application can be run as a foreground application or as a background "daemon", or as a System tray Icon (Gnome/Linux) that monitors the presence of upto two RFID tags on a connected reader and generates "system events" when tags are placed on it, or removed from it (at the moment using a simple script mechanism).

It runs on Linux and Mac OS X.
Volunteers are working to enable running on Windows.

If you're interested in helping out, see our [volunteer's page](VolunteerPage.md).

In particular some help on Icon design would be very welcome! Mine stink! It's much tougher to design a decent icon that I'd ever have imagined.

Read more:
  * [Basic build, run and install info](ReadMe.md)
  * [Install from binaries without building from source](InstallationDirectories.md)
  * [Installation directories](InstallationDirectories.md)
  * [Supported RFID/NFC reader devices and tag types](SupportedReadersAndTags.md)
  * [Systems it currently runs on](SupportedSystems.md) and [Software Dependencies](SoftwareDependencies.md)
  * [Functionality](Functionality.md)
  * [Invocation of tagEventor from the command line](Invocation.md)
  * [What scripts get run for tag events and writing your own](Scripts.md)
  * [A Description of output / log messages by level](OutputDescription.md)
  * [A description of the latest feature additions ](NewFeatures.md)
  * [List of things to do, and ideas for how to evolve it in the future](Todo.md)