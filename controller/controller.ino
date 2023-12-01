/*
 controller.ino
 */

/*
  INPUT RANGE NOTES:
  Button 0 (pin 3) is right button
  Button 1 (pin 19) is left button
  Switch 0 (pin 4) is right switch
  Switch 1 (pin 18) is left switch
  Joy 0 (pin A6) is right joystick - range: 128 - 336 - 524
  Joy 1 (pin A7) is left joystick - range: 120 - 327 - 530
  Pot 0 (pin A0) top-most - range: 0 - 335
  Pot 1 (pin A1) left-most - range: 0 - 326
  Pot 2 (pin A2) bottom-most - range: 0 - 346
*/

// TODO: barrel needs to toggle

#include "RF24.h"
#include "printf.h"
#include <SPI.h>
#include <Bounce2.h>

#define DEBUG true

// PINS
#define CE_PIN 9
#define CSN_PIN 10

#define TX_ADDRESS_NUMBER 0
#define RX_ADDRESS_NUMBER 1

#define BARREL_TRIM 0
#define R_WHEEL_TRIM 1
#define L_WHEEL_TRIM 2
#define JOY_0 6
#define JOY_1 7

#define BARREL_ON 3 // BARREL_ON
#define WHEEL_INVERSE 19
#define PWR_ON 4 // POWER_ON
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

/*
  payload[0] = on/off
  payload[1] = pin motor
  payload[2] = r wheel motor
  payload[3] = l wheel motor
*/
float payload[4];
float barrel_trim = 0.0;   // pot0
float r_wheel_trim = 0.0;  // pot1
float l_wheel_trim = 0.0;  // pot2
float joy0 = 0.0;
float joy1 = 0.0;
bool barrel_on = false;  // btn 0
bool wheels_inverse = false;  // btn 1
bool pwr_on = false;          // swt 0
bool channel = false;         // swt 1

Bounce btn_barrel_on = Bounce();  // Instantiate a Bounce object
Bounce btn_wheel_inverse = Bounce();   // Instantiate a Bounce object


// SETUP

void setupRadio() {
  while (!radio.begin())
    Serial.println(F("radio hardware is not responding!!"));

  Serial.println(F("This is the TRANSMITTER"));
  Serial.print(F("radioNumber = "));
  Serial.println((int) TX_ADDRESS_NUMBER);

  // TODO: Change freq depending on "channel"

  // if (channel)
  //   radio.setChannel(152);

  radio.setPALevel(RF24_PA_LOW); // TODO: CHANGE to high power!
  radio.setPayloadSize(sizeof(payload));
  radio.openWritingPipe(address[TX_ADDRESS_NUMBER]);
  radio.openReadingPipe(1, address[!TX_ADDRESS_NUMBER]);

  radio.stopListening();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(BARREL_ON, INPUT);
  pinMode(WHEEL_INVERSE, INPUT);
  pinMode(PWR_ON, INPUT);
  pinMode(CHANNEL, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(ORANGE_LED, OUTPUT);
  btn_barrel_on.attach(BARREL_ON, HIGH);
  btn_barrel_on.interval(50);
  btn_wheel_inverse.attach(WHEEL_INVERSE, HIGH);
  btn_wheel_inverse.interval(50);

  channel = digitalRead(CHANNEL);

  Serial.println("Serial connected");

  // if (!DEBUG) {
    digitalWrite(RED_LED, HIGH);
    setupRadio();
    digitalWrite(RED_LED, LOW);
  // }
}

void printInputs() {
  Serial.print("barrel_trim: ");
  Serial.println(barrel_trim);
  Serial.print("r_wheel_trim: ");
  Serial.println(r_wheel_trim);
  Serial.print("l_wheel_trim: ");
  Serial.println(l_wheel_trim);
  Serial.print("joy0: ");
  Serial.println(joy0);
  Serial.print("joy1: ");
  Serial.println(joy1);
  Serial.print("pwr_on: ");
  Serial.println(pwr_on);
  Serial.print("channel: ");
  Serial.println(channel);
  Serial.print("barrel_on: ");
  Serial.println(barrel_on);
  Serial.print("wheels_inverse: ");
  Serial.println(wheels_inverse);
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
  barrel_trim = analogRead(BARREL_TRIM);   // Pot 0:
  r_wheel_trim = analogRead(R_WHEEL_TRIM);  // Pot 1:
  l_wheel_trim = analogRead(L_WHEEL_TRIM);  // Pot 2:
  joy0 = analogRead(JOY_0);          // Joy 0:
  joy1 = analogRead(JOY_1);          // Joy 1:
  pwr_on = digitalRead(PWR_ON);       // Swt 0:

  // buttons w debounce
  btn_barrel_on.update();
  btn_wheel_inverse.update();

  if (btn_barrel_on.changed())
    barrel_on = !barrel_on;
  if (btn_wheel_inverse.changed())
    wheels_inverse = !wheels_inverse;
}

void formatSignals() {
  payload[0] = pwr_on; // on/off
  payload[1] = constrain(barrel_on ? 255 - barrel_trim : 0, 0, 255); // barrel
  
  payload[2] = map(
    joy1 + (l_wheel_trim - (POT_MAX / 2)) / 4, 
    JOY_MIN, 
    JOY_MAX + POT_MAX / 8,
    0, 
    255
  ); // l_wheel
  
  payload[3] = map(
    joy0 + (r_wheel_trim - (POT_MAX / 2)) / 4, 
    JOY_MIN, 
    JOY_MAX + POT_MAX / 8,
    0, 
    255
  ); // r_wheel
}

void transmitRadio() {
  unsigned long start_timer = micros();  // start the timer
  bool report = radio.write(&payload, sizeof(payload));
  unsigned long end_timer = micros();  // end the timer

  if (DEBUG) {
    if (report) {
      Serial.print(F("Transmission successful! "));  // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);  // print the timer result
      Serial.print(F(" us. Sent:\t"));

    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }
  }
  if (report)
    digitalWrite(ORANGE_LED, HIGH);
  else
    digitalWrite(ORANGE_LED, LOW);
}

void loop() {
  getInputs();
  formatSignals();
  // if (!DEBUG)
  transmitRadio();

  if (DEBUG) {
    printInputs();
    printSignals();
  }
  delay(1000);  // slow transmissions down by 1 second
}
