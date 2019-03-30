int offset = 454; // zero pressure calibrate
int fullScale = 4096; // max pressure calibrate
float pressure; // final pressure
float vacuum;

void setup() {
  Serial.begin(9600);
}

void loop() {
  vacuum = (analogRead(14) - offset) * 11.6376 / (fullScale - offset);
  pressure = (analogRead(14) - offset) * 6.89476  / (fullScale - offset); // pressure conversion
  Serial.print("Pressure is  ");
  Serial.print(vacuum);
  Serial.print("  bar, ");
  Serial.print("Analogue read is: ");
  Serial.println(analogRead(14));
  delay(2000);
}
