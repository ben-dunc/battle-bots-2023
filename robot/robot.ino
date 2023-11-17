/*
    robot.ino
 */
 
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
 
#define CE_PIN 9
#define CSN_PIN 10

#define TX_ADDRESS_NUMBER 0
#define RX_ADDRESS_NUMBER 1

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };

float payload[4];
 
// SETUP

void setupRadio() {
  while (!radio.begin())
    Serial.println(F("radio hardware is not responding!!"));
  Serial.println(F("This is the TRANSMITTER"));

  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(payload));
  radio.openWritingPipe(address[RX_ADDRESS_NUMBER]);
  radio.openReadingPipe(1, address[!RX_ADDRESS_NUMBER]);

  radio.startListening();
}

void setup() {
  Serial.begin(9600);
  setupRadio();
} 

// LOOP

void readRadioValues() {
  uint8_t pipe;
  if (radio.available(&pipe)) {              // is there a payload? get the pipe number that recieved it
    radio.read(&payload, sizeof(payload));             // fetch payload from FIFO

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
  }
}

void loop() {
  readRadioValues();
}