# CANBUS INTEGRATION #


The CAN_Decoder.ino file is setup to detect and autorun multiple CANBUS protocols.  To add more protocols for your specific usecase, either follow the examples in the top of the code using your protocol or submit an issue/pull request with the json or dbc of your ecu/standalone's CAN protocol.

## Adding MCP2515 to Arduino to access CANBUS ##
This effectively makes the Arduino a CANBUS node

The specific INO file for HALTECHs or other standalones which is in the project will need to be uploaded to the Arduino via Arduino IDE

In the IDE, the appropriate libraries need to be installed so that the ino file(referenced above) has access to them.
Do this by:
- Going to the top and clikcing: ```Sketch → Include Library → Manage Libraries```
- Search for ```MCP_CAN```.  MCP_CAN_lib (Cory Fowler)
- This the libary the CANBUS ino's use ```#include <mcp_can.h>```



## Part needed: ##
https://a.co/d/fVGMQm0

## Connections ##

MCP215 to Arduino Mega

| MCP2515 Pin   | Arduino Mega Pin | Why                                 |
| ------------- | ---------------- | ----------------------------------- |
| **VCC**       | **5V**           | Power (most of these boards are 5V) |
| **GND**       | **GND**          | Ground                              |
| **CS**        | **10**           | SPI Chip Select (Mega hardware SS)  |
| **SO (MISO)** | **50**           | SPI MISO                            |
| **SI (MOSI)** | **51**           | SPI MOSI                            |
| **SCK**       | **52**           | SPI Clock                           |
| **INT**       | **2**            | Interrupt from MCP2515              |



MCP2515 ->  CAN

| MCP2515 Terminal | Haltech/etc  |
| ---------------- | ------------ |
| **CANH**         | CAN High     |
| **CANL**         | CAN Low      |




## Finding CANBUS output for your specific car/build if you dont know it##

SavvyCan(https://www.savvycan.com/) will allow you to hook up to the OBD2 port on your car and will give you all the CANBUS data.  

It will be up to you to determine, based off TimeStamps, IDs, and Data, what each value corresponds to as each CANBUS network is different




Copyright (c) 2025 Travis Way
