// =======================================================
// Travis Digital Dash
// Haltech CAN → Arduino → TS Dash Serial Bridge
//
// Hardware:
//   Arduino Mega 2560
//   MCP2515 CAN module (16MHz) @ 500kbps
//
// Serial protocol emulates TS / Speeduino dash:
//   Q -> signature
//   S -> version
//   r -> 87-byte output channel block
//
// Signature: speeduino-travis
// =======================================================

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

// ===================== IDENTITY =====================
static const char SIGNATURE[] = "speeduino-travis";
static const char VERSION[]   = "Travis Digital Dash v1.0";

// ===================== SERIAL =====================
static constexpr uint32_t BAUD_RATE = 115200;

// ===================== CAN (MCP2515) =====================
#define CAN_CS_PIN   53
#define CAN_INT_PIN  2
#define CAN_CLOCK    MCP_16MHZ
#define CAN_SPEED    CAN_500KBPS

MCP_CAN CAN(CAN_CS_PIN);

// ===================== HALTECH CAN IDS =====================
// These are COMMON Haltech defaults.
// Adjust if your ECU broadcast layout differs.
static constexpr uint16_t ID_RPM_MAP    = 0x360;
static constexpr uint16_t ID_TEMPS      = 0x361;
static constexpr uint16_t ID_PRESSURES  = 0x362;
static constexpr uint16_t ID_SPEED_FUEL = 0x363;
static constexpr uint16_t ID_INDICATORS = 0x364;

// ===================== OUTPUT CHANNEL BLOCK =====================
static constexpr uint8_t OCH_BLOCK_SIZE = 87;
static uint8_t och[OCH_BLOCK_SIZE];

// ===================== LIVE DATA =====================
static volatile uint16_t rpm = 0;
static volatile uint16_t map_kpa = 100;
static volatile int16_t  clt_c = 20;
static volatile int16_t  oil_c = 20;
static volatile uint8_t  oil_psi = 0;
static volatile uint8_t  fuel_psi = 0;
static volatile uint16_t speed_kph = 0;
static volatile uint8_t  fuel_pct = 0;

static volatile uint8_t leftTurn = 0;
static volatile uint8_t rightTurn = 0;
static volatile uint8_t cel = 0;
static volatile uint8_t highBeam = 0;
static volatile uint8_t handbrake = 0;

// ===================== CAN DEBUG =====================
static uint16_t lastCanId = 0;
static uint8_t  lastCanLen = 0;
static uint8_t  lastCanBuf[8] = {0};
static uint32_t lastCanRxMs = 0;

// ===================== SERIAL DEBUG =====================
static bool debugEnabled = false;
static uint32_t lastDebugPrint = 0;

// ===================== HELPERS =====================
static inline uint16_t u16le(const uint8_t *b) {
  return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}
static inline int16_t s16le(const uint8_t *b) {
  return (int16_t)((uint16_t)b[0] | ((uint16_t)b[1] << 8));
}
static inline void writeU16LE(uint8_t *dst, uint16_t v) {
  dst[0] = v & 0xFF;
  dst[1] = v >> 8;
}
static inline uint8_t tempToIni(int16_t c) {
  int v = c + 40;
  if (v < 0) v = 0;
  if (v > 255) v = 255;
  return (uint8_t)v;
}

// ===================== CAN HANDLER =====================
static void handleCan(uint16_t id, const uint8_t *buf, uint8_t len) {
  lastCanId = id;
  lastCanLen = len;
  memcpy(lastCanBuf, buf, len);
  lastCanRxMs = millis();

  switch (id) {
    case ID_RPM_MAP:
      rpm = u16le(&buf[0]);
      map_kpa = u16le(&buf[2]);
      break;

    case ID_TEMPS:
      clt_c = s16le(&buf[0]);
      oil_c = s16le(&buf[2]);
      break;

    case ID_PRESSURES:
      oil_psi  = buf[0];
      fuel_psi = buf[1];
      break;

    case ID_SPEED_FUEL:
      speed_kph = u16le(&buf[0]);
      fuel_pct  = buf[2] > 100 ? 100 : buf[2];
      break;

    case ID_INDICATORS: {
      uint8_t b = buf[0];
      leftTurn  = (b & 0x01) ? 1 : 0;
      rightTurn = (b & 0x02) ? 1 : 0;
      cel       = (b & 0x04) ? 1 : 0;
      highBeam  = (b & 0x08) ? 1 : 0;
      handbrake = (b & 0x10) ? 1 : 0;
    } break;
  }
}

// ===================== BUILD OCH BLOCK =====================
static void buildOch() {
  memset(och, 0, sizeof(och));

  writeU16LE(&och[0], rpm);          // rpm
  och[3]  = tempToIni(oil_c);        // oil temp raw
  writeU16LE(&och[4], map_kpa);      // map
  och[7]  = tempToIni(clt_c);        // coolant temp raw
  och[10] = oil_psi;                 // oil pressure
  och[11] = fuel_psi;                // fuel pressure
  och[15] = fuel_pct;                // fuel level
  writeU16LE(&och[19], speed_kph);   // vss

  och[40] = leftTurn;
  och[41] = rightTurn;
  och[42] = cel;
  och[43] = highBeam;
  och[44] = handbrake;
}

// ===================== SERIAL COMMAND HANDLER =====================
static void handleSerial(uint8_t c) {

  // ---- TS REQUIRED ----
  if (c == 'Q') { Serial.print(SIGNATURE); return; }
  if (c == 'S') { Serial.print(VERSION);   return; }
  if (c == 'r') {
    buildOch();
    Serial.write(och, OCH_BLOCK_SIZE);
    return;
  }

  // ---- DEBUG COMMANDS ----
  if (c == 'd') {
    debugEnabled = !debugEnabled;
    Serial.println(debugEnabled ? "DEBUG ON" : "DEBUG OFF");
    return;
  }

  if (c == 'D') {
    Serial.println(F("---- HALTECH CAN DEBUG ----"));
    Serial.print(F("RPM: ")); Serial.println(rpm);
    Serial.print(F("MAP kPa: ")); Serial.println(map_kpa);
    Serial.print(F("CLT C: ")); Serial.println(clt_c);
    Serial.print(F("Oil C: ")); Serial.println(oil_c);
    Serial.print(F("Oil PSI: ")); Serial.println(oil_psi);
    Serial.print(F("Fuel PSI: ")); Serial.println(fuel_psi);
    Serial.print(F("Speed kph: ")); Serial.println(speed_kph);
    Serial.print(F("Fuel %: ")); Serial.println(fuel_pct);
    Serial.print(F("LT RT CEL HB HBK: "));
    Serial.print(leftTurn); Serial.print(" ");
    Serial.print(rightTurn); Serial.print(" ");
    Serial.print(cel); Serial.print(" ");
    Serial.print(highBeam); Serial.print(" ");
    Serial.println(handbrake);
    Serial.println(F("---------------------------"));
    return;
  }

  if (c == 'C') {
    Serial.print(F("CAN ID 0x"));
    Serial.print(lastCanId, HEX);
    Serial.print(F(" LEN "));
    Serial.println(lastCanLen);
    Serial.print(F("DATA "));
    for (uint8_t i = 0; i < lastCanLen; i++) {
      if (lastCanBuf[i] < 16) Serial.print('0');
      Serial.print(lastCanBuf[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
    return;
  }
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(CAN_INT_PIN, INPUT);

  if (CAN.begin(MCP_ANY, CAN_SPEED, CAN_CLOCK) != CAN_OK) {
    while (1) {;}
  }
  CAN.setMode(MCP_NORMAL);
  lastCanRxMs = millis();
}

// ===================== LOOP =====================
void loop() {

  // ---- CAN RECEIVE ----
  if (!digitalRead(CAN_INT_PIN)) {
    long unsigned int rxId;
    unsigned char len;
    unsigned char buf[8];

    if (CAN.readMsgBuf(&rxId, &len, buf) == CAN_OK) {
      handleCan(rxId & 0x7FF, buf, len);
    }
  }

  // ---- CAN TIMEOUT SAFETY ----
  if (millis() - lastCanRxMs > 1000) {
    rpm = 0;
    speed_kph = 0;
  }

  // ---- SERIAL ----
  while (Serial.available()) {
    handleSerial((uint8_t)Serial.read());
  }

  // ---- OPTIONAL LIVE DEBUG (USE ONLY WITHOUT TS DASH) ----
  if (debugEnabled && millis() - lastDebugPrint > 500) {
    lastDebugPrint = millis();
    Serial.print(F("RPM "));
    Serial.print(rpm);
    Serial.print(F(" MAP "));
    Serial.print(map_kpa);
    Serial.print(F(" CLT "));
    Serial.print(clt_c);
    Serial.print(F(" OilPSI "));
    Serial.println(oil_psi);
  }
}
