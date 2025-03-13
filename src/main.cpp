#include "Arduino.h"

const int button_1 = D5;
const int button_2 = D6;
const int button_3 = D7;

const int led_red = D1;
const int led_yellow = D2;
const int led_green = D3;

int mode = 3;
int is_night_mode = 0;
/*
 * mode 1: red
 * mode 2: blinking yellow
 * mode 3:
 * - day mode: red -> yellow -> green -> red
 * - night mode: blinking yellow
 */
int control_source = 0; // 0: button, 1: serial

void resetLEDs();
void yDelay(int ms);

void setup() {
  Serial.begin(9600);

  pinMode(led_red, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  pinMode(led_green, OUTPUT);

  pinMode(button_1, INPUT);
  pinMode(button_2, INPUT);
  pinMode(button_3, INPUT);
}
void loop() {
  if (mode == 1) {
    digitalWrite(led_red, HIGH);
    Serial.println("R");
    yDelay(10000);
  } else if (mode == 2) {
    if (digitalRead(led_yellow) == 1) {
      digitalWrite(led_yellow, LOW);
      Serial.println("O");
      yDelay(1000);
    } else {
      digitalWrite(led_yellow, HIGH);
      Serial.println("Y");
      yDelay(1000);
    }
  } else if (mode == 3) {
    if (is_night_mode) {
      if (digitalRead(led_yellow) == 1) {
        digitalWrite(led_yellow, LOW);
        Serial.println("O");
        yDelay(1000);
      } else {
        digitalWrite(led_yellow, HIGH);
        Serial.println("Y");
        yDelay(1000);
      }
    } else {
      if (digitalRead(led_red) == 1) {
        digitalWrite(led_red, LOW);
        digitalWrite(led_yellow, HIGH);
        Serial.println("Y");
        yDelay(3000);
      } else if (digitalRead(led_yellow) == 1) {
        digitalWrite(led_yellow, LOW);
        digitalWrite(led_green, HIGH);
        Serial.println("G");
        yDelay(10000);
      } else if (digitalRead(led_green) == 1) {
        digitalWrite(led_green, LOW);
        digitalWrite(led_red, HIGH);
        Serial.println("R");
        yDelay(5000);
      } else {
        digitalWrite(led_red, HIGH);
        Serial.println("R");
        yDelay(5000);
      }
    }
  }
}

void resetLEDs() {
  digitalWrite(led_red, LOW);
  digitalWrite(led_yellow, LOW);
  digitalWrite(led_green, LOW);
}

void yDelay(int ms) {
  for (int i = 0; i <= ms; i++) {
    // serial handler
    if (Serial.available() > 0) {
      char c = Serial.read();

      if (c == 'T') {
        control_source = 1 - control_source;
        Serial.println(control_source ? "S" : "B");
      } else if (c == 'N') {
        resetLEDs();
        is_night_mode = 1;
      } else if (c == 'D') {
        resetLEDs();
        is_night_mode = 0;
      }

      if (c == '1' && control_source == 1) {
        mode = 1;
        resetLEDs();
        Serial.println("K");
        break;
      } else if (c == '2' && control_source == 1) {
        mode = 2;
        resetLEDs();
        Serial.println("Z");
        break;
      } else if (c == '3' && control_source == 1) {
        mode = 3;
        resetLEDs();
        Serial.println("E");
        break;
      }
    }

    // button handler
    if (digitalRead(button_1) == LOW && control_source == 0) {
      while (digitalRead(button_1) == LOW)
        ;
      mode = 1;
      resetLEDs();
      Serial.println("K");
      break;
    } else if (digitalRead(button_2) == LOW && control_source == 0) {
      while (digitalRead(button_2) == LOW)
        ;
      mode = 2;
      resetLEDs();
      Serial.println("Z");
      break;
    } else if (digitalRead(button_3) == LOW && control_source == 0) {
      while (digitalRead(button_3) == LOW)
        ;
      mode = 3;
      resetLEDs();
      Serial.println("E");
      break;
    }
    delay(1);
  }
}
