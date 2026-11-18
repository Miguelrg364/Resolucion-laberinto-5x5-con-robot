// Programa que hace 9 tonos de sonido mediante el buzzer

#define BUZZER 5

void setup() {
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  tone(BUZZER, 100);
  delay(200);
  noTone(BUZZER);

  tone(BUZZER, 300);
  delay(200);
  noTone(BUZZER);

  tone(BUZZER, 500);
  delay(200);
  noTone(BUZZER);

  tone(BUZZER, 700);
  delay(200);
  noTone(BUZZER);

  tone(BUZZER, 900);
  delay(200);
  noTone(BUZZER);
}

