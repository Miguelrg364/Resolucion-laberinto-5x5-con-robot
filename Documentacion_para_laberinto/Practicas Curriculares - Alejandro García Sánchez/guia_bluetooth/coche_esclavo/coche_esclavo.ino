#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <Servo_megaTinyCore.h>

// Pines
#define CNY4 1
#define CNY3 2
#define CNY2 3
#define CNY1 4
#define BUZZER 5
#define LED 8
#define BPWM 9
#define AIN_ 10
#define BIN_ 11
#define ECHO 12
#define TRIG 13
#define IO 14
#define STBY 15
#define APWM 16

#define referencia_negro 800
#define referencia_blanco 300

#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);

Servo pala;

char nuevaOrden = 'I';

// Valores que puede devolver la función lectura_sensor()
#define negro 0
#define blanco 1
#define sin_superficie 2

// Identificadores numéricos para la evasión (Ahorran memoria Flash)
#define EVADIR_DELANTE 1
#define EVADIR_ATRAS   2

unsigned long tiempoReferencia = 0;
unsigned long tiempoZigZag = 0;
int estadoZigZag = 0;

int velocidadActual;
float distanciaActual;

int velocidadBase = 180;
float distanciaBase = 10;
int servoBase = 180;

int velocidadAumentada = 255;
float distanciaAumentada = 50;
int servoMax = 0;

void setup() {
  Serial.begin(9600);

  pinMode(APWM, OUTPUT);
  pinMode(AIN_, OUTPUT);
  pinMode(BIN_, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(STBY, LOW); // Dormimos al motor inicialmente (disminuimos consumo)

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(CNY4, INPUT);
  pinMode(CNY3, INPUT);
  pinMode(CNY2, INPUT);
  pinMode(CNY1, INPUT);

  pala.attach(IO);

  velocidadActual = velocidadBase;
  distanciaActual = distanciaBase;
  pala.write(servoBase);

  digitalWrite(STBY, HIGH); 
  
  tiempoZigZag = millis();
}

void loop() {
  // 1. Escuchamos el Serial y aplicamos el cambio de estado de manera síncrona
  if (Serial.available() > 0) {
    char c = Serial.read();
    c = toupper(c);

    if (c == 'R' || c == 'B' || c == 'N') {
      nuevaOrden = c;

      // Aplicamos las configuraciones de los poderes en el acto
      if (nuevaOrden == 'R') {         // MODO ROJO: Máxima velocidad
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.show();
        velocidadActual = velocidadAumentada;
      } 
      else if (nuevaOrden == 'B'){    // MODO BLANCO: Bajar Pala
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
        pixels.show();
        pala.write(servoMax);
      }
      else if (nuevaOrden == 'N'){    // MODO NEGRO / ESTADO BASE: Visión ampliada
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
        distanciaActual = distanciaAumentada;
      }
    }
    // Consumimos cualquier residuo extraño del búfer serie antes de continuar
    while(Serial.available() > 0) Serial.read();
  }

  // Si está en estado inicial 'I', nos aseguramos de que mantenga los valores seguros de carrera
  if (nuevaOrden == 'I') {
    velocidadActual = velocidadBase;
    distanciaActual = distanciaBase; // Solo busca a 10 cm, evitando falsos positivos en la mesa
    pala.write(servoBase);
  }

  // 2. La rutina merodear corre libremente usando los valores vigentes de velocidadActual y distanciaActual
  merodear();
}

// --- CONTROL DE MOTORES ---
void motores(int velA, int velB) {
  // Motor A
  if (velA >= 0) {
    digitalWrite(AIN_, HIGH);
    analogWrite(APWM, velA);
  } else {
    digitalWrite(AIN_, LOW);
    analogWrite(APWM, abs(velA));
  }
  
  // Motor B
  if (velB >= 0) {
    digitalWrite(BIN_, HIGH);
    analogWrite(BPWM, velB);
  } else {
    digitalWrite(BIN_, LOW);
    analogWrite(BPWM, abs(velB));
  }
}

// --- SENSORES CNY ---
short lectura_sensor(unsigned int sensor) {
  int valor_leido = analogRead(sensor);
  if (valor_leido <= referencia_blanco) return blanco;
  if (valor_leido >= referencia_negro) return negro;
  return sin_superficie;
}

// --- SENSOR ULTRASONIDOS ---
float obtenerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long tiempo = pulseIn(ECHO, HIGH);
  float distancia = (tiempo / 2.0) / 29.2;
  return distancia;
}

// --- FUNCIONES DE ACCIÓN ---
void perseguir() {
  motores(velocidadAumentada, velocidadAumentada); // Al frente a máxima potencia
}

void evadirLinea(int tipoPosicion) {
  if (tipoPosicion == EVADIR_DELANTE) {
    // Marcha atrás y giro brusco de 180 grados
    motores(-velocidadActual, -velocidadActual);
    delay(400);
    motores(velocidadActual, -velocidadActual);
    delay(600);
  } 
  else if (tipoPosicion == EVADIR_ATRAS) {
    // Hacia delante rápido para no salirte por detrás
    motores(velocidadActual, velocidadActual);
    delay(500);
  }
  // Resetear el estado de zigzag tras la evasión para reanudar limpio
  estadoZigZag = 0;
  tiempoZigZag = millis();
}


// --- RUTINA MERODEAR ---
void merodear() {
  // 1. Prioridad: Límites del Tatami
  if (lectura_sensor(CNY1) == blanco || lectura_sensor(CNY4) == blanco) { evadirLinea(EVADIR_DELANTE); return; }
  if (lectura_sensor(CNY2) == blanco || lectura_sensor(CNY3) == blanco) { evadirLinea(EVADIR_ATRAS); return; }

  // 2. Prioridad: Enemigo a la vista
  float distanciaEnemigo = obtenerDistancia();
  if (distanciaEnemigo > 0 && distanciaEnemigo <= distanciaActual) { 
    perseguir();
    return; 
  }

  // 3. Movimiento ZigZag Simplificado usando una oscilación en el tiempo
  // Cada 1.2 segundos el coche varía de forma fluida el giro de sus ruedas sin usar la pesada estructura switch-case
  unsigned long tiempoActual = millis() - tiempoZigZag;
  if (tiempoActual < 1200) {
    motores(velocidadActual, velocidadActual); // Va recto
  } else if (tiempoActual < 2000) {
    motores(velocidadActual * 0.3, velocidadActual); // Hace el zigzag leve hacia la derecha
  } else {
    tiempoZigZag = millis(); // Reinicia el ciclo de movimiento
  }
}
