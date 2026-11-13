#include <SoftwareSerial.h>

// Pines para el Arduino del Brazo
#define RX A2
#define TX A3 

SoftwareSerial blueSerial(RX, TX);

void setup()
{
  Serial.begin(9600);
  blueSerial.begin(9600); // Posibilidad de usar 38400 para usar comandos AT en caso de no funcionar con 9600
  Serial.println("Listo para comandos AT");
}

void loop()
{
  if (blueSerial.available())
  {
    Serial.write(blueSerial.read());
  }
  if (Serial.available())
  {
    blueSerial.write(Serial.read());
  }
}