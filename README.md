## License

This project is source-available and free for personal and educational use.
Commercial use is NOT permitted.

# Pi-Arduino-Digital-Dash
A DIY digital dash I made using a Raspberry Pi 5, Arduino Mega 2560, and a touchscreen.  While my use case was a 02-07 Subaru WRX/STI variation, this will work for any car from 1920-2025+, with OEM ECUs, Standalones ECUs, with or without CANBUS...as long as you have sensors to pass data, this can work for you.  This [youtube video](https://youtu.be/Trl1QRnZZaE) contains an entire walkthrough. The code/files I made and used are all here.

#### For CANBUS ####
Please read the ```CANBUS_Communication_Setup.md```
<br>
I have added a Haltech INO to read directly from haltech's CANBUS.  This is just a WIP/draft and the arduino will need to be added as a CAN node.  That can be done with this 10$ part(https://a.co/d/0xAnIYF)

Future standalones/canbus configs will be added as I have time.
  
<br>

### *****Please do not use these files to sell this to others.  I made these so that those wanting to modify their dash/cluster dont have to spend an arm and a leg to do so.***** ###
<br>
<br>

## **Pi Setup**
I have added the dietPi OS img file with TS Dash already setup and autostarting on boot.  You will only need to write the image to your SD Card or boot disk, and then install it in Raspberry Pi 5.
<br>
<br>
The usernames for this img are ```dietpi``` and ```root```, with the default password being setup as ```password```.  Although, that will only be needed if you are modifying the OS in some way.
<br>
<br>
Upon booting for the first time, you will be taken directly to the TS Dash and the dashboard I created will display.  To modify the dashboard, add gauges, etc, you will need to register for the TS Dash Pro version at: https://www.efianalytics.com/register/viewProduct.jsp?productCode=TS_DashPro.
<br>
<br>
For free dashboard downloads go to https://tunerstudiodashboards.com/ (you will still need TS Dash pro to install them). 
<br>
<br>
## **Arduino Setup**
After the dashboard is setup how you like, the fun starts.  I utilized a arduino to communicate with the Pi and TS Dash to pass sensor data.  The ```.ino``` file in this repo contains all the code to add the Arduino Mega.  I will try to keep this file updated with suggestions from users or with additions I find useful.
<br>
<br>
---------------------------------------------------------------------------------------------------<br>
***I have added a gps speedo/odometer .ino file!  All the other sensor data is the same, just speed and mileage is coming from a m10 gps module.***
<br>
You can find the gps module here: https://a.co/d/0e3IsbNv 
<br>
and it is wired like so:
<br>
<br>
GPS --- Arduino
<br>
TX	--> RX1 (Pin 19)
<br>
VCC	--> 5v
<br>
GND	--> GND
<br>
<br>
I have also added different speed reading *modes*, with the ability to "fallback" to VSS if the GPS fails for any reason.  Default for the GPS ino file is *2*
<br>
<br>
Mode	Behavior
<br>
0	 =  VSS only
<br>
1	 =  GPS primary, VSS fallback
<br>
2	 =  GPS only

<br>
<br>
- Going to the top and clicking: ```Sketch → Include Library → Manage Libraries```
<br>
- Search for ```TinyGPSPlus```.
<br>
- This the libary the CANBUS ino's use ```#include <TinyGPS++.h>```
<br>
<br>
---------------------------------------------------------------------------------------------------<br>
<br>
<br>
To upload the ```.ino``` to the arduino you will need the ArduinoIDE(https://www.arduino.cc/en/software/) and to download it to a separate machine(not needed on the Pi)
<br>
<br>
Once you have the arduinoIDE installed and open on your machine, open the ```.ino``` in the IDE.  
<br>
<br>
Connect your Arduino Mega via usb and select the proper USB/location of the Arduino at the top right of the IDE. 
<br>
<br>
Press the right arrow button in the upper right(upload) to transfer the ```.ino``` to the arduino.
<br>
<br>
Wait for it to say "Done uploading" at the bottom and then you can unplug the arduino.
<br>
<br>
After that you just need to wire the sensors to the arduino.  My youtube video goes over that in depth, but basically you need to utilize sensors that already exist on your vehicle or purchase new ones solely for this purpose.
<br>
<br>
  
## **3d Print files**
3d print file for 02-07 Subaru wrx/sti cluster ----  https://makerworld.com/en/models/2182382-02-07-subaru-wrx-sti-gauge-cluster-for-7in-screen
<br>
<br>
<br>
  
## **Items Needed:**
<br>
Raspberry Pi 5 - 75$ https://amzn.to/4w82O1x

7in touchscreen(Or use different brand or size(10in, etc) - 41$ https://amzn.to/48lf5Fq

Arduino Mega 2560 - 23$ https://amzn.to/3QtXipC

12v to 5v 5A converter(2 pack) -15$ https://amzn.to/4963jPq

Jumper Wires/pins - 15$ https://amzn.to/4d0w0jf

Resistor Kit - 13$ https://amzn.to/4t53yBO

Transistor Kit - 9$ https://amzn.to/41QFyHp

Micro SD card - 8$(Use SSD or NVMe for quicker boot) https://amzn.to/4cK0eG2

Printer cable(cut red wire on for data transfer only) - 7$ https://amzn.to/4cvHmf1

Power Distribution Board - 7$ https://amzn.to/4cHWswK
<br>

### **Optional but makes it easier**

GPIO Terminal Block Breakout Board -29$ https://amzn.to/4cKgAhP

Pi 5v wall plug(to bench test) - 12$ https://amzn.to/4sTHk5u

22 awg wire - 8$ https://amzn.to/4sUK96m



Copyright (c) 2025 Travis Way
