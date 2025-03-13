#include "Arduino.h"

const int LEDS[3] = {D1, D2, D3};       // red, yellow, green
const int BUTTONS[3] = {D5, D6, D7};    // mode 1, mode 2, mode 3
const int DURATIONS[4] = {5, 3, 10, 1}; // red, yellow, green, blink
const char SEND_MSG[9] = {'K', 'Z', 'E', 'R', 'Y', 'G', 'O', 'M', 'A'};
const char RECEIVE_MSG[7] = {'1', '2', '3', 'T', 'D', 'N', 'I'};

/*
 * mode 1: red
 * mode 2: blinking yellow
 * mode 3:
 * - day mode: red -> yellow -> green -> red
 * - night mode: blinking yellow
 */
int mode = 3;
int is_night_mode = 0;
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
    Serial.println("R");
    ovr_delay(DURATIONS[0]);
  } else if (mode == 2) {
    int yellowState = digitalRead(LEDS[1]);
    digitalWrite(LEDS[1], yellowState == 0);
    Serial.println(yellowState == HIGH ? "O" : "Y");
    ovr_delay(DURATIONS[3]);
  } else if (mode == 3) {
    if (is_night_mode != 0) {
      int yellowState = digitalRead(LEDS[1]);
      digitalWrite(LEDS[1], yellowState == 0);
      Serial.println(yellowState == HIGH ? "O" : "Y");
      ovr_delay(DURATIONS[3]);
    } else {
      if (digitalRead(LEDS[0]) == HIGH) {
        digitalWrite(LEDS[0], LOW);
        digitalWrite(LEDS[1], HIGH);
        Serial.println("Y");
        ovr_delay(DURATIONS[1]);
      } else if (digitalRead(LEDS[1]) == HIGH) {
        digitalWrite(LEDS[1], LOW);
        digitalWrite(LEDS[2], HIGH);
        Serial.println("G");
        ovr_delay(DURATIONS[2]);
      } else {
        digitalWrite(LEDS[2], LOW);
        digitalWrite(LEDS[0], HIGH);
        Serial.println("R");
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
  Serial.println(SEND_MSG[newMode - 1]);
}

void ovr_delay(int delay_ms) {
  for (int i = 0; i <= delay_ms * 10; i++) {
    // Handle serial input
    if (Serial.available() != 0) {
      char data = (char)Serial.read();

      if (data == RECEIVE_MSG[3]) {
        control_source = 1 - control_source;
        Serial.println(SEND_MSG[control_source + 7]);
      } else if (data == RECEIVE_MSG[4]) {
        reset_LEDs();
        is_night_mode = 0;
        if (mode == 3)
          i = delay_ms + 1;
      } else if (data == RECEIVE_MSG[5]) {
        reset_LEDs();
        is_night_mode = 1;
        if (mode == 3)
          i = delay_ms + 1;
      } else if (data == RECEIVE_MSG[6]) {
        Serial.println(SEND_MSG[mode - 1]);
        delay(100);
        Serial.println(SEND_MSG[control_source + 7]);
        delay(100);
      } else if (control_source == 1) {
        int newMode = data - '0';
        if (newMode >= 1 && newMode <= 3 && newMode != mode) {
          switch_mode(newMode);
          i = delay_ms * 10 + 1;
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
        i = delay_ms * 10 + 1;
        break;
      }
    }

    delay(100);
  }
}
