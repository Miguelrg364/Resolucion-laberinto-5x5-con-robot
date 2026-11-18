#include <Adafruit_NeoPixel.h>

// Según tu esquemático, el LED está en el PIN 8
#define PIN_LED 8    
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  // El ATtiny816 usa internamente PB2 y PB3 para Serial.
  // En tu placa están etiquetados como 6 y 7.
  Serial.begin(9600); 
  
  pixels.begin();
  pixels.setBrightness(50); // Un poco de brillo para no saturar
  pixels.show();

  // Color AZUL al arrancar (Confirmación de energía)
  mostrarColor(0, 0, 255);
  delay(1000);
  mostrarColor(0, 0, 0);
}

void loop() {
  // Si llega algo por el Bluetooth (Pines 6/7)...
  if (Serial.available() > 0) {
    char datoRecibido = Serial.read();

    // Enviamos el eco de vuelta al móvil
    Serial.print("Recibido: ");
    Serial.println(datoRecibido);

    // Parpadeo VERDE (Confirmación de recepción)
    mostrarColor(0, 255, 0);
    delay(100);
    mostrarColor(0, 0, 0);
  }
}

void mostrarColor(int r, int g, int b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}