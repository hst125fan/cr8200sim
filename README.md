# cr8200sim
Roughly simulates the CR8200 scan engine

# Building
Dependencies (for build):
* nu-book/ZXing-cpp (see submodules - clone with --recursive to obtain directly)
* shixiongfei/crc (see submodules  - clone with --recursive to obtain directly)
* Marzac/rs232 (see submodules - clone with --recursive to obtain directly)
* Python 3.x
* gcc, g++ Version 8 (On Windows e.g. MinGw)

Simply run the build.py script. It will produce the executable "cr8200.exe" (or "cr8200" without ".exe" on Linux).
Windows binaries can be found under "Releases".

# Installing
Put the executable in a separate, empty directory. This is not required, but useful (see next section)

# How to use
Start the cr8200(.exe) on command line, with COM-Port as first parameter. This allows you to connect using your own software which would
normally communicate with a real CR8200 via serial interface. Keep in mind, you can't do this on the same machine with the same port. Either,
you start cr8200(.exe) and your software on two different devices that are connnected using a serial cable (or USB emulating serial communication)
or use a COM Loop / Null-Modem (e.g. com0com), which will create two new serial ports on your computer that are connected with each other,
so you'll want to use one for the cr8200(.exe) and the other one for your program. Now you can do what you would do with a real CR8200, 
e.g. ask it to send you an image or to decode incoming barcodes.

To decode barcodes, put a "barcode.png" file with the said barcode next to the cr8200(.exe). As soon as a decode command is given,
the barcode will be decoded. The barcode.png file will be deleted after use. Already decoded barcode data can be enqueued for consumption
with a file "barcode.dat" which esentially works the same way.

# Limitations
* Raw commands are not supported, packet protocol is required.
* Most commands don't acutually do anything, they only return "OK". There is no configuration base (yet).
* GETting on most configuration parameters will fail.
* Large AZTEC codes cause some problems. In General, only barcodes that ZXing can read will work.
* Addressing is not implemented properly, it is expected that only one simulated reader exists per serial port.
* Data Parsing is not supported.
