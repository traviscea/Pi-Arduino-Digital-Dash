#ifdef CAN_ENABLED
// =======================================================
// Travis Digital Dash – Haltech CAN → TS Dash Serial Bridge
// Arduino Mega 2560 + MCP2515 (16MHz) @ 500kbps 
// Copyright (c) 2025 Travis Way
//
// Serial side emulates the INI requirements:
//   queryCommand   = "Q"  -> returns "speeduino-travis"
//   versionInfo    = "S"  -> returns VERSION string
//   ochGetCommand  = "r"  -> returns 87-byte output block
//
// INI OutputChannels (selected):
//   rpm             U16 @ 0
//   oilanalograw    U08 @ 3    (°C + 40)
//   mapraw          U16 @ 4    (kPa absolute)
//   coolantanalograw U08 @ 7   (°C + 40)
//   oilPressure     U08 @ 10   (PSI)
//   fuelPressure    U08 @ 11   (PSI)
//   fuellevel       U08 @ 15   (%)
//   vss             U16 @ 19   (kph)
//   leftTurn        U08 @ 40   (0/1)
//   rightTurn       U08 @ 41   (0/1)
//   cel             U08 @ 42   (0/1)
//   highBeam        U08 @ 43   (0/1)
//   handbrake       U08 @ 44   (0/1)
// =======================================================

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

// ===================== IDENTITY (INI MATCH) =====================
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

// =======================================================
// HALTECH CAN MAPPING (DEFAULTS / YOU MAY NEED TO ADJUST)
// -------------------------------------------------------
// Haltech CAN layouts can vary by ECU config and stream.
// These defaults are *common* patterns, but if any channel
// shows wrong, sniff the bus and update IDs/bytes/scales.
// =======================================================

// --------- Frame IDs (11-bit) ----------
static constexpr uint16_t ID_RPM_MAP      = 0x360; // rpm + map
static constexpr uint16_t ID_TEMPS        = 0x361; // clt + oil temp (example)
static constexpr uint16_t ID_PRESSURES    = 0x362; // oil psi + fuel psi (example)
static constexpr uint16_t ID_SPEED_FUEL   = 0x363; // speed kph + fuel % (example)
static constexpr uint16_t ID_INDICATORS   = 0x364; // turn/cel/high/handbrake (example)

// --------- Byte layouts / scaling ----------
// RPM:        U16 little-endian, units rpm
// MAP:        U16 little-endian, units kPa absolute
// CLT/OIL:    int16 or u16? (varies). We convert to °C then encode as U08 (°C + 40).
// Pressures:  usually kPa or PSI; we output PSI already scaled in firmware (U08).
// Speed:      U16 kph
// Fuel:       U08 percent
//
// If your Haltech stream uses different scaling (e.g. 0.1 kPa, 0.1°C), adjust here:
static constexpr float TEMP_SCALE = 1.0f;   // e.g. 0.1f if temp is in 0.1°C
static constexpr float MAP_SCALE  = 1.0f;   // e.g. 0.1f if map is in 0.1 kPa
static constexpr float RPM_SCALE  = 1.0f;   // e.g. 0.5f if half-RPM, etc.
static constexpr float PSI_SCALE  = 1.0f;   // e.g. 0.145038 if kPa->psi, etc.

// ===================== OUTPUT BLOCK =====================
static constexpr uint8_t OCH_BLOCK_SIZE = 87;  // ini: ochBlockSize = 87
static uint8_t och[OCH_BLOCK_SIZE];

// ===================== LIVE VALUES (decoded from CAN) =====================
static volatile uint16_t g_rpm = 0;
static volatile uint16_t g_map_kpa = 100;     // default ~atmosphere
static volatile int16_t  g_clt_c = 20;
static volatile int16_t  g_oil_c = 20;
static volatile uint8_t  g_oil_psi = 0;
static volatile uint8_t  g_fuel_psi = 0;
static volatile uint16_t g_speed_kph = 0;
static volatile uint8_t  g_fuel_pct = 0;

static volatile uint8_t  g_leftTurn = 0;
static volatile uint8_t  g_rightTurn = 0;
static volatile uint8_t  g_cel = 0;
static volatile uint8_t  g_highBeam = 0;
static volatile uint8_t  g_handbrake = 0;

// Optional: basic timeout protection
static uint32_t lastCanRxMs = 0;

// ===================== SMALL HELPERS =====================
static inline uint16_t u16le(const uint8_t *b) {
  return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}
static inline int16_t s16le(const uint8_t *b) {
  return (int16_t)((uint16_t)b[0] | ((uint16_t)b[1] << 8));
}

static inline uint8_t clampU8(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return (uint8_t)v;
}

static inline uint8_t tempC_to_iniRaw(int16_t tempC) {
  // INI expects U08 = (°C + 40)
  return clampU8((int)tempC + 40);
}

static void writeU16LE(uint8_t *dst, uint16_t v) {
  dst[0] = (uint8_t)(v & 0xFF);
  dst[1] = (uint8_t)(v >> 8);
}

// ===================== CAN RECEIVE =====================
static void handleCanFrame(uint16_t id, const uint8_t *buf, uint8_t len) {
  (void)len;
  lastCanRxMs = millis();

  switch (id) {
    case ID_RPM_MAP: {
      // [0..1]=RPM, [2..3]=MAP (kPa abs)
      uint16_t rpmRaw = u16le(&buf[0]);
      uint16_t mapRaw = u16le(&buf[2]);

      uint32_t rpm = (uint32_t)(rpmRaw * RPM_SCALE);
      uint32_t map = (uint32_t)(mapRaw * MAP_SCALE);

      if (rpm > 30000) rpm = 30000;
      if (map > 4000)  map = 4000;

      g_rpm = (uint16_t)rpm;
      g_map_kpa = (uint16_t)map;
    } break;

    case ID_TEMPS: {
      // Example: [0..1]=CLT, [2..3]=OilTemp (in °C * TEMP_SCALE)
      int16_t cltRaw = s16le(&buf[0]);
      int16_t oilRaw = s16le(&buf[2]);

      int16_t cltC = (int16_t)(cltRaw * TEMP_SCALE);
      int16_t oilC = (int16_t)(oilRaw * TEMP_SCALE);

      // sanity
      if (cltC < -40) cltC = -40;
      if (cltC > 215) cltC = 215;
      if (oilC < -40) oilC = -40;
      if (oilC > 215) oilC = 215;

      g_clt_c = cltC;
      g_oil_c = oilC;
    } break;

    case ID_PRESSURES: {
      // Example: [0]=oil psi, [1]=fuel psi (already PSI)
      // If yours is kPa, change PSI_SCALE and conversion.
      uint16_t oilRaw  = buf[0];
      uint16_t fuelRaw = buf[1];

      uint16_t oilPsi  = (uint16_t)(oilRaw * PSI_SCALE);
      uint16_t fuelPsi = (uint16_t)(fuelRaw * PSI_SCALE);

      if (oilPsi  > 255) oilPsi  = 255;
      if (fuelPsi > 255) fuelPsi = 255;

      g_oil_psi  = (uint8_t)oilPsi;
      g_fuel_psi = (uint8_t)fuelPsi;
    } break;

    case ID_SPEED_FUEL: {
      // Example: [0..1]=speed kph, [2]=fuel %
      g_speed_kph = u16le(&buf[0]);
      g_fuel_pct  = buf[2];
      if (g_fuel_pct > 100) g_fuel_pct = 100;
    } break;

    case ID_INDICATORS: {
      // Example: packed bits in buf[0]
      // bit0=left, bit1=right, bit2=cel, bit3=high, bit4=handbrake
      uint8_t bits = buf[0];
      g_leftTurn  = (bits & (1 << 0)) ? 1 : 0;
      g_rightTurn = (bits & (1 << 1)) ? 1 : 0;
      g_cel       = (bits & (1 << 2)) ? 1 : 0;
      g_highBeam  = (bits & (1 << 3)) ? 1 : 0;
      g_handbrake = (bits & (1 << 4)) ? 1 : 0;
    } break;

    default:
      break;
  }
}

// ===================== BUILD THE 87-BYTE OCH BLOCK =====================
static void buildOchBlock() {
  // Clear everything (anything not defined in your INI stays 0)
  memset(och, 0, sizeof(och));

  // Match INI offsets exactly :contentReference[oaicite:4]{index=4}
  // rpm: U16 @ 0
  writeU16LE(&och[0], g_rpm);

  // oilanalograw: U08 @ 3  (°C + 40)
  och[3] = tempC_to_iniRaw(g_oil_c);

  // mapraw: U16 @ 4 (kPa abs)
  writeU16LE(&och[4], g_map_kpa);

  // coolantanalograw: U08 @ 7 (°C + 40)
  och[7] = tempC_to_iniRaw(g_clt_c);

  // oilPressure: U08 @ 10 (PSI)
  och[10] = g_oil_psi;

  // fuelPressure: U08 @ 11 (PSI)
  och[11] = g_fuel_psi;

  // fuellevel: U08 @ 15 (%)
  och[15] = g_fuel_pct;

  // vss: U16 @ 19 (kph)
  writeU16LE(&och[19], g_speed_kph);

  // indicators: U08 @ 40..44
  och[40] = g_leftTurn;
  och[41] = g_rightTurn;
  och[42] = g_cel;
  och[43] = g_highBeam;
  och[44] = g_handbrake;
}

// ===================== TS SERIAL COMMAND HANDLING =====================
// TS will send:
//  'Q' -> expects signature string :contentReference[oaicite:5]{index=5}
//  'S' -> expects version string :contentReference[oaicite:6]{index=6}
//  'r' -> expects ochBlockSize bytes :contentReference[oaicite:7]{index=7}
//
// Important: Do NOT print debug text while TS is connected.
static void handleSerialByte(uint8_t c) {
  if (c == 'Q') {
    Serial.print(SIGNATURE);
    return;
  }
  if (c == 'S') {
    Serial.print(VERSION);
    return;
  }
  if (c == 'r') {
    buildOchBlock();
    Serial.write(och, OCH_BLOCK_SIZE);
    return;
  }
  // ignore anything else (keeps TS happy)
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(CAN_INT_PIN, INPUT);

  if (CAN.begin(MCP_ANY, CAN_SPEED, CAN_CLOCK) != CAN_OK) {
    // No Serial prints here if you want TS to connect cleanly later.
    // If you need debug, temporarily uncomment this:
    // Serial.println("CAN INIT FAILED");
    while (1) {;}
  }

  CAN.setMode(MCP_NORMAL);
  lastCanRxMs = millis();
}

// ===================== LOOP =====================
void loop() {
  // --- CAN service ---
  if (!digitalRead(CAN_INT_PIN)) {
    long unsigned int rxId32;
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN.readMsgBuf(&rxId32, &len, buf) == CAN_OK) {
      uint16_t id = (uint16_t)(rxId32 & 0x7FF); // 11-bit
      handleCanFrame(id, buf, len);
    }
  }

  // --- Optional: CAN timeout fallback (prevent stale numbers) ---
  if (millis() - lastCanRxMs > 1000) {
    g_rpm = 0;
    g_speed_kph = 0;
    // keep temps/map last-known; you can zero them too if you prefer
  }

  // --- Serial service ---
  while (Serial.available()) {
    handleSerialByte((uint8_t)Serial.read());
  }
}
#endif