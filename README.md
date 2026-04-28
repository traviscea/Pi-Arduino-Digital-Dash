## License

This project is source-available and free for personal and educational use.
Commercial use is NOT permitted.

# Pi-Arduino-Digital-Dash
A DIY digital dash I made using a Raspberry Pi 5, Arduino Mega 2560, and a touchscreen.  While my use case was a 02-07 Subaru WRX/STI variation, this will work for any car from 1920-2025+, with OEM ECUs, Standalones ECUs, with or without CANBUS...as long as you have sensors to pass data, this can work for you.  This [youtube video](https://youtu.be/Trl1QRnZZaE) contains an entire walkthrough. The code/files I made and used are all here.

#### For CANBUS ####
Please read the ```CANBUS_Communication_Setup.md```
<br>
I have added a Haltech INO to read directly from haltech's CANBUS.  This is just a WIP/draft and the arduino will need to be added as a CAN node.  That can be done with this [10$ part](https://www.amazon.com/dp/B01D0WSEWU?_encoding=UTF8&social_share=cm_sw_r_cp_ud_dp_AGTZYBY4H0VM5RWMRAPV_2&th=1&linkCode=ll2&tag=traviscea05-20&linkId=021101977929d6ecaa7c7b91e73d4613&language=en_US&ref_=as_li_ss_tl)

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

Disclaimer: Some of the links below are affiliate links, which means I may earn a commission at no extra cost to you.

As an Amazon Associate I earn from qualifying purchases.


Raspberry Pi 5 - 75$ [Amazon link](https://www.amazon.com/dp/B0CK3L9WD3?social_share=cm_sw_r_cp_ud_dp_1DD5B1RH6S7VT9CAR58Q&linkCode=ll2&tag=traviscea05-20&linkId=cc6cb0955534345e90336c41fa2e8cda&language=en_US&ref_=as_li_ss_tl)

7in touchscreen(Or use different brand or size(10in, etc) - 41$ [Amazon link](https://www.amazon.com/dp/B09XKC53NH?social_share=cm_sw_r_cp_ud_dp_2H0RCT2M49G77G8262WM&th=1&linkCode=ll2&tag=traviscea05-20&linkId=0e5cf05bad60380c72b185e2ed2eb054&language=en_US&ref_=as_li_ss_tl)

Arduino Mega 2560 - 23$ [Amazon link](https://www.amazon.com/dp/B01H4ZLZLQ?_encoding=UTF8&social_share=cm_sw_r_cp_ud_dp_H5PVXBFQVMWZ74VF5H8X&th=1&linkCode=ll2&tag=traviscea05-20&linkId=068af092f5ed31a900f140e21d30bf38&language=en_US&ref_=as_li_ss_tl)

12v to 5v 5A converter(2 pack) -15$ [Amazon link](https://www.amazon.com/dp/B0FD735LFG?_encoding=UTF8&social_share=cm_sw_r_cp_ud_dp_BKV0VXHK97E7EW68ZXR2&th=1&linkCode=ll2&tag=traviscea05-20&linkId=24cc0ba28d05f93e766c38906a40cadf&language=en_US&ref_=as_li_ss_tl)

Jumper Wires/pins - 15$ [Amazon link](https://www.amazon.com/dp/B01EV70C78?social_share=cm_sw_r_cp_ud_dp_9RW2PS22RH9QZZ6XJS54&linkCode=ll2&tag=traviscea05-20&linkId=f17e8f965fc5da432afe242fd58ea229&language=en_US&ref_=as_li_ss_tl)

Resistor Kit - 13$ [Amazon link](https://www.amazon.com/dp/B07P3MFG5D?social_share=cm_sw_r_cp_ud_dp_AR3AT7NKF7YTJ80EKTGY&linkCode=ll2&tag=traviscea05-20&linkId=f5d1872c0fa2ab0a6985f64593e99e42&language=en_US&ref_=as_li_ss_tl)

Transistor Kit - 9$ [Amazon link](https://www.amazon.com/dp/B07T61SY9Y?social_share=cm_sw_r_cp_ud_dp_KZQ6QB7G823G09D3W2A2&linkCode=ll2&tag=traviscea05-20&linkId=cd18b3b204634c84de0f02acdf4b187f&language=en_US&ref_=as_li_ss_tl)

Micro SD card - 8$(Use SSD or NVMe for quicker boot) [Amazon links](https://www.amazon.com/dp/B07R8GVGN9?social_share=cm_sw_r_cp_ud_dp_2X8Y5AD031V7FXTXV9S7&th=1&linkCode=ll2&tag=traviscea05-20&linkId=c00dc833592126202c7006f9aff2c5ac&language=en_US&ref_=as_li_ss_tl)

Printer cable(cut red wire on for data transfer only) - 7$ [Amazon link](https://www.amazon.com/dp/B00NH11KIK?social_share=cm_sw_r_cp_ud_dp_DQVA80VTP8BKRPJ0FV8R&th=1&linkCode=ll2&tag=traviscea05-20&linkId=47be211ae2fb288962e057958b334546&language=en_US&ref_=as_li_ss_tl)

Power Distribution Board - 7$ [Amazon link](https://www.amazon.com/dp/B07DW2C4ZB?social_share=cm_sw_r_cp_ud_dp_60D75AV4HJK6VE8ER2DJ&th=1&linkCode=ll2&tag=traviscea05-20&linkId=b2d9fb080aafcb5abc51864d3c2cb777&language=en_US&ref_=as_li_ss_tl)
<br>

### **Optional but makes it easier**

GPIO Terminal Block Breakout Board -29$ [Amazon link](https://www.amazon.com/dp/B08XVMBR6P?social_share=cm_sw_r_cp_ud_dp_N4XBDZ5H1Z55E4W18QH8_1&linkCode=ll2&tag=traviscea05-20&linkId=46b7f5d496ef40f445e4604e2ebfe857&language=en_US&ref_=as_li_ss_tl)

Pi 5v wall plug(to bench test) - 12$ [Amazon link](https://www.amazon.com/dp/B0CQLNP9DZ?social_share=cm_sw_r_cp_ud_dp_5660V558FVQBK9CKQKKT_1&linkCode=ll2&tag=traviscea05-20&linkId=b41b6724b453714e56e8fb84e31edabc&language=en_US&ref_=as_li_ss_tl)

22 awg wire - 8$ [Amazon link](https://www.amazon.com/dp/B08F7BMVJW?social_share=cm_sw_r_cp_ud_dp_PR2Z3ZMNATQXQVNBAFGD&th=1&linkCode=ll2&tag=traviscea05-20&linkId=3fae784fd5aca037f971f8dcff5ffbad&language=en_US&ref_=as_li_ss_tl)


<br>
<br>
Copyright (c) 2025 Travis Way
