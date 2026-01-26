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
After the dashboard is setup how you like, the fun starts.  I utilized a arduino to communicate with the Pi and TS Dash to pass sensor data.  The ```digital-dash-arduino``` platformio project in this repo contains all the code to add the Arduino Mega.  I will try to keep this file updated with suggestions from users or with additions I find useful 
<br>
<br>
Please follow the instrustions in the [README.md file in digital-dash-arduino](digital-dash-arduino/README.md)
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
Raspberry Pi 5 - 75$ https://a.co/d/6ZqmxDW

7in touchscreen(Or use different brand or size(10in, etc) - 41$ https://a.co/d/9zJ6kil

Arduino Mega 2560 - 23$ https://a.co/d/3NjqWy6

12v to 5v 5A converter(2 pack) -15$ https://a.co/d/bSsF607

Jumper Wires/pins - 15$ https://a.co/d/avUqsKu

Resistor Kit - 13$ https://a.co/d/8vXxGQ8

Transistor Kit - 9$ https://a.co/d/8RgLv33

Micro SD card - 8$(Use SSD or NVMe for quicker boot) https://a.co/d/foGQRCk

Printer cable(cut red wire on for data transfer only) - 7$ https://a.co/d/ezsc7eX

Power Distribution Board - 7$ https://a.co/d/5vZJxqO
<br>
### **Optional but makes it easier**

GPIO Terminal Block Breakout Board -29$ https://a.co/d/5wfxBdf

Pi 5v wall plug(to bench test) - 12$ https://a.co/d/3odHP2B

22 awg wire - 8$ https://a.co/d/eDb6Mbe



Copyright (c) 2025 Travis Way
