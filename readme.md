# Roomba controller using ESP32's WiFi access point
## About
This is Roomba remote control demo project based on the [Roomba-Butler Belvedere project by wolffan.](https://www.instructables.com/id/Belvedere-A-Butler-Robot/) Using ESP32's access point feature, you can connect and control the Roomba from your smartphone over wifi.


## System
The system consists of a M5Stack/ESP32, a Roomba and self made M5 unit for Roomba. M5Stack communicates with Roomba over Roomba's programming port.


### Interfacing Roomba
The protocol, Open Interface is documented [here](https://www.irobotweb.com/~/media/MainSite/PDFs/About/STEM/Create/iRobot_Roomba_600_Open_Interface_Spec.pdf). The document was written for Roomba 600 / Create 2, but I could communicate Roomba 700 series with this protocol. I've implemented most of features to move the Roomba around, but there are some functions I wasn't interested to interface. Please check the OI document for further details.


### Setting up
This project was built on PlatformIO. You could use Arduino IDE instead, but you may need to modify the folder structure.
You also need to copy indel.html under SD folder onto a micro SD card and insert in to the M5Stack.


### Roomba module for M5
I also made a Roomba interface for M5Stack. To make the interface, you need to find 3.3-5V level shifter for UART level conversion [like this], and preferably a stepdown regulator[like this] to supply 5V. You could power up the M5 using battery module, but since the software access point feature consumes considerable power, I decided to draw power from Roomba's battery.
Schematic Diagram
Picture

### Note regarding code
I couldn't communicate with M5Stack on 9600 bps after calling M5.begin(). So it seems better idea to stick on 115200 bps instead.

## Reference
We've referenced from the [Roomba-Butler Belvedere project by wolffan.](https://www.instructables.com/id/Belvedere-A-Butler-Robot/) Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'll be eternally C-noob, I couldn't make it without your support.
