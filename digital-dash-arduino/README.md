# Arduino MEGA 2560 project setup #

1. Install [visual studio code](https://code.visualstudio.com/Download)
2. Install the platformio extension
    * Open VSCode Package Manager
    * Search for the official platformio ide extension
    * Install PlatformIO IDE.
3. Double click `digital-dash-arduino.code-workspace` to open the project
    * platformio will automatically download and install the dependencies required for the project
4. Open the file `platformio.ini` and configure the project
    * If you need CAN enabled, uncomment the line for CAN
5. Plug in your mega 2560 and click the button in the right to upload the code