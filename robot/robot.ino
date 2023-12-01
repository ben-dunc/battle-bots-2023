/*
    robot.ino
 */

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

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
long missTimeRef = 0;
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
  while (!Serial)
    ;
  printf_begin();

  pinMode(PIN_CHANNEL, INPUT);
  
  channel = digitalRead(PIN_CHANNEL);
  missTimeRef = millis();
  setupRadio();

  if (DEBUG) {
    Serial.print("channel: ");
    Serial.println(channel);
  }
}

// LOOP

void readRadioValues() {
  uint8_t pipe;
  if (radio.available(&pipe)) {             // is there a payload? get the pipe number that recieved it
    radio.read(&payload, sizeof(payload));  // fetch payload from FIFO
    radio.flush_rx();

    Serial.print(F("Received "));
    Serial.print(sizeof(payload));  // print the size of the payload
    Serial.print(F(" bytes on pipe "));
    Serial.print(pipe);  // print the pipe number
    Serial.print(F(":\t"));
    Serial.print(payload[0]);  // print the payload's value
    Serial.print("\t");        // print the payload's value
    Serial.print(payload[1]);  // print the payload's value
    Serial.print("\t");        // print the payload's value
    Serial.print(payload[2]);  // print the payload's value
    Serial.print("\t");        // print the payload's value
    Serial.print(payload[3]);  // print the payload's value
    Serial.println();          // print the payload's value
    missTimeRef = millis() + 1000;
  } else if (missTimeRef < millis()) {
    missTimeRef = millis() + 1000;
    Serial.print(".");
  }
}

void loop() {
  readRadioValues();
}