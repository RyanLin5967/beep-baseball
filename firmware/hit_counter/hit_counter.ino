/*
 * hit_counter — impact counter for the durability test rig.
 *
 * Counts impacts on the rig from an accelerometer. (The ball itself is a 555 + piezo
 * with no microcontroller; this runs on the rig.)
 *
 * Hardware (analog, Arduino Uno/Nano):
 *   A0  ADXL335 accelerometer, impact axis
 *   D13 status LED
 *
 * Output (Serial, 115200): millis,hit_count,accel_peak
 */

const uint8_t PIN_ACCEL = A0;
const uint8_t PIN_LED   = 13;

const int   ACCEL_REST        = 512;   // measure your sensor's rest value
const int   HIT_THRESHOLD     = 180;   // |accel - rest| above this counts as a hit
const unsigned long HIT_DEBOUNCE_MS = 150; // ignore ringing after one hit

unsigned long hitCount = 0;
unsigned long lastHitMs = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.begin(115200);
  Serial.println(F("# millis,hit_count,accel_peak"));
}

void logRow(unsigned long now, int peak) {
  Serial.print(now);      Serial.print(',');
  Serial.print(hitCount); Serial.print(',');
  Serial.println(peak);
}

void loop() {
  unsigned long now = millis();
  int peak = abs(analogRead(PIN_ACCEL) - ACCEL_REST);

  if (peak > HIT_THRESHOLD && (now - lastHitMs) > HIT_DEBOUNCE_MS) {
    hitCount++;
    lastHitMs = now;
    logRow(now, peak);
  }
}
