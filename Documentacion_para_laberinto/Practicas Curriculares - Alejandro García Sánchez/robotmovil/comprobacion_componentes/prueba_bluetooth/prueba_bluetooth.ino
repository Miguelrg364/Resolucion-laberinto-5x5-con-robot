#include <Adafruit_NeoPixel.h>

// --- PINES DE LOS MOTORES ---
#define APWM 9
#define BPWM 16
#define AIN_ 10
#define BIN_ 11
#define STBY 15

// --- PINES DE LUCES, SONIDO Y SENSORES ---
#define PIN_LED 8
#define PIN_BUZZER 5
#define TRIGGER 13
#define ECHO 12
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

// --- VARIABLE DE ESTADO ---
String estadoActual = "PARAR"; 

void setup() {
  // Configuración de motores
  pinMode(APWM, OUTPUT);
  pinMode(AIN_, OUTPUT);
  pinMode(BPWM, OUTPUT);
  pinMode(BIN_, OUTPUT);
  pinMode(STBY, OUTPUT);
  
  // Configuración de sonido y ultrasonido
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  // Inicializamos el Bluetooth
  Serial.begin(9600);
  
  // Inicializamos el LED
  pixels.begin();
  pixels.setBrightness(50);
  pixels.show(); 

  // Despertamos los motores
  digitalWrite(STBY, HIGH); 
}

void loop() {
  // 1. LEER NUEVAS ÓRDENES DEL MÓVIL
  if (Serial.available() > 0) {
    // Si entra cualquier dato, el LED se pone BLANCO un momento
    pixels.setPixelColor(0, pixels.Color(255, 255, 255)); 
    pixels.show();
    delay(100);

    String nuevaOrden = Serial.readStringUntil('\n');
    nuevaOrden.trim();
    nuevaOrden.toUpperCase();

    Serial.println("He recibido: " + nuevaOrden);

    // CHIVATO: Si recibimos basura, parpadea en ROJO
    // Si recibimos texto legible, se pone VERDE
    if (nuevaOrden.length() > 0) {
        if (isAlpha(nuevaOrden[0])) { // ¿Es la primera letra una letra real?
             pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // VERDE
        } else {
             pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // ROJO (basura)
        }
        pixels.show();
    }
    
    if (nuevaOrden != "") {
      estadoActual = nuevaOrden;
      detenerTodo(); // Pausa limpia antes de cambiar de estado
    }
  }

  // 2. LEER EL SENSOR DE ULTRASONIDO
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
  // Usamos un timeout (30000) para que si no detecta nada el robot no se quede "congelado"
  long tiempo = pulseIn(ECHO, HIGH, 30000); 
  float distancia = (tiempo / 2.0) / 29.2;

  // 3. LÓGICA DE EJECUCIÓN (Máquina de estados + Alerta)
  
  // Si hay un obstáculo válido a 10 cm o menos...
  if (distancia > 0 && distancia <= 10) {
    // Paramos los motores y el zumbador por seguridad
    controlarMotores(0, 0);
    noTone(PIN_BUZZER);
    
    // Parpadeo Amarillo y Rosa
    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Amarillo
    pixels.show();
    delay(150);
    
    pixels.setPixelColor(0, pixels.Color(255, 0, 255)); // Rosa/Magenta
    pixels.show();
    delay(150);
  } 
  // Si el camino está libre (> 10 cm), hacemos caso a la orden del Bluetooth
  else {
    if (estadoActual == "GIRAR") {
      controlarMotores(120, -120); 
    } 
    else if (estadoActual == "ROJO") {
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
    } 
    else if (estadoActual == "AZUL") {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255));
      pixels.show();
    }
    else if (estadoActual == "PITO") {
      tone(PIN_BUZZER, 1000);
      delay(200);
      noTone(PIN_BUZZER);
      delay(200);
    } 
    else if (estadoActual == "PARAR") {
      detenerTodo();
    }
    else {
      estadoActual = "PARAR";
    }
  }
}

// --- FUNCIONES AUXILIARES ---

void controlarMotores(int velIzq, int velDer) {
  analogWrite(APWM, abs(velIzq));
  digitalWrite(AIN_, velIzq >= 0 ? HIGH : LOW);
  
  analogWrite(BPWM, abs(velDer));
  digitalWrite(BIN_, velDer >= 0 ? HIGH : LOW);
}

void detenerTodo() {
  controlarMotores(0, 0);
  pixels.clear();
  pixels.show();
  noTone(PIN_BUZZER);
}