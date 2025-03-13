#include "Arduino.h"

const int LED_RED = D1;
const int LED_YELLOW = D2;
const int LED_GREEN = D3;

const int BUTTON_1 = D5;
const int BUTTON_2 = D6;
const int BUTTON_3 = D7;

const int SERIAL_BAUD_RATE = 9600;
const int RED_DURATION = 5000;
const int YELLOW_DURATION = 3000;
const int GREEN_DURATION = 10000;
const int BLINK_INTERVAL = 1000;

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

const char send_msg[9] = {'K', 'Z', 'E', 'R', 'Y', 'G', 'O', 'A', 'M'};
const char receive_msg[6] = {'1', '2', '3', 'T', 'D', 'N'};

static void ovr_delay(int delay_ms);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
}

void loop() {
  if (mode == 1) {
    digitalWrite(LED_RED, HIGH);
    Serial.println("R");
    ovr_delay(RED_DURATION);
  } else if (mode == 2) {
    if (digitalRead(LED_YELLOW) == HIGH) {
      digitalWrite(LED_YELLOW, LOW);
      Serial.println("O");
      ovr_delay(BLINK_INTERVAL);
    } else {
      digitalWrite(LED_YELLOW, HIGH);
      Serial.println("Y");
      ovr_delay(BLINK_INTERVAL);
    }
  } else if (mode == 3) {
    if (is_night_mode != 0) {
      if (digitalRead(LED_YELLOW) == HIGH) {
        digitalWrite(LED_YELLOW, LOW);
        Serial.println("O");
        ovr_delay(BLINK_INTERVAL);
      } else {
        digitalWrite(LED_YELLOW, HIGH);
        Serial.println("Y");
        ovr_delay(BLINK_INTERVAL);
      }
    } else {
      if (digitalRead(LED_RED) == HIGH) {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        Serial.println("Y");
        ovr_delay(YELLOW_DURATION);
      } else if (digitalRead(LED_YELLOW) == HIGH) {
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_GREEN, HIGH);
        Serial.println("G");
        ovr_delay(GREEN_DURATION);
      } else if (digitalRead(LED_GREEN) == HIGH) {
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_RED, HIGH);
        Serial.println("R");
        ovr_delay(RED_DURATION);
      } else {
        digitalWrite(LED_RED, HIGH);
        Serial.println("R");
        ovr_delay(RED_DURATION);
      }
    }
  }
}

static void reset_LEDs() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
}

static void switch_mode(int newMode) {
  reset_LEDs();
  mode = newMode;
  Serial.println(send_msg[mode - 1]);
}

void ovr_delay(int delay_ms) {
  for (int i = 0; i <= delay_ms; i++) {

    if (Serial.available() != 0) {
      char data = (char)Serial.read();

      if (data == receive_msg[3]) {
        control_source = 1 - control_source;
        Serial.println(send_msg[control_source + 7]);
      } else if (data == receive_msg[4]) {
        is_night_mode = 0;
        Serial.println(send_msg[is_night_mode + 5]);
      } else if (data == receive_msg[5]) {
        is_night_mode = 1;
        Serial.println(send_msg[is_night_mode + 5]);
      } else if (control_source == 1) {
        int newMode = data - '0';
        if (newMode >= 1 && newMode <= 3) {
          switch_mode(newMode);
          break;
        }
      }
    }

    if (digitalRead(BUTTON_1) == LOW && control_source == 0) {
      while (digitalRead(BUTTON_1) == LOW)
        ;
      switch_mode(1);
      break;
    } else if (digitalRead(BUTTON_2) == LOW && control_source == 0) {
      while (digitalRead(BUTTON_2) == LOW)
        ;
      switch_mode(2);
      break;
    } else if (digitalRead(BUTTON_3) == LOW && control_source == 0) {
      while (digitalRead(BUTTON_3) == LOW)
        ;
      switch_mode(3);
      break;
    }

    delay(1);
  }
}
