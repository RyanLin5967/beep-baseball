/*
 * hit_counter — impact counter for the durability test rig.
 *
 * Counts impacts on the rig from an accelerometer and reports whether the ball under
 * test is still beeping (mic peak-to-peak). (The ball is a 555 + piezo with no
 * microcontroller; this runs on the rig.)
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
const unsigned long HIT_DEBOUNCE_MS = 150; // ignore ringing after one hit (hit COUNTING)
// Mic sensing only needs to skip the few-ms impact transient, NOT the full hit debounce.
// If it used HIT_DEBOUNCE_MS, a rig hitting faster than every 150ms would keep refreshing
// lastHitMs, the gate would never open, and beep failure would never be detected.
const unsigned long MIC_SETTLE_MS = 30;

const int   BEEP_FLOOR        = 40;    // mic peak-to-peak: ball ON vs OFF
// Window must span enough cycles of the beep for a reliable peak-to-peak, and we
// interleave a mic + an accel read each pass, so 4000us keeps the mic well-sampled and
// leaves no accelerometer blind spot during the window.
const unsigned long MIC_WINDOW_US = 4000;
// How long the beep must stay silent before we call it dead (reject a momentary dropout
// during impact). Time-based, so it doesn't drift with loop speed.
const unsigned long BEEP_LOST_MS = 300;
// The mic check blocks for MIC_WINDOW_US. If we ran it every loop the accelerometer
// would be sampled only once per ~2ms and would miss sub-ms impact spikes (undercount).
// So poll accel every loop and only run the mic check this often.
const unsigned long MIC_CHECK_INTERVAL_MS = 50;

unsigned long hitCount = 0;
unsigned long lastHitMs = 0;
unsigned long lastMicCheckMs = 0;
unsigned long lastBeepHeardMs = 0; // last time the beep was confirmed sounding
bool beepLost = false;
unsigned long failedAtHit = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.begin(115200);
  Serial.println(F("# millis,hit_count,accel_peak,beeping"));
  unsigned long t = millis();
  lastBeepHeardMs = t;                    // assume beeping at start; don't fail instantly
  // Place the hit/mic timers "in the past" so a hit in the first 150ms after reset still
  // counts and the mic gate is open immediately. Unsigned wrap keeps the elapsed-time
  // comparisons correct even though this underflows at t~0.
  lastHitMs      = t - (HIT_DEBOUNCE_MS + 1);
  lastMicCheckMs = t - (MIC_CHECK_INTERVAL_MS + 1);
}

// Sample mic AND accelerometer across the window so the accelerometer is never blind
// while we listen. Returns mic peak-to-peak; writes the max |accel - rest| seen during
// the window to *accelPeakOut so the caller can still catch a hit that lands here.
int sampleWindow(int *accelPeakOut) {
  int lo = 1023, hi = 0, accelPeak = 0;
  unsigned long start = micros();
  while (micros() - start < MIC_WINDOW_US) {
    int m = analogRead(PIN_MIC);
    if (m < lo) lo = m;
    if (m > hi) hi = m;
    int d = abs(analogRead(PIN_ACCEL) - ACCEL_REST);
    if (d > accelPeak) accelPeak = d;
  }
  *accelPeakOut = accelPeak;
  return hi - lo;
}

void logRow(unsigned long now, int peak) {
  Serial.print(now);      Serial.print(',');
  Serial.print(hitCount); Serial.print(',');
  Serial.print(peak);     Serial.print(',');
  Serial.println(beepLost ? 0 : 1);
}

void loop() {
  unsigned long now = millis();
  int peak = abs(analogRead(PIN_ACCEL) - ACCEL_REST);

  // Sense the beep only periodically; the window blocks ~MIC_WINDOW_US. Skip only the
  // brief MIC_SETTLE_MS impact transient, not the full hit debounce, so a fast/continuous
  // rig still gets sensing windows between hits. The window samples the accelerometer
  // too, so a hit landing during it is still caught (no blind spot).
  if (now - lastHitMs > MIC_SETTLE_MS &&
      now - lastMicCheckMs >= MIC_CHECK_INTERVAL_MS) {
    lastMicCheckMs = now;
    int windowAccelPeak = 0;
    int p2p = sampleWindow(&windowAccelPeak);
    if (windowAccelPeak > peak) peak = windowAccelPeak; // use the larger of the two
    if (p2p >= BEEP_FLOOR) {
      lastBeepHeardMs = now;
    }
    if (!beepLost && (now - lastBeepHeardMs) >= BEEP_LOST_MS) {
      beepLost = true;
      failedAtHit = hitCount;
      digitalWrite(PIN_LED, HIGH); // latch
      Serial.print(F("# FAILURE at hit "));
      Serial.println(failedAtHit);
      // Emit a machine-readable row at the failure edge too — failure usually happens
      // between hits, so a parser must see a beeping=0 data row, not just this comment.
      logRow(now, peak);
    }
  }

  // Hit detection, using the larger of the per-loop read and any in-window peak.
  if (peak > HIT_THRESHOLD && (now - lastHitMs) > HIT_DEBOUNCE_MS) {
    hitCount++;
    lastHitMs = now;
    logRow(now, peak);
  }
}
