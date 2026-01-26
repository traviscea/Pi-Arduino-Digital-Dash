# Arduino MEGA 2560 project setup #

1. Install [visual studio code](https://code.visualstudio.com/Download)
2. Install the platformio extension
    * Open VSCode Package Manager
    * Search for the official platformio ide extension
    * Install PlatformIO IDE. <br><img width="356" height="245" alt="image" src="https://github.com/user-attachments/assets/87de76bd-3424-4039-af0c-b0152788d397" />

3. Double click `digital-dash-arduino.code-workspace` to open the project
    * platformio will automatically download and install the dependencies required for the project <br> <img width="662" height="351" alt="image" src="https://github.com/user-attachments/assets/fb826438-3ba0-4a74-9cd5-d65f1392a2cb" />

4. Open the file `platformio.ini` and configure the project
    * If you need CAN enabled, uncomment the line for CAN <br> <img width="513" height="42" alt="image" src="https://github.com/user-attachments/assets/913ea828-0b08-4fa5-a469-1266dc95f5e9" />

5. Plug in your mega 2560 and click the button in the right to upload the code <br><img width="259" height="167" alt="image" src="https://github.com/user-attachments/assets/8adfb0e3-8582-4537-9db0-8286d385837c" />
