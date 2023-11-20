// NOTE: For controller

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


# define PIN_MIN 2
# define PIN_MAX 19

bool on_flag = false;

void setup() {
  Serial.begin(9600);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(18, INPUT);
  pinMode(19, INPUT);
}

void loop() {
  Serial.print("Pot 0: "); Serial.println(analogRead(0));
  Serial.print("Pot 1: "); Serial.println(analogRead(1));
  Serial.print("Pot 2: "); Serial.println(analogRead(2));
  Serial.print("Joy 0: "); Serial.println(analogRead(6));
  Serial.print("Joy 1: "); Serial.println(analogRead(7));
  Serial.print("But 0: "); Serial.println(digitalRead(3));
  Serial.print("But 1: "); Serial.println(digitalRead(19));
  Serial.print("Swt 0: "); Serial.println(digitalRead(4));
  Serial.print("Swt 1: "); Serial.println(digitalRead(18));
  delay(1000);
  for (int i = 0; i < 25; i++)
    Serial.println();
}