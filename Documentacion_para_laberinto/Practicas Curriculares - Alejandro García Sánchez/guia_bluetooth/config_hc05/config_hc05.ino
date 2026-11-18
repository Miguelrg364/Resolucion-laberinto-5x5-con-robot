#include <SoftwareSerial.h>

// Pines para el Arduino del Brazo
#define RX A2
#define TX A3 

SoftwareSerial blueSerial(RX, TX);

void setup()
{
  Serial.begin(9600);
  blueSerial.begin(38400);
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