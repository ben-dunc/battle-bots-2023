/*
 controller.ino
 */

#include "RF24.h"
#include "printf.h"
#include <SPI.h>
#include <Bounce2.h>

#define CE_PIN 9
#define CSN_PIN 10

#define TX_ADDRESS_NUMBER 0
#define RX_ADDRESS_NUMBER 1

#define DEBUG true

#define BARREL_TRIM 0
#define R_WHEEL_TRIM 2
#define L_WHEEL_TRIM 1
#define JOY_0 6
#define JOY_1 7

#define BARREL_ON 3  // BARREL_ON
#define WHEEL_INVERSE 19
#define PWR_ON 4  // POWER_ON
#define CHANNEL 18

#define RED_LED 6
#define BLUE_LED 5
#define ORANGE_LED 7
#define GREEN_LED 8

// RANGES
#define POT_MAX 340
#define JOY_MAX 540
#define JOY_MIN 120
#define JOY_RANGE 120

RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };


uint8_t payload[4];
long missTimeRef = 0;
long reportTimeRef = 0;

float barrel_trim = 0.0;   // pot0
float r_wheel_trim = 0.0;  // pot1
float l_wheel_trim = 0.0;  // pot2
float joy0 = 0.0;
float joy1 = 0.0;
bool barrel_on = false;  // btn 0
bool wheels_inverse = false;  // btn 1
bool pwr_on = false;          // swt 0
bool channel = false;         // swt 1

Bounce2::Button btn_barrel_on = Bounce2::Button();  // Instantiate a Bounce object
Bounce2::Button btn_wheel_inverse = Bounce2::Button();   // Instantiate a Bounce object

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
  radio.setRetries(0, 0);  // very small delay and no retries
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

  pinMode(BARREL_ON, INPUT);
  // pinMode(WHEEL_INVERSE, INPUT);
  pinMode(PWR_ON, INPUT);
  pinMode(CHANNEL, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  btn_barrel_on.attach(BARREL_ON, INPUT);
  btn_barrel_on.interval(20);
  btn_barrel_on.setPressedState(HIGH);
  btn_wheel_inverse.attach(WHEEL_INVERSE, INPUT);
  btn_wheel_inverse.interval(20);
  btn_wheel_inverse.setPressedState(HIGH);

  pinMode(CHANNEL, INPUT);
  long missTimeRef = millis() + 1000;
  long reportTimeRef = millis() + 250;
  
  digitalWrite(ORANGE_LED, HIGH);
  setupRadio();
  digitalWrite(ORANGE_LED, LOW);

  channel = digitalRead(CHANNEL);
  if (DEBUG) {
    Serial.print("channel: ");
    Serial.println(channel);
  }
}

void printInputs() {
  Serial.print("barrel_trim: ");
  Serial.print(barrel_trim);
  Serial.print("  r_wheel_trim: ");
  Serial.print(r_wheel_trim);
  Serial.print("  l_wheel_trim: ");
  Serial.print(l_wheel_trim);
  Serial.print("  joy0: ");
  Serial.print(joy0);
  Serial.print("  joy1: ");
  Serial.print(joy1);
  Serial.print("  pwr_on: ");
  Serial.print(pwr_on);
  Serial.print("  channel: ");
  Serial.print(channel);
  Serial.print("  barrel_on: ");
  Serial.print(barrel_on);
  Serial.print("  wheels_inverse: ");
  Serial.print(wheels_inverse);
  Serial.println("");
}

void printSignals() {
  Serial.print("on/off: ");
  Serial.print(payload[0]);
  Serial.print("\tb_speed: ");
  Serial.print(payload[1]);
  Serial.print("\tr_speed: ");
  Serial.print(payload[2]);
  Serial.print("\tl_speed: ");
  Serial.print(payload[3]);
  Serial.println();
}

void getInputs() {
  barrel_trim = analogRead(BARREL_TRIM);    // Pot 0:
  r_wheel_trim = analogRead(R_WHEEL_TRIM);  // Pot 1:
  l_wheel_trim = analogRead(L_WHEEL_TRIM);  // Pot 2:
  joy0 = analogRead(JOY_0);                 // Joy 0:
  joy1 = analogRead(JOY_1);                 // Joy 1:
  pwr_on = digitalRead(PWR_ON);             // Swt 0:

  // buttons w debounce
  btn_barrel_on.update();
  btn_wheel_inverse.update();

  if (btn_barrel_on.pressed())
    barrel_on = !barrel_on;
    digitalWrite(RED_LED, barrel_on);
  if (btn_wheel_inverse.pressed())
    wheels_inverse = !wheels_inverse;
    digitalWrite(BLUE_LED, wheels_inverse);
}

void formatSignals() {
  payload[0] = pwr_on; // on/off
  payload[1] = constrain(barrel_on ? 255 - barrel_trim : 0, 0, 255); // barrel
  
  uint8_t l_wheel_value = constrain(map(
    joy1,
    JOY_MIN, 
    JOY_MAX,
    0, 
    255
  ) - (l_wheel_trim - (POT_MAX / 2)), 0, 255);
  
  uint8_t r_wheel_value = constrain(map(
    joy0,
    JOY_MIN, 
    JOY_MAX,
    0, 
    255
  ) - (r_wheel_trim - (POT_MAX / 2)), 0, 255);

  if (wheels_inverse) {
    uint8_t swap = map(l_wheel_value, 0, 255, 255, 0);;
    l_wheel_value = map(r_wheel_value, 0, 255, 255, 0);
    r_wheel_value = swap;
  }

  payload[2] = l_wheel_value;
  payload[3] = r_wheel_value;
}

void transmitRadio() {
  unsigned long start_timer = micros();  // start the timer
  bool report = radio.writeFast(&payload, sizeof(payload));
  radio.reUseTX();
  unsigned long end_timer = micros();  // end the timer
  // radio.flush_tx();

  if (report) {
    if (reportTimeRef < millis()) {
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
      reportTimeRef = millis() + 250;
    }
    missTimeRef = millis() + 1000;
    digitalWrite(ORANGE_LED, LOW);
  } else if (missTimeRef < millis()) {
    missTimeRef = millis() + 1000;
    digitalWrite(ORANGE_LED, HIGH);
    Serial.print(F("."));
  }
}

void loop() {

  getInputs();
  // printInputs();
  formatSignals();

  transmitRadio();

  digitalWrite(GREEN_LED, pwr_on);

  // delay(1000); // slow transmissions down by 1 second
}
