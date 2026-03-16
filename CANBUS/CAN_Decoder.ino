/*
   Travis Digital Dash
   Multi-ECU CAN Decoder
   Mega 2560 Optimized

   Auto detects:
   Haltech
   MaxxECU
   EMU Black

   Output: Speeduino OCH protocol
*/

#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS_PIN 10
#define BAUD_RATE 115200
#define OCH_BLOCK_SIZE 87

#define MAX_SIGNALS 128
#define MAX_CAN_IDS 32

MCP_CAN CAN(CAN_CS_PIN);

/* ============================
   CAN SIGNAL STRUCT
============================ */

struct CANSignal {
  uint32_t id;
  uint8_t start;
  uint8_t length;
  float scale;
  float offset;
  bool littleEndian;
  const char *name;
};

/* ============================
   CAN FRAME MAP
============================ */

struct FrameMap {
  uint32_t id;
  uint16_t startIndex;
  uint8_t count;
};

FrameMap frameMap[MAX_CAN_IDS];
uint8_t frameCount=0;

/* ============================
   HALTECH
============================ */

const CANSignal haltechProtocol[] PROGMEM = {

{0x360,0,2,1,0,false,"RPM"},
{0x360,2,2,0.1,0,false,"MAP"},
{0x361,2,2,0.1,-101.3,false,"OIL_PRESSURE"},
{0x361,0,2,0.1,-101.3,false,"FUEL_PRESSURE"},
{0x370,0,2,0.1,0,false,"VEHICLE_SPEED"},
{0x3E0,0,2,0.1,0,false,"COOLANT_TEMP"},
{0x3E0,6,2,0.1,0,false,"OIL_TEMP"},
{0x372,0,2,0.1,0,false,"BATTERY_VOLTAGE"}

};

/* ============================
   MAXXECU
============================ */

const CANSignal maxxecuProtocol[] PROGMEM = {

{0x520,0,2,1,0,true,"RPM"},
{0x520,4,2,0.1,0,true,"MAP"},
{0x522,6,2,0.1,0,true,"VEHICLE_SPEED"},
{0x530,6,2,0.1,0,true,"COOLANT_TEMP"},
{0x536,6,2,0.1,0,true,"OIL_TEMP"},
{0x536,4,2,0.001,0,true,"OIL_PRESSURE"},
{0x530,0,2,0.01,0,true,"BATTERY_VOLTAGE"}

};

/* ============================
   EMU BLACK
============================ */

const CANSignal emuProtocol[] PROGMEM = {

{0x600,0,2,1,0,true,"RPM"},
{0x600,4,2,1,0,true,"MAP"},
{0x602,0,2,1,0,true,"VEHICLE_SPEED"},
{0x602,6,2,1,0,true,"COOLANT_TEMP"},
{0x602,3,1,1,0,true,"OIL_TEMP"},
{0x602,4,1,0.0625,0,true,"OIL_PRESSURE"},
{0x604,2,2,0.027,0,true,"BATTERY_VOLTAGE"}

};

/* ============================
   ACTIVE PROTOCOL
============================ */

const CANSignal *protocol;
uint16_t signalCount;

/* ============================
   HASH LOOKUP
============================ */

struct SignalLookup {
  uint32_t hash;
  uint16_t index;
};

SignalLookup lookupTable[MAX_SIGNALS];
float dashValues[MAX_SIGNALS];

uint8_t och[OCH_BLOCK_SIZE];

unsigned long rxId;
unsigned char len;
unsigned char rxBuf[8];

/* ============================
   HASH
============================ */

uint32_t hashSignal(const char *str){
  uint32_t hash=5381;
  while(*str)
    hash=((hash<<5)+hash)+*str++;
  return hash;
}

/* ============================
   BUILD SIGNAL LOOKUP
============================ */

void buildLookup(){

  for(uint16_t i=0;i<signalCount;i++){
    CANSignal sig;
    memcpy_P(&sig,&protocol[i],sizeof(sig));

    lookupTable[i].hash=hashSignal(sig.name);
    lookupTable[i].index=i;
  }
}

/* ============================
   BUILD FRAME MAP
============================ */

void buildFrameMap(){

  frameCount=0;

  for(uint16_t i=0;i<signalCount;i++){

    CANSignal sig;
    memcpy_P(&sig,&protocol[i],sizeof(sig));

    bool found=false;

    for(uint8_t f=0;f<frameCount;f++){
      if(frameMap[f].id==sig.id){
        frameMap[f].count++;
        found=true;
        break;
      }
    }

    if(!found){
      frameMap[frameCount].id=sig.id;
      frameMap[frameCount].startIndex=i;
      frameMap[frameCount].count=1;
      frameCount++;
    }
  }
}

/* ============================
   SIGNAL LOOKUP
============================ */

float getSignal(const char* name){

  uint32_t h=hashSignal(name);

  for(uint16_t i=0;i<signalCount;i++){
    if(lookupTable[i].hash==h)
      return dashValues[lookupTable[i].index];
  }

  return 0;
}

/* ============================
   SELECT PROTOCOL
============================ */

void selectProtocol(uint8_t p){

  switch(p){

    case 0:
      protocol=haltechProtocol;
      signalCount=sizeof(haltechProtocol)/sizeof(CANSignal);
      Serial.println("Protocol: Haltech");
      break;

    case 1:
      protocol=maxxecuProtocol;
      signalCount=sizeof(maxxecuProtocol)/sizeof(CANSignal);
      Serial.println("Protocol: MaxxECU");
      break;

    case 2:
      protocol=emuProtocol;
      signalCount=sizeof(emuProtocol)/sizeof(CANSignal);
      Serial.println("Protocol: EMU Black");
      break;
  }

  buildLookup();
  buildFrameMap();
}

/* ============================
   AUTO DETECT ECU
============================ */

void autoDetectProtocol(){

  bool seen360=false;
  bool seen520=false;
  bool seen600=false;

  Serial.println("Detecting ECU...");

  uint32_t start=millis();

  while(millis()-start<2000){

    if(CAN_MSGAVAIL==CAN.checkReceive()){

      CAN.readMsgBuf(&rxId,&len,rxBuf);

      if(rxId==0x360) seen360=true;
      if(rxId==0x520) seen520=true;
      if(rxId==0x600) seen600=true;
    }
  }

  if(seen360) selectProtocol(0);
  else if(seen520) selectProtocol(1);
  else if(seen600) selectProtocol(2);
  else{
    Serial.println("Unknown ECU → Haltech");
    selectProtocol(0);
  }
}

/* ============================
   CAN DECODER (FAST)
============================ */

void decodeCAN(uint32_t id,uint8_t *buf){

  for(uint8_t f=0;f<frameCount;f++){

    if(frameMap[f].id!=id) continue;

    uint16_t start=frameMap[f].startIndex;
    uint8_t count=frameMap[f].count;

    for(uint8_t i=0;i<count;i++){

      CANSignal sig;
      memcpy_P(&sig,&protocol[start+i],sizeof(sig));

      uint32_t raw=0;

      if(sig.littleEndian){
        for(uint8_t b=0;b<sig.length;b++)
          raw|=buf[sig.start+b]<<(8*b);
      }
      else{
        for(uint8_t b=0;b<sig.length;b++){
          raw<<=8;
          raw|=buf[sig.start+b];
        }
      }

      dashValues[start+i]=raw*sig.scale+sig.offset;
    }

    break;
  }
}

/* ============================
   BUILD OCH
============================ */

void buildOch(){

  memset(och,0,OCH_BLOCK_SIZE);

  float rpm=getSignal("RPM");
  float clt=getSignal("COOLANT_TEMP");
  float oilt=getSignal("OIL_TEMP");
  float oilp=getSignal("OIL_PRESSURE");
  float map=getSignal("MAP");
  float vss=getSignal("VEHICLE_SPEED");
  float batt=getSignal("BATTERY_VOLTAGE");

  uint16_t r=(uint16_t)rpm;
  och[0]=r&0xFF;
  och[1]=r>>8;

  och[7]=(uint8_t)clt;
  och[3]=(uint8_t)oilt;

  int16_t boost=(int16_t)map;
  och[4]=boost&0xFF;
  och[5]=boost>>8;

  och[10]=(uint8_t)oilp;

  uint16_t sp=(uint16_t)vss;
  och[19]=sp&0xFF;
  och[20]=sp>>8;

  uint16_t vb=(uint16_t)(batt*10);
  och[22]=vb&0xFF;
  och[23]=vb>>8;
}

/* ============================
   SERIAL HELPER
============================ */

uint16_t readU16LE(){
  while(Serial.available()<2);
  uint8_t lo=Serial.read();
  uint8_t hi=Serial.read();
  return (hi<<8)|lo;
}

/* ============================
   SETUP
============================ */

void setup(){

  Serial.begin(BAUD_RATE);

  if(CAN.begin(MCP_ANY,CAN_1000KBPS,MCP_8MHZ)!=CAN_OK){
    Serial.println("CAN FAIL");
    while(1);
  }

  CAN.setMode(MCP_NORMAL);

  autoDetectProtocol();

  Serial.println("CAN Decoder Ready");
}

/* ============================
   LOOP
============================ */

void loop(){

  if(CAN_MSGAVAIL==CAN.checkReceive()){
    CAN.readMsgBuf(&rxId,&len,rxBuf);
    decodeCAN(rxId,rxBuf);
  }

  while (Serial.available()) {
    char c = Serial.read();

    if(c=='Q') {
      Serial.write("speeduino-travis",32);
    }

    else if(c=='S') {
      Serial.write("CAN Dash v1.0",32);
    }

    else if (c == 'F') {
      uint8_t f[3] = {0,0,0};
      Serial.write(f,3);
    }

    else if (c == 'r') {
      readU16LE();
      readU16LE();
      buildOch();
      Serial.write(och, OCH_BLOCK_SIZE);
    }

    else if (c == 'p') {
      readU16LE();
      readU16LE();
      uint16_t len = readU16LE();

      static uint8_t z[288];
      memset(z, 0, min(len,(uint16_t)288));
      Serial.write(z, min(len,(uint16_t)288));
    }

    else if (c == 'b') {
      readU16LE();
    }

    else if (c == 'd') {
      readU16LE();
      uint32_t crc = 0;
      Serial.write((uint8_t*)&crc,4);
    }
  }
}
