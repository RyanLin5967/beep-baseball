/*
 * hit_counter — impact counter for the durability test rig.
 *
 * Counts impacts on the rig from an accelerometer. (The ball itself is a 555 + piezo
 * with no microcontroller; this runs on the rig.)
 *
 * Hardware (analog, Arduino Uno/Nano):
 *   A0  ADXL335 accelerometer, impact axis
 *   A1  electret mic module analog out
 *   D13 status LED
 *
 * Output (Serial, 115200): millis,hit_count,accel_peak,beeping
 */

const uint8_t PIN_ACCEL = A0;
const uint8_t PIN_MIC   = A1;
const uint8_t PIN_LED   = 13;

const int   ACCEL_REST        = 512;   // measure your sensor's rest value
const int   HIT_THRESHOLD     = 180;   // |accel - rest| above this counts as a hit
const unsigned long HIT_DEBOUNCE_MS = 150; // ignore ringing after one hit

const int   BEEP_FLOOR        = 40;    // mic peak-to-peak: ball ON vs OFF
const unsigned long MIC_WINDOW_US = 2000; // mic sampling window
const int   SILENCE_STREAK_TO_FAIL = 5;   // consecutive silent checks -> beep dead

unsigned long hitCount = 0;
unsigned long lastHitMs = 0;
int silenceStreak = 0;
bool beepLost = false;
unsigned long failedAtHit = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.begin(115200);
  Serial.println(F("# millis,hit_count,accel_peak,beeping"));
}

// Peak-to-peak mic amplitude over a short window.
int micPeakToPeak() {
  int lo = 1023, hi = 0;
  unsigned long start = micros();
  while (micros() - start < MIC_WINDOW_US) {
    int v = analogRead(PIN_MIC);
    if (v < lo) lo = v;
    if (v > hi) hi = v;
  }
  return hi - lo;
}

bool isBeeping() {
  return micPeakToPeak() >= BEEP_FLOOR;
}

void logRow(unsigned long now, int peak, bool beeping) {
  Serial.print(now);      Serial.print(',');
  Serial.print(hitCount); Serial.print(',');
  Serial.print(peak);     Serial.print(',');
  Serial.println(beeping ? 1 : 0);
}

void loop() {
  unsigned long now = millis();
  int peak = abs(analogRead(PIN_ACCEL) - ACCEL_REST);

  bool beeping = isBeeping();
  if (!beeping) {
    if (silenceStreak < 32767) silenceStreak++;
  } else {
    silenceStreak = 0;
  }
  if (!beepLost && silenceStreak >= SILENCE_STREAK_TO_FAIL) {
    beepLost = true;
    failedAtHit = hitCount;
    digitalWrite(PIN_LED, HIGH); // latch
    Serial.print(F("# FAILURE at hit "));
    Serial.println(failedAtHit);
  }

  if (peak > HIT_THRESHOLD && (now - lastHitMs) > HIT_DEBOUNCE_MS) {
    hitCount++;
    lastHitMs = now;
    logRow(now, peak, !beepLost);
  }
}
