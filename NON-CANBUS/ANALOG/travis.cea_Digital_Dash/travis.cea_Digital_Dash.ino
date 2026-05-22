// =======================================================
// Travis Digital Dash – TunerStudio Interface
// FINAL PRODUCTION BUILD  +  BATT VOLTAGE
// Arduino Mega 2560
// Copyright (c) 2025 Travis Way
// Signature: speeduino-travis
// =======================================================

#include <Arduino.h>
#include <math.h>
#include <EEPROM.h>

// =======================================================
// USER CONFIG: SENSOR SCALING + TUNING (SAFE TO EDIT)
// =======================================================

// ---------- ODOMETER ----------
// ODOMETER_INITIAL_MILES
// • Used ONLY on first-ever boot
// • Ignored once EEPROM is initialized
// • Change only if replacing cluster or resetting EEPROM
static constexpr float ODOMETER_INITIAL_MILES = 2200.0f;


// ---------- ADC / VREF ----------
static constexpr float ADC_MAX = 1023.0f;
static constexpr float VREF    = 5.0f;

// ---------- EMA FILTERING ----------
static constexpr float EMA_ALPHA_FAST = 0.12f;
static constexpr float EMA_ALPHA_SLOW = 0.08f;


// ----------- BATT VOLTAGE ------------
static constexpr float    VBAT_R1 = 22000.0f; // 22k
static constexpr float    VBAT_R2 = 10000.0f; // 10k
static constexpr float    VBAT_DIV_RATIO = (VBAT_R1 + VBAT_R2) / VBAT_R2;
static constexpr float    VBAT_SANITY_MIN = 9.0f;
static constexpr float    VBAT_SANITY_MAX = 17.0f; //Some Alternators can hit 17v on startup

// ---------- PRESSURE SENDERS ----------
static constexpr float PRESSURE_V_MIN = 0.5f;
static constexpr float PRESSURE_V_MAX = 4.5f;
static constexpr float PRESSURE_PSI_MAX = 145.0f;
static constexpr float PRESSURE_PSI_PER_V = PRESSURE_PSI_MAX / (PRESSURE_V_MAX - PRESSURE_V_MIN);

// Acceptable voltage sanity window (0.45–4.75)
static constexpr float PRESSURE_V_SANITY_MIN = 0.45f;
static constexpr float PRESSURE_V_SANITY_MAX = 4.75f;

// ---------- ProSport BOOST SENDER (Voltage -> PSI lookup, then PSI -> kPa) ----------
static constexpr uint8_t BOOST_NPTS = 5;
static constexpr float BOOST_V[BOOST_NPTS]   = {0.02f, 1.00f, 2.00f, 3.00f, 4.00f};
static constexpr float BOOST_PSI[BOOST_NPTS] = {-14.5f, 0.0f, 14.5f, 29.0f, 43.51f};
static constexpr float BOOST_FILTER_A = 0.25f;
static constexpr float PSI_TO_KPA     = 6.89476f;
static constexpr float BOOST_ZERO_TRIM_PSI = 8.0f; // Zero-offset calibration (measured KOEO error)

// ---------- ProSport TEMP SENDER (2-wire) ----------
static constexpr float THERM_PULLUP = 1000.0f; // 1.0k pull-up to 5V

static constexpr uint8_t PS_NPTS = 7;
static constexpr float PS_TEMP_F[PS_NPTS] = {104, 140, 176, 212, 248, 284, 302};
static constexpr float PS_OHMS[PS_NPTS]   = {5830, 3020, 1670, 975, 599, 386, 316};

static constexpr float TEMP_F_FLOOR = 100.0f; // “ProSport-style floor”
static constexpr float TEMP_F_CEIL  = 302.0f; // clamp

// Temp ADC sanity 
static constexpr float TEMP_ADC_SANITY_MIN = 5.0f;
static constexpr float TEMP_ADC_SANITY_MAX = 1018.0f;

// ---------- FUEL LEVEL (sender ohms -> %) ----------
static constexpr float FUEL_PULLUP_OHMS = 330.0f;
static constexpr float FUEL_OHMS_EMPTY  = 100.0f;

// Fuel boot/validity guard
static constexpr uint32_t FUEL_SETTLE_MS = 1500;
static constexpr float    FUEL_V_MIN_OK  = 0.05f;
static constexpr float    FUEL_V_MAX_OK  = 4.95f;

// ---------- RPM ----------
static constexpr uint8_t  PULSES_PER_REV   = 2;     // Subaru tach
static constexpr uint32_t RPM_MIN_PULSE_US = 3000;  // Noise reject
static constexpr float    RPM_MAX          = 9000.0f;
static constexpr uint32_t RPM_STOP_TIMEOUT_MS = 400;
static constexpr float    DECEL_RPM_PER_SEC = 22000.0f;

// ---------- VSS ----------
static constexpr uint32_t VSS_MIN_PULSE_US = 500;
static constexpr float    VSS_PULSES_PER_KM = 8000.0f;
static constexpr uint32_t VSS_MIN_VALID_PERIOD_US = 1500;    // speed spike guard
static constexpr uint32_t VSS_STOP_TIMEOUT_US = 2500000;
static constexpr float    VSS_MAX_KPH = 300.0f;
volatile uint16_t vssPulseCount = 0;
volatile uint32_t vssLastPulseUs = 0;
volatile uint32_t vssWindowStartUs = 0;
static constexpr uint8_t VSS_MIN_PULSES_TO_MOVE = 6;
static constexpr float VSS_MIN_UNLOCK_KPH = 8.0f; // ~5 mph
static uint8_t vssValidPulseStreak = 0;
static constexpr float VSS_IDLE_RPM_CUTOFF = 1800.0f;




static bool vssMoving = false;

// ---------- OCH OUTPUT ----------
static constexpr uint8_t OCH_BLOCK_SIZE = 87;

// ---------- UPDATE RATES ----------
static constexpr uint8_t SLOW_FRAME_DIV_MAX = 5; // slowUpdate at ~5 Hz if base is ~25 Hz
static constexpr uint8_t PRESSURE_DIV_MAX   = 2; // pressure at ~12.5 Hz if base is ~25 Hz
static constexpr uint32_t TURN_HOLD_MS      = 300;

// =======================================================
// IDENTITY (fixed)
// =======================================================
static const char SIGNATURE[32] = "speeduino-travis";
static const char VERSION[32]   = "Travis Digital Dash v1.0";



// =======================================================
// Odometer
// =======================================================
#define EEPROM_ODO_ADDR 0
#define EEPROM_MAGIC_ADDR  8
#define EEPROM_MAGIC_VALUE 0xC0DE
// =======================================================
// SERIAL
// =======================================================
#define BAUD_RATE 115200

// =======================================================
// PINS
// =======================================================
#define OIL_PRESSURE_PIN   A0
#define FUEL_PRESSURE_PIN  A1
#define CLT_TEMP_PIN       A2
#define OIL_TEMP_PIN       A3
#define BOOST_PIN          A4
#define FUEL_LEVEL_PIN     A5
#define BATT_VOLTAGE_PIN   A6

#define RPM_PIN 2
#define VSS_PIN 3

#define TURN_LEFT_PIN    22
#define TURN_RIGHT_PIN   23
#define CEL_PIN          24
#define HIGH_BEAM_PIN    25
#define HANDBRAKE_PIN    26

// =======================================================
// RATE CONTROL / CACHED VALUES
// =======================================================
static uint8_t slowFrameDiv = 0;

// Cached all sensor values
static uint16_t mapCached = 0; // (kept for compatibility; not used in your current file)

static uint8_t  cltCached  = 0;
static uint8_t  oilTCached = 0;
static uint8_t  fuelCached = 0;

static uint8_t  oilPCached  = 0;
static uint8_t  fuelPCached = 0;
static bool vssEverSeenPulse = false;

// =======================================================
// OUTPUT BUFFER
// =======================================================
uint8_t och[OCH_BLOCK_SIZE];

// =======================================================
// STATE
// =======================================================
float emaOilP=NAN, emaFuelP=NAN;
float emaCltT=NAN, emaOilT=NAN;
float emaFuelLvl=NAN;

// ---- Fuel input guard (prevents floating ADC poisoning)
static uint32_t fuelBootMs = 0;
static bool fuelValidSeen = false;

// RPM/VSS
static uint32_t lastPulseMs = 0;
static float vssFreqEma = NAN;
volatile uint32_t tachPeriodUs = 0;
volatile uint32_t lastTachUs   = 0;
volatile uint32_t vssPeriodUs = 0;
volatile uint32_t vssLastUs   = 0;
volatile float engineRpm = 0.0f;


float rpmNow = 0.0f;
float rpmFiltered = 0.0f;

float kphNow=0, kphEma=NAN;

static uint32_t tlHold = 0;
static uint32_t trHold = 0;

static float odometerMiles = 0.0f;
static uint32_t lastOdoMs = 0;

static float emaVbat = NAN;
static uint16_t vbatCached = 0; // scaled for TS

// =======================================================
// HELPERS
// =======================================================
void loadOdometer() {
  uint16_t magic;
  EEPROM.get(EEPROM_MAGIC_ADDR, magic);

  if (magic != EEPROM_MAGIC_VALUE) {
    // First boot / EEPROM not initialized
    odometerMiles = ODOMETER_INITIAL_MILES;   // ← THIS LINE WAS MISSING
    EEPROM.put(EEPROM_ODO_ADDR, odometerMiles);
    EEPROM.put(EEPROM_MAGIC_ADDR, (uint16_t)EEPROM_MAGIC_VALUE);
    return;
  }

  EEPROM.get(EEPROM_ODO_ADDR, odometerMiles);
  if (!isfinite(odometerMiles) || odometerMiles < 0)
    odometerMiles = 0.0f;
}



void saveOdometer() {
  EEPROM.put(EEPROM_ODO_ADDR, odometerMiles);
}

void shouldSaveOdometer(){
  static float lastSaved = 0;
  if (fabs(odometerMiles - lastSaved) > 0.1f) {
    saveOdometer();
    lastSaved = odometerMiles;
  }
}

static inline uint8_t clampU8(int v){
  if(v<0) return 0;
  if(v>255) return 255;
  return v;
}

uint16_t readBatteryVoltage_x10(float &ema) {
  float adc = readAdcFiltered(BATT_VOLTAGE_PIN, ema, 8, 1, EMA_ALPHA_SLOW);
  float vAdc = adc * (VREF / ADC_MAX);
  float vBat = vAdc * VBAT_DIV_RATIO;

  // Sanity clamp
  if (vBat < VBAT_SANITY_MIN || vBat > VBAT_SANITY_MAX)
    return vbatCached; // hold last good value

  return (uint16_t)(vBat * 10.0f + 0.5f);
}

// --- ProSport interpolation helper ---
static float prosportTempF_fromOhms(float r){
  if (r >= PS_OHMS[0]) return PS_TEMP_F[0];
  if (r <= PS_OHMS[PS_NPTS-1]) return PS_TEMP_F[PS_NPTS-1];

  for (uint8_t i = 0; i < PS_NPTS - 1; i++) {
    float r_hi = PS_OHMS[i];
    float r_lo = PS_OHMS[i + 1];

    if (r <= r_hi && r >= r_lo) {
      float t_hi = PS_TEMP_F[i];
      float t_lo = PS_TEMP_F[i + 1];

      float x  = log(r);
      float xh = log(r_hi);
      float xl = log(r_lo);

      float frac = (x - xh) / (xl - xh);
      return t_hi + frac * (t_lo - t_hi);
    }
  }
  return PS_TEMP_F[PS_NPTS-1];
}

// =======================================================
// ADC FILTER
// =======================================================
float readAdcFiltered(int pin, float &ema, uint8_t samples, uint8_t trim, float alpha){
  analogRead(pin);
  int buf[16];
  for(uint8_t i=0;i<samples;i++){
    buf[i]=analogRead(pin);
    delayMicroseconds(60);
  }
  for(uint8_t i=0;i<samples-1;i++)
    for(uint8_t j=0;j<samples-1-i;j++)
      if(buf[j]>buf[j+1]){
        int t=buf[j]; buf[j]=buf[j+1]; buf[j+1]=t;
      }

  long sum=0;
  for(uint8_t i=trim;i<samples-trim;i++) sum+=buf[i];
  float avg=(float)sum/(samples-(trim*2));

  if(isnan(ema)) ema=avg;
  else ema+=alpha*(avg-ema);
  return ema;
}

// =======================================================
// CONVERSIONS (same outputs, constants moved up)
// =======================================================
uint8_t readPressurePSI(int pin,float &ema){
  float v=readAdcFiltered(pin,ema,16,2,EMA_ALPHA_FAST)*(VREF/ADC_MAX);
  if(v<PRESSURE_V_SANITY_MIN || v>PRESSURE_V_SANITY_MAX) return 0;
  return clampU8((int)((v-PRESSURE_V_MIN)*PRESSURE_PSI_PER_V+0.5f));
}

static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

float readBoostKpa_Prosport(int pin) {
  static float filteredPsi = NAN;

  uint32_t acc = 0;
  for (int i = 0; i < 8; i++) acc += analogRead(pin);
  float raw = acc / 8.0f;

  float v = (raw * VREF) / ADC_MAX;

  float psi;

  if (v <= BOOST_V[0]) {
    psi = BOOST_PSI[0];
  } else if (v >= BOOST_V[BOOST_NPTS - 1]) {
    psi = BOOST_PSI[BOOST_NPTS - 1];
  } else {
    for (int i = 0; i < BOOST_NPTS - 1; i++) {
      if (v <= BOOST_V[i + 1]) {
        float t = (v - BOOST_V[i]) / (BOOST_V[i + 1] - BOOST_V[i]);
        psi = lerp(BOOST_PSI[i], BOOST_PSI[i + 1], t);
        break;
      }
    }
  }

  // Apply KOEO zero trim
  psi += BOOST_ZERO_TRIM_PSI;

  // EMA filter (CRITICAL INIT)
  if (isnan(filteredPsi))
    filteredPsi = psi;
  else
    filteredPsi += BOOST_FILTER_A * (psi - filteredPsi);

  // Convert to gauge kPa
  float boostKpa = filteredPsi * PSI_TO_KPA;

  // Physical clamps
  if (boostKpa < -101.325f) boostKpa = -101.325f;
  if (boostKpa > 300.0f)    boostKpa = 300.0f;

  return boostKpa;
}


uint8_t readTempC40_Prosport(int pin, float &ema){
  float a = readAdcFiltered(pin, ema, 4, 1, EMA_ALPHA_SLOW);

  // ADC sanity (open/short)
  if (a < TEMP_ADC_SANITY_MIN || a > TEMP_ADC_SANITY_MAX) {
    float tempC = (TEMP_F_FLOOR - 32.0f) * (5.0f / 9.0f);
    return clampU8((int)(tempC + 40.0f + 0.5f));
  }

  // Resistance from ADC counts (no VREF dependence)
  float r = THERM_PULLUP * (a / (ADC_MAX - a));

  float tempF = prosportTempF_fromOhms(r);

  // ProSport-style floor
  if (tempF < TEMP_F_FLOOR) tempF = TEMP_F_FLOOR;

  // Optional upper clamp
  if (tempF > TEMP_F_CEIL) tempF = TEMP_F_CEIL;

  float tempC = (tempF - 32.0f) * (5.0f / 9.0f);

  return clampU8((int)(tempC + 40.0f + 0.5f));
}

// Fuel sender -> %
uint8_t fuelPctFromOhms(float r) {

  // Conservative empty (protect against running dry)
  if (r >= FUEL_OHMS_EMPTY)
    return 0;

  // Linear base
  float pct = 100.0f * (1.0f - (r / FUEL_OHMS_EMPTY));

  // Gentle top compression (never optimistic)
  if (pct > 85.0f)
    pct = 85.0f + (pct - 85.0f) * 0.15f;

  // Clamp hard limits
  if (pct < 0.0f)   pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;

  return clampU8((int)(pct + 0.5f));
}





// =======================================================
// RPM / VSS ISR
// =======================================================
void isrRPM() {
  uint32_t now = micros();
  uint32_t dt  = now - lastTachUs;

  lastTachUs = now;

  // Reject noise / impossible pulses
  if (dt > RPM_MIN_PULSE_US && dt < 80000) { // ~375 RPM floor
    tachPeriodUs = dt;
    lastPulseMs  = millis();
  }
}

void isrVSS() {
  uint32_t now = micros();

  static uint32_t lastUs = 0;
  uint32_t dt = now - lastUs;
  if (dt < 800) return;          // noise reject (fast spikes)
  lastUs = now;

  vssLastPulseUs = now;

  // Start window on first seen pulse
  if (vssWindowStartUs == 0) {
    vssWindowStartUs = now;
  }

  vssPulseCount++;
  vssEverSeenPulse = true;
}





// =======================================================
// updateRPM
// =======================================================
void updateRPM() {
  static uint32_t lastUpdateUs = 0;

  // Snapshot shared data
  uint32_t period;
  uint32_t pulseMs;
  noInterrupts();
  period  = tachPeriodUs;
  pulseMs = lastPulseMs;
  interrupts();

  // Timeout: engine stopped
  if (millis() - pulseMs > RPM_STOP_TIMEOUT_MS) {
    rpmFiltered = 0.0f;
    rpmNow = 0.0f;
    lastUpdateUs = 0;
    return;
  }

  // Must have a valid period
  if (period == 0 || period < RPM_MIN_PULSE_US) {
    rpmNow = rpmFiltered;
    return;
  }

  float rpmRaw = 60000000.0f / (period * PULSES_PER_REV);

  // Hard sanity clamp FIRST
  if (rpmRaw < 0.0f)    rpmRaw = 0.0f;
  if (rpmRaw > RPM_MAX) rpmRaw = RPM_MAX;

  // THEN spike rejection
  if (rpmFiltered > 0.0f) {
    float delta = fabs(rpmRaw - rpmFiltered);
    if (delta > 2000.0f) {
      rpmNow = rpmFiltered;
      return;
    }
  }

  // Compute real dt for decel damping
  uint32_t nowUs = micros();
  float dtSec = (lastUpdateUs == 0) ? 0.01f : (nowUs - lastUpdateUs) * 1e-6f;
  lastUpdateUs = nowUs;

  if (dtSec < 0.0005f) dtSec = 0.0005f;
  if (dtSec > 0.02f)  dtSec = 0.02f;

  // Acceleration: immediate
  if (rpmRaw >= rpmFiltered) {
    rpmFiltered = rpmRaw;
    rpmNow = rpmFiltered;
    return;
  }

  // Deceleration: light physical damping
  float maxDrop = DECEL_RPM_PER_SEC * dtSec;

  float delta = rpmRaw - rpmFiltered;
  if (delta < -maxDrop)
    rpmFiltered -= maxDrop;
  else
    rpmFiltered = rpmRaw;

  rpmNow = rpmFiltered;
  engineRpm = rpmFiltered;
}

// =======================================================
// updateVSS
// =======================================================
void updateVSS() {
  const uint32_t WINDOW_US = 250000;      // 250 ms
  const uint32_t CONTINUOUS_US = 300000;  // must be seeing pulses recently to "start moving"
  const uint8_t  START_GOOD_WINDOWS = 4;  // consecutive good windows required to unlock motion

  static uint8_t startGood = 0;

  uint32_t now = micros();

  uint16_t pulses;
  uint32_t lastPulse;
  uint32_t winStart;

  // -------------------------------
  // Atomic snapshot
  // -------------------------------
  noInterrupts();
  pulses    = vssPulseCount;
  lastPulse = vssLastPulseUs;
  winStart  = vssWindowStartUs;
  interrupts();

  // Never saw any pulse since boot
  if (!vssEverSeenPulse) {
    kphNow = 0.0f;
    kphEma = 0.0f;
    vssMoving = false;
    startGood = 0;
    return;
  }

  // -------------------------------
  // STOP timeout (ONLY hard zero while moving)
  // -------------------------------
  if (now - lastPulse > VSS_STOP_TIMEOUT_US) {
    // Only treat as stop if we were already basically stopped
    if (!vssMoving || kphEma < 5.0f) {
      kphNow = 0.0f;
      kphEma = 0.0f;
      vssMoving = false;
      startGood = 0;

      noInterrupts();
      vssPulseCount = 0;
      vssWindowStartUs = 0;
      interrupts();
    } else {
      // Glitch while cruising → HOLD speed
      kphNow = kphEma;
    }

    return;
  }


  // Wait for window to complete
  if (now - winStart < WINDOW_US) {
    kphNow = vssMoving ? kphEma : 0.0f;
    return;
  }

  // Consume pulses and advance window
  noInterrupts();
  vssPulseCount = 0;
  vssWindowStartUs = now;
  interrupts();

  // Convert pulses → kph (windowed)
  float pulsesPerSec = pulses * (1e6f / WINDOW_US);
  float kphRaw = (pulsesPerSec * 3600.0f) / VSS_PULSES_PER_KM;

  // Deadband
  if (kphRaw < 2.0f) kphRaw = 0.0f;

  // --------------------------------
  // NOT MOVING: require sustained evidence to unlock
  // --------------------------------
  if (!vssMoving) {

    // Block idle-correlated VSS chatter (only while trying to unlock)
    if (engineRpm < VSS_IDLE_RPM_CUTOFF) {
      startGood = 0;
      kphNow = 0.0f;
      kphEma = 0.0f;
      return;
    }
    // If pulses are not continuous, treat as noise and stay at 0
    if ((now - lastPulse) > CONTINUOUS_US) {
      startGood = 0;
      kphNow = 0.0f;
      kphEma = 0.0f;
      return;
    }

    // Require enough pulses AND reasonable speed for multiple windows
    if (pulses >= VSS_MIN_PULSES_TO_MOVE && kphRaw >= 5.0f) {
      if (startGood < 255) startGood++;
    } else {
      startGood = 0;
    }

    // Not unlocked yet
    if (startGood < START_GOOD_WINDOWS) {
      kphNow = 0.0f;
      kphEma = 0.0f;
      return;
    }

    // Unlock motion
    vssMoving = true;
    kphEma = kphRaw;
    kphNow = kphEma;
    return;
  }

  // --------------------------------
  // MOVING: never drop to 0 due to a single bad window
  // --------------------------------

  // If too few pulses this window while moving,
  // allow speed to decay toward zero
  if (pulses < VSS_MIN_PULSES_TO_MOVE) {

    // Decay speed gradually (simulates rolling to a stop)
    kphEma *= 0.75f;   // ~25% drop per window (~1 sec to zero)

    if (kphEma < 1.5f) {
      kphEma = 0.0f;
      vssMoving = false;   // now officially stopped
      startGood = 0;
    }

    kphNow = kphEma;
    return;
  }


  // Prevent sudden drops (glitch protection)
  if (isfinite(kphEma) && kphRaw < kphEma - 4.0f) {
    kphRaw = kphEma - 4.0f;
  }

  // Smooth output
  if (isnan(kphEma)) {
    kphEma = kphRaw;
  } else {
    float alpha = (kphEma < 10.0f) ? 0.08f : 0.25f;
    kphEma += alpha * (kphRaw - kphEma);
  }

  kphNow = kphEma;
}





// =======================================================
// UPDATE ODOMETER
// =======================================================
void updateOdometer(float speedKph) {
  uint32_t now = millis();
  if (lastOdoMs == 0) {
    lastOdoMs = now;
    return;
  }

  float dtHours = (now - lastOdoMs) / 3600000.0f;
  lastOdoMs = now;

  if (speedKph > 0.5f) {   // noise guard
    float speedMph = speedKph * 0.621371f;
    odometerMiles += speedMph * dtHours;
  }
}


// =======================================================
// OUTPUT PACKET
// =======================================================
void updateOch(){
  memset(och, 0, OCH_BLOCK_SIZE);  // REQUIRED
  slowFrameDiv++;
  bool slowUpdate = (slowFrameDiv >= SLOW_FRAME_DIV_MAX);
  if (slowUpdate) slowFrameDiv = 0;

  uint32_t now = millis();

  // Pressure readings slower than RPM
  static uint8_t pressureDiv = 0;
  pressureDiv++;

  if (pressureDiv >= PRESSURE_DIV_MAX) {
    pressureDiv = 0;
    oilPCached  = readPressurePSI(OIL_PRESSURE_PIN, emaOilP);
    fuelPCached = readPressurePSI(FUEL_PRESSURE_PIN, emaFuelP);
  }

  och[10] = oilPCached;
  och[11] = fuelPCached;

  // Temp readings slower than RPM and slower than pressure
  if (slowUpdate) {
    cltCached  = readTempC40_Prosport(CLT_TEMP_PIN, emaCltT);
    oilTCached = readTempC40_Prosport(OIL_TEMP_PIN, emaOilT);
  }

  och[7] = cltCached;
  och[3] = oilTCached;

  int16_t kpa = (int16_t)(readBoostKpa_Prosport(BOOST_PIN) + 0.5f);
  och[4] = kpa & 0xFF;
  och[5] = (kpa >> 8) & 0xFF;

  // =======================
  // Fuel Level (engine-safe)
  // =======================
  if (slowUpdate) {
    uint16_t fadc = (uint16_t)readAdcFiltered(
        FUEL_LEVEL_PIN,
        emaFuelLvl,
        16,
        3,
        EMA_ALPHA_SLOW
    );

    float vFuel = fadc * (VREF / ADC_MAX); // keep for logging

    // Only reject true dead short / open
    if (fadc <= 1 || fadc >= (ADC_MAX - 1)) {
      // do nothing – hold last value
    } else {
      float rFuel = FUEL_PULLUP_OHMS * ((float)fadc / (ADC_MAX - fadc));
      fuelCached = fuelPctFromOhms(rFuel);
      
    }
  }
  och[15] = fuelCached;



  // RPM
  uint16_t r = (uint16_t)(rpmNow + 0.5f);
  och[0] = r & 0xFF;
  och[1] = r >> 8;

  // Speed (kph)
  uint16_t v = (uint16_t)constrain((int)(kphNow + 0.5f), 0, 65535);
  och[19] = v & 0xFF;
  och[20] = v >> 8;

  // Indicators
  bool tl  = (digitalRead(TURN_LEFT_PIN)  == LOW);
  bool tr  = (digitalRead(TURN_RIGHT_PIN) == LOW);
  bool cel = (digitalRead(CEL_PIN)        == LOW);
  bool hi  = (digitalRead(HIGH_BEAM_PIN)  == LOW);
  bool hb  = (digitalRead(HANDBRAKE_PIN)  == LOW);

  if (tl) tlHold = now;
  if (tr) trHold = now;

  och[40] = (now - tlHold < TURN_HOLD_MS) ? 1 : 0;
  och[41] = (now - trHold < TURN_HOLD_MS) ? 1 : 0;
  och[42] = cel ? 1 : 0;
  och[43] = hi  ? 1 : 0;
  och[44] = hb  ? 1 : 0;


  // =======================
  // Odometer (miles, float)
  // =======================
  float odo = odometerMiles;
  uint8_t *p = (uint8_t*)&odo;

  och[60] = p[0];
  och[61] = p[1];
  och[62] = p[2];
  och[63] = p[3];

  // -----------------------
  // Battery Voltage
  // -----------------------
  if (slowUpdate) {
    vbatCached = readBatteryVoltage_x10(emaVbat);
  }

  och[22] = vbatCached & 0xFF;
  och[23] = vbatCached >> 8;


}

// =======================================================
// SERIAL
// =======================================================
uint16_t readU16LE(){
  uint32_t t0 = millis();
  while (Serial.available() < 2) {
    if (millis() - t0 > 50) {
      // Drain any partial bytes so not to desync the command stream
      while (Serial.available()) Serial.read();
      return 0;
    }
  }
  uint8_t lo = Serial.read();
  uint8_t hi = Serial.read();
  return (hi << 8) | lo;
}

// =======================================================
// SETUP 
// =======================================================
void setup(){
  loadOdometer();
  analogReference(DEFAULT);
  Serial.begin(BAUD_RATE);

  pinMode(TURN_LEFT_PIN, INPUT_PULLUP);
  pinMode(TURN_RIGHT_PIN, INPUT_PULLUP);
  pinMode(CEL_PIN, INPUT_PULLUP);
  pinMode(HIGH_BEAM_PIN, INPUT_PULLUP);
  pinMode(HANDBRAKE_PIN, INPUT_PULLUP);

  vbatCached = readBatteryVoltage_x10(emaVbat);

  pinMode(RPM_PIN, INPUT);
  pinMode(VSS_PIN, INPUT);

  fuelBootMs = millis();

  attachInterrupt(digitalPinToInterrupt(RPM_PIN), isrRPM, FALLING);
  attachInterrupt(digitalPinToInterrupt(VSS_PIN), isrVSS, FALLING);
}


// =======================================================
// LOOP
// =======================================================
void loop(){
  updateRPM();
  updateVSS();
  updateOdometer(kphNow);
  shouldSaveOdometer();

  while (Serial.available()) {
    char c = Serial.read();

    if (c == 'Q') { Serial.write((const uint8_t*)SIGNATURE, 32); }
    else if (c == 'S') { Serial.write((const uint8_t*)VERSION, 32); }
    else if (c == 'F') { uint8_t f[3]={0,0,0}; Serial.write(f,3); }
    else if (c == 'r') {
      readU16LE();
      readU16LE();
      updateOch();
      Serial.write(och, OCH_BLOCK_SIZE);
    }
    else if (c == 'p') {
      readU16LE(); readU16LE();
      uint16_t len = readU16LE();
      static uint8_t z[288];
      memset(z, 0, min(len, (uint16_t)288));
      Serial.write(z, min(len, (uint16_t)288));
    }
    else if (c == 'b') { readU16LE(); }
    else if (c == 'd') { readU16LE(); uint32_t crc=0; Serial.write((uint8_t*)&crc,4); }
  }
}
