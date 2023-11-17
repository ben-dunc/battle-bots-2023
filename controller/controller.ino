/*
 controller.ino
 */

#include "RF24.h"
#include "printf.h"
#include <SPI.h>

#define CE_PIN 9
#define CSN_PIN 10

#define TX_ADDRESS_NUMBER 0
#define RX_ADDRESS_NUMBER 1

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = {"1Node", "2Node"};

// float payload = 0.0;
float payload[4];

// SETUP

void setupRadio() {
    while (!radio.begin())
        Serial.println(F("radio hardware is not responding!!"));

    Serial.println(F("This is the TRANSMITTER"));
    Serial.print(F("radioNumber = "));
    Serial.println((int)TX_ADDRESS_NUMBER);

    radio.setPALevel(RF24_PA_LOW);
    radio.setPayloadSize(sizeof(payload));
    radio.openWritingPipe(address[TX_ADDRESS_NUMBER]);
    radio.openReadingPipe(1, address[!TX_ADDRESS_NUMBER]);

    radio.stopListening();
}

void setup() {
    Serial.begin(9600);
    while (!Serial)
        ;
    setupRadio();
}

// LOOP

void transmitRadio() {
    unsigned long start_timer = micros(); // start the timer
    bool report = radio.write(&payload, sizeof(payload));
    unsigned long end_timer = micros(); // end the timer

    if (report) {
        Serial.print(F("Transmission successful! ")); // payload was delivered
        Serial.print(F("Time to transmit = "));
        Serial.print(end_timer - start_timer); // print the timer result
        Serial.print(F(" us. Sent:\t"));
        Serial.print(payload[0]); // print payload sent
        Serial.print("\t");       // print payload sent
        Serial.print(payload[1]); // print payload sent
        Serial.print("\t");       // print payload sent
        Serial.print(payload[2]); // print payload sent
        Serial.print("\t");       // print payload sent
        Serial.print(payload[3]); // print payload sent
        Serial.println();         // print payload sent
    } else {
        Serial.println(F("Transmission failed or timed out")); // payload was not delivered
    }
}

void loop() {
    payload[0] += 0.01;
    payload[1] += 0.02;
    payload[2] += 0.03;
    payload[3] += 0.04;

    transmitRadio();

    delay(1000); // slow transmissions down by 1 second
}