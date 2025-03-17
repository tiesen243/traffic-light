#include "Arduino.h"

const int LEDS[3] = {D2, D1, D0};       // red, yellow, green
const int BUTTONS[3] = {D5, D4, D3};    // mode 1, mode 2, mode 3
const int DURATIONS[4] = {5, 3, 10, 1}; // red, yellow, green, blink

const char SEND_MSGS[11] = {'1', '2', '3', 'M', 'A', 'R',
                            'O', 'Y', 'G', 'E', 'S'};
const char RECEIVE_MSGS[5] = {'T', 'D', 'N', 'I', 'S'};
int durations[4] = {5, 3, 10, 1}; /* Red, yellow, green, blink */

/* Mode
 * - 1: Ony red led
 * - 2: Blink yellow led 1s
 * - 3:
 *   + Day mode: 5s red -> 3s yellow -> 10s green -> loop
 *   + Night mode: blink yellow led 1s
 */
int mode = 3;
int is_night_mode = 0;

/* Control mode
 * - 0: Manual mode
 * - 1: Auto mode
 */
int control_source = 0;

void ovr_delay(int delay_ms);

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
    pinMode(BUTTONS[i], INPUT);

    digitalWrite(LEDS[i], LOW);
  }
}

void loop() {
  if (mode == 1) {
    digitalWrite(LEDS[0], HIGH);
    Serial.println(SEND_MSGS[5]);
    ovr_delay(DURATIONS[0]);
  } else if (mode == 2 || (mode == 3 && is_night_mode == 1)) {
    int yellowState = digitalRead(LEDS[1]);
    digitalWrite(LEDS[1], yellowState == 0);
    Serial.println(SEND_MSGS[6 + yellowState]);
    ovr_delay(DURATIONS[3]);
  } else if (mode == 3) {
    if (digitalRead(LEDS[0]) == HIGH) {
      digitalWrite(LEDS[0], LOW);
      digitalWrite(LEDS[1], HIGH);
      Serial.println(SEND_MSGS[7]);
      ovr_delay(DURATIONS[1]);
    } else if (digitalRead(LEDS[1]) == HIGH) {
      digitalWrite(LEDS[1], LOW);
      digitalWrite(LEDS[2], HIGH);
      Serial.println(SEND_MSGS[8]);
      ovr_delay(DURATIONS[2]);
    } else {
      digitalWrite(LEDS[2], LOW);
      digitalWrite(LEDS[0], HIGH);
      Serial.println(SEND_MSGS[5]);
      ovr_delay(DURATIONS[0]);
    }
  }
}

static void reset_LEDs() {
  for (int led : LEDS)
    digitalWrite(led, LOW);
}

static void switch_mode(int newMode) {
  reset_LEDs();
  mode = newMode;
  Serial.println(SEND_MSGS[newMode - 1]);
}

void ovr_delay(int delay_ms) {
  for (int i = 0; i <= delay_ms * 10; i++) {
    // Handle serial input
    if (Serial.available() != 0) {
      char data = (char)Serial.read();

      if (data == RECEIVE_MSGS[0]) {
        // Toggle control source
        control_source = 1 - control_source;
        Serial.println(SEND_MSGS[control_source + 3]);
      } else if (data == RECEIVE_MSGS[1]) {
        // Switch to day mode
        reset_LEDs();
        is_night_mode = 0;
        if (mode == 3)
          return;
      } else if (data == RECEIVE_MSGS[2]) {
        // Switch to night mode
        reset_LEDs();
        is_night_mode = 1;
        if (mode == 3)
          return;
      } else if (data == RECEIVE_MSGS[3]) {
        // Send current mode and control source
        Serial.println(SEND_MSGS[mode - 1]);
        delay(100);
        Serial.println(SEND_MSGS[control_source + 3]);
        delay(100);
      } else if (data == RECEIVE_MSGS[4]) {
        // Receive new durations like
        // SRRYYGG:
        // S: set cmd
        // RR: red duration (2 digits)
        // YY: yellow duration (2 digits)
        // GG: green duration (2 digits)
      } else if (control_source == 1) {
        // Change mode
        int newMode = data - '0';
        if (newMode >= 1 && newMode <= 3 && newMode != mode) {
          switch_mode(newMode);
          return;
        }
      }
    }

    // Handle button input
    for (int j = 0; j < 3; j++) {
      if (digitalRead(BUTTONS[j]) == LOW && mode != j + 1 &&
          control_source == 0) {
        while (digitalRead(BUTTONS[j]) == LOW)
          ;
        switch_mode(j + 1);
        return;
      }
    }

    delay(100);
  }
}
