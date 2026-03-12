## Wiring Added and Not Covered in the Youtube Tutorial ##

### Battery Voltage Reading for voltage guage ###
You can not send anything over 5v to the Analog pins on the arduino.  <br>
To combat this, we need to wire the voltage to A6 on arduino as follows:<br>

Car Battery + (9-17v)<br>
       |<br>
       |<br>
    22kΩ<br>
       |<br>
       |<br>
     +----------> A6  (Arduino Mega)<br>
       |<br>
       |<br>
    10kΩ<br>
       |<br>
       |<br>
    Ground<br>



### Wiring for GPS module(also covered more in the readme file) ###
You can find the gps module here: https://a.co/d/0e3IsbNv 
<br>
and it is wired like so:
<br>
<br>
GPS --- Arduino
<br>
X	--> RX1 (Pin 19)
<br>
VCC	--> 5v
<br>
GND	--> GND
<br>
<br>
