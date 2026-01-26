## Wiring Added and Not Covered in the Youtube Tutorial ##

### Battery Voltage ###
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
