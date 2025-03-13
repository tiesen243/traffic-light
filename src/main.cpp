#include "Arduino.h"
#include "core_esp8266_features.h"

const int LEDS[3] = {D1, D2, D3};
const int BUTTONS[3] = {D5, D6, D7};

const int SERIAL_BAUD_RATE = 9600;
const int DURATIONS[] = {5000, 3000, 10000};
const int BLINK_INTERVAL = 1000;
const int MODE_OFFSET = 1;
const int CONTROL_SOURCE_OFFSET = 7;
const int NIGHT_MODE_OFFSET = 5;

/*
 * mode 1: red
 * mode 2: blinking yellow
 * mode 3:
 * - day mode: red -> yellow -> green -> red
 * - night mode: blinking yellow
 */
static int mode = 3;
static int is_night_mode = 0;
static int control_source = 0;

const char send_msg[9] = {'K', 'Z', 'E', 'R', 'Y', 'G', 'O', 'M', 'A'};
const char receive_msg[7] = {'1', '2', '3', 'T', 'D', 'N', 'I'};

static void ovr_delay(int delay_ms);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  for (int i = 0; i < 3; i++) {
    pinMode(LEDS[i], OUTPUT);
    pinMode(BUTTONS[i], INPUT);

    digitalWrite(LEDS[i], LOW);
  }
}

void loop() {
  if (mode == 1) {
    digitalWrite(LEDS[0], HIGH);
    Serial.println("R");
    ovr_delay(DURATIONS[0]);
  } else if (mode == 2) {
    int yellowState = digitalRead(LEDS[1]);
    digitalWrite(LEDS[1], yellowState == 0);
    Serial.println(yellowState == HIGH ? "O" : "Y");
    ovr_delay(BLINK_INTERVAL);
  } else if (mode == 3) {
    if (is_night_mode != 0) {
      int yellowState = digitalRead(LEDS[1]);
      digitalWrite(LEDS[1], yellowState == 0);
      Serial.println(yellowState == HIGH ? "O" : "Y");
      ovr_delay(BLINK_INTERVAL);
    } else {
      if (digitalRead(LEDS[0]) == HIGH) {
        digitalWrite(LEDS[0], LOW);
        digitalWrite(LEDS[1], HIGH);
        Serial.println(send_msg[4]);
        ovr_delay(DURATIONS[1]);
      } else if (digitalRead(LEDS[1]) == HIGH) {
        digitalWrite(LEDS[1], LOW);
        digitalWrite(LEDS[2], HIGH);
        Serial.println(send_msg[5]);
        ovr_delay(DURATIONS[2]);
      } else if (digitalRead(LEDS[2]) == HIGH) {
        digitalWrite(LEDS[2], LOW);
        digitalWrite(LEDS[0], HIGH);
        Serial.println(send_msg[3]);
        ovr_delay(DURATIONS[0]);
      } else {
        digitalWrite(LEDS[0], HIGH);
        Serial.println(send_msg[2]);
        ovr_delay(DURATIONS[0]);
      }
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
  Serial.println(send_msg[newMode - MODE_OFFSET]);
}

void ovr_delay(int delay_ms) {
  for (int i = 0; i <= delay_ms; i++) {
    // Handle serial input
    if (Serial.available() != 0) {
      char data = (char)Serial.read();

      if (data == receive_msg[3]) {
        control_source = 1 - control_source;
        Serial.println(send_msg[control_source + CONTROL_SOURCE_OFFSET]);
      } else if (data == receive_msg[NIGHT_MODE_OFFSET - 1]) {
        reset_LEDs();
        is_night_mode = 0;
        Serial.println(send_msg[is_night_mode + NIGHT_MODE_OFFSET]);
        if (mode == 3)
          i = delay_ms + 1;
      } else if (data == receive_msg[NIGHT_MODE_OFFSET]) {
        reset_LEDs();
        is_night_mode = 1;
        Serial.println(send_msg[is_night_mode + NIGHT_MODE_OFFSET]);
        if (mode == 3)
          i = delay_ms + 1;
      } else if (data == receive_msg[6]) {
        Serial.println(send_msg[mode - MODE_OFFSET]);
        delay(100);
        Serial.println(send_msg[control_source + CONTROL_SOURCE_OFFSET]);
        delay(100);
      } else if (control_source == 1) {
        int newMode = data - '0';
        if (newMode >= 1 && newMode <= 3 && newMode != mode) {
          switch_mode(newMode);
          i = delay_ms + 1;
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
        i = delay_ms + 1;
        break;
      }
    }

    delay(1);
  }
}
