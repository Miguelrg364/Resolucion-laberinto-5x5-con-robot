#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// Pin LED
#define PIN_LED 8
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

String nuevaOrden = "NINGUNO";

unsigned long tiempoReferencia = 0;


void setup() {
  Serial.begin(9600);

  pixels.clear();
  pixels.begin();
  pixels.show();
}

void loop() {
  // Escuchamos Serial a la espera de órdenes
  if (Serial.available() > 0) {
    nuevaOrden = Serial.readStringUntil('\n');
    nuevaOrden.trim();
    nuevaOrden.toUpperCase();

    tiempoReferencia = millis();
  }

  if (nuevaOrden == "INICIO_PARPADEO") {
    ejecutarParpadeo();
  } 
  else {
    // Si es "NINGUNO" apagamos las luces
    apagarLuces();
  }
}


void ejecutarParpadeo() {
  unsigned long transcurrido = millis() - tiempoReferencia;
  
  if (transcurrido < 300) {
    pixels.setPixelColor(0, pixels.Color(20, 20, 50)); // Azul tenue
  } else if (transcurrido < 600) {
    pixels.setPixelColor(0, pixels.Color(20, 50, 20)); // Verde tenue
  } else if (transcurrido < 800) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));    // Apagado
  } else {
    tiempoReferencia = millis(); // Reiniciar ciclo
  }
  pixels.show();
}


void apagarLuces() {
  pixels.clear();
  pixels.show();
}

