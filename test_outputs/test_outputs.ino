# define PIN_MIN 2
# define PIN_MAX 19

bool on_flag = false;

void setup() {
  for (int i = PIN_MIN; i <= PIN_MAX; i++)
    pinMode(i, OUTPUT);
}

void loop() {
  for (int i = PIN_MIN; i <= PIN_MAX; i++)
    digitalWrite(i, on_flag);
  on_flag = !on_flag;
  delay(1000);
}
