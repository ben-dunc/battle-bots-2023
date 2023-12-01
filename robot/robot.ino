/*
    robot.ino
 */
 
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <Servo.h>

#define TX_ADDRESS_NUMBER 0
#define RX_ADDRESS_NUMBER 1

#define FULL 255
#define HALF 127

#define DEBUG true

// pins
#define CE_PIN 9
#define CSN_PIN 10

#define PIN_LED_CON 2
#define PIN_LED_CHANNEL 4
#define PIN_ESC_PWM 6
#define PIN_R_PWM 3
#define PIN_L_PWM 5
#define PIN_IN_1 14
#define PIN_IN_2 15
#define PIN_IN_3 16
#define PIN_IN_4 17

#define PIN_CHANNEL 8

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };

uint8_t payload[4];

uint8_t pwr_on_read = 0;
uint8_t barrel_read = 0;
uint8_t l_wheel_read = 0;
uint8_t r_wheel_read = 0;

Servo ESC;

long missTimeRef = 0;
long reportTimeRef = 0;
bool channel = false;

 
// SETUP

void setupRadio() {
  while (!radio.begin())
    Serial.println(F("radio hardware is not responding!!"));
  Serial.println(F("This is the TRANSMITTER"));

  if (channel)
    radio.setChannel(0);
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(payload));
  radio.openWritingPipe(address[RX_ADDRESS_NUMBER]);
  radio.openReadingPipe(1, address[!RX_ADDRESS_NUMBER]);

  radio.startListening();
  radio.printPrettyDetails();
  radio.flush_rx();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  printf_begin();

  // initialize pins
  pinMode(PIN_LED_CON, OUTPUT);
  pinMode(PIN_LED_CHANNEL, OUTPUT);
  pinMode(PIN_ESC_PWM, OUTPUT);
  pinMode(PIN_R_PWM, OUTPUT);
  pinMode(PIN_L_PWM, OUTPUT);
  pinMode(PIN_IN_1, OUTPUT);
  pinMode(PIN_IN_2, OUTPUT);
  pinMode(PIN_IN_3, OUTPUT);
  pinMode(PIN_IN_4, OUTPUT);

  ESC.attach(PIN_ESC_PWM, 1000, 2000);

  pinMode(PIN_CHANNEL, INPUT);
  channel = digitalRead(PIN_CHANNEL);
  if (DEBUG) {
    Serial.print("channel: ");
    Serial.println(channel);
  }

  missTimeRef = millis();
  reportTimeRef = millis();

  digitalWrite(PIN_LED_CHANNEL, channel ? HIGH : LOW);

  setupRadio();
} 

void applySignals() {
  if (pwr_on_read) {
    // barrel esc
    ESC.write(map(barrel_read, 0, 255, 0, 180));

    // l_wheel
    analogWrite(PIN_L_PWM, map(abs(l_wheel_read - HALF), 0, HALF, 0, FULL));
    if (l_wheel_read < HALF - 5) { // backwards
      digitalWrite(PIN_IN_1, LOW);
      digitalWrite(PIN_IN_2, HIGH);
    } else if (l_wheel_read > HALF + 5) { // forwards
      digitalWrite(PIN_IN_1, HIGH);
      digitalWrite(PIN_IN_2, LOW);
    } else { // off
      digitalWrite(PIN_IN_1, LOW);
      digitalWrite(PIN_IN_2, LOW);
    }

    // r_wheel - NOTE: This one is reversed because the motor is mirrored compared to the left one
    analogWrite(PIN_R_PWM, map(abs(r_wheel_read - HALF), 0, HALF, 0, FULL));
    if (r_wheel_read < HALF - 5) { // backwards
      digitalWrite(PIN_IN_3, HIGH);
      digitalWrite(PIN_IN_4, LOW);
    } else if (r_wheel_read > HALF + 5) { // forwards
      digitalWrite(PIN_IN_3, LOW);
      digitalWrite(PIN_IN_4, HIGH);
    } else { // off
      digitalWrite(PIN_IN_3, LOW);
      digitalWrite(PIN_IN_4, LOW);
    }
    
  } else {
    ESC.write(0);

    // shut everything down
    analogWrite(PIN_L_PWM, 0);
    digitalWrite(PIN_IN_1, LOW);
    digitalWrite(PIN_IN_2, LOW);
    
    analogWrite(PIN_R_PWM, 0);
    digitalWrite(PIN_IN_3, LOW);
    digitalWrite(PIN_IN_4, LOW);
  }
}

void readRadioValues() {
  uint8_t pipe;
  if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
    radio.read(&payload, sizeof(payload));             // fetch payload from FIFO
    radio.flush_rx();

    if (reportTimeRef < millis()) {
      Serial.print(F("Received "));
      Serial.print(sizeof(payload));  // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      Serial.print(pipe);  // print the pipe number
      Serial.print(F(":\t"));
      Serial.print(payload[0]);  // print the payload's value
      Serial.print("\t");  // print the payload's value
      Serial.print(payload[1]);  // print the payload's value
      Serial.print("\t");  // print the payload's value
      Serial.print(payload[2]);  // print the payload's value
      Serial.print("\t");  // print the payload's value
      Serial.print(payload[3]);  // print the payload's value
      Serial.println();  // print the payload's value
      reportTimeRef = millis() + 0;
      digitalWrite(PIN_LED_CON, LOW);
    }
  } else if (missTimeRef < millis()) {
    missTimeRef = millis() + 3000;
    digitalWrite(PIN_LED_CON, HIGH);
    Serial.print(".");
  }
}

void loop() {
  readRadioValues();
  applySignals();
}