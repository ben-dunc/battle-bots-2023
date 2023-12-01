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

#define DEBUG true

#define BARREL_TRIM 0
#define R_WHEEL_TRIM 1
#define L_WHEEL_TRIM 2
#define JOY_0 6
#define JOY_1 7

#define BARREL_ON 3  // BARREL_ON
#define WHEEL_INVERSE 19
#define PWR_ON 4  // POWER_ON
#define CHANNEL 18

#define RED_LED 6
#define ORANGE_LED 7

// RANGES
#define POT_MAX 340
#define JOY_MAX 540
#define JOY_MIN 120
#define JOY_RANGE 120

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
  Serial.print(F("radioNumber = "));
  Serial.println((int)TX_ADDRESS_NUMBER);

  if (channel)
    radio.setChannel(0);
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(payload));
  radio.openWritingPipe(address[TX_ADDRESS_NUMBER]);
  radio.openReadingPipe(1, address[!TX_ADDRESS_NUMBER]);

  radio.stopListening();

  radio.printPrettyDetails();

  radio.flush_tx();
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  printf_begin();

  pinMode(CHANNEL, INPUT);
  long missTimeRef = millis() + 1000;
  setupRadio();
  
  channel = digitalRead(CHANNEL);
  if (DEBUG) {
    Serial.print("channel: ");
    Serial.println(channel);
  }
}

// LOOP

void transmitRadio() {
  unsigned long start_timer = micros();  // start the timer
  bool report = radio.writeFast(&payload, sizeof(payload));
  radio.reUseTX();
  unsigned long end_timer = micros();  // end the timer
  // radio.flush_tx();

  if (report) {
    Serial.print(F("Transmission successful! "));  // payload was delivered
    Serial.print(F("Time to transmit = "));
    Serial.print(end_timer - start_timer);  // print the timer result
    Serial.print(F(" us. Sent:\t"));
    Serial.print(payload[0]);  // print payload sent
    Serial.print("\t");        // print payload sent
    Serial.print(payload[1]);  // print payload sent
    Serial.print("\t");        // print payload sent
    Serial.print(payload[2]);  // print payload sent
    Serial.print("\t");        // print payload sent
    Serial.print(payload[3]);  // print payload sent
    Serial.println();          // print payload sent
    missTimeRef = millis() + 1000;
  } else if (missTimeRef < millis()) {
    missTimeRef = millis() + 1000;
    Serial.print(F("."));
  }
}

void loop() {
  payload[0] += 1;
  payload[1] += 2;
  payload[2] -= 1;
  payload[3] -= 2;

  transmitRadio();

  // delay(1000); // slow transmissions down by 1 second
}
