# CANBUS INTEGRATION #

## Finding CANBUS output for your specific car/build ##

SavvyCan(https://www.savvycan.com/) will allow you to hook up to the OBD2 port on your car and will give you all the CANBUS data.  

It will be up to you to determine, based off TimeStamps, IDs, and Data, what each value corresponds to as each CANBUS network is different



## Adding MCP2515 to Arduino to access CANBUS ##
This effectively makes the Arduino a CANBUS node

The specific INO file for HALTECHs or other standalones which is in the project will need to be uploaded to the Arduino via Arduino IDE

In the IDE, the appropriate library needs to be installed so that the ino file(referenced above) has access to it.
Do this by:
- Going to the top and clikcing: ```Sketch → Include Library → Manage Libraries```
- Search for ```MCP_CAN```
- This the libary the CANBUS ino's use ```#include <mcp_can.h>```


## Part needed: ##
https://a.co/d/fVGMQm0

## Connections ##

MCP215 to Arduino Mega

| MCP2515 Pin   | Arduino Mega Pin | Why                                 |
| ------------- | ---------------- | ----------------------------------- |
| **VCC**       | **5V**           | Power (most of these boards are 5V) |
| **GND**       | **GND**          | Ground                              |
| **CS**        | **53**           | SPI Chip Select (Mega hardware SS)  |
| **SO (MISO)** | **50**           | SPI MISO                            |
| **SI (MOSI)** | **51**           | SPI MOSI                            |
| **SCK**       | **52**           | SPI Clock                           |
| **INT**       | **2**            | Interrupt from MCP2515              |



MCP2515 -> Haltech CAN

| MCP2515 Terminal | Haltech  |
| ---------------- | -------- |
| **CANH**         | CAN High |
| **CANL**         | CAN Low  |

Copyright (c) 2025 Travis Way
