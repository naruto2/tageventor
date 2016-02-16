Here is the list of currently supported hardware (readers and tags). See below for detailed comments on particular hardware.

The first version of tagEventor was written to work with touchatag (previously tikitag) tag reader and tags.
This is an OEM of the ACS ACR122U reader and the tags are MiFare Ultalight tags.

## Readers Devices ##
  * [ACS 122U Tag reader](Acr122U.md)
    * NFC Type 1 (Jewel / Topaz) - should work, haven't been able to test yet
    * NFC Type 2
      * MiFare Ultralight - OK, Tested
      * MiFare 1K         - OK, Tested
      * MiFare 4K         - OK, Tested
    * NFC Type 3 - Sony FeliCa - not been able to test yet.
    * NFC Type 4 - JCOP - Causes error at the moment
  * SCM SCL 3710 USB-dongle NFC Reader/Writer
    * working on it