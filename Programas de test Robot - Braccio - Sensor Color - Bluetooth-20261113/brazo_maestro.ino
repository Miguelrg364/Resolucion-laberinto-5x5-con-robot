#include <SoftwareSerial.h>

#define RX A2
#define TX A3

// Configuración Bluetooth
SoftwareSerial btSerial(RX, TX);

String orden = "";

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
}

void loop() {
  // Envía orden de parpadear el LED durante 5 segundos
  orden = "INICIO_PARPADEO";
  Serial.println("ENVIANDO ORDEN DE PARPADEO...");
  btSerial.println(orden);
  delay(5000);

  // Apagará el LED durante 2 segundos
  orden = "NINGUNO";
  Serial.println("ENVIANDO ORDEN DE PARAR...");
  btSerial.println(orden);
  delay(2000);
}
