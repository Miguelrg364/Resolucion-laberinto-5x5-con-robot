#define out 2
#define s0 3
#define s1 4
#define s2 5
#define s3 6

int countRed = 0; 
int countGreen = 0; 
int countBlue = 0;

void setup() {
  Serial.begin(9600); 

  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  // Escalado de frecuencia al 20% (S0 HIGH, S1 LOW) suele ser más estable para Arduino Uno
  digitalWrite(s0, HIGH); 
  digitalWrite(s1, LOW); 
}

void loop() {
  // --- PASO 1: LLAMAR A LA FUNCIÓN DE LECTURA ---
  readColor(); 

  // Imprimir valores para que puedas calibrar tus IFs
  Serial.print("R: "); Serial.print(countRed); 
  Serial.print(" G: "); Serial.print(countGreen); 
  Serial.print(" B: "); Serial.print(countBlue);

  // Calibrar según sensor
  if (countRed < 65 && countGreen < 250 && countBlue > 90) { 
    Serial.println(" - ROJO");
  } else if (countRed > 110 && countGreen > 300 && countBlue > 250) {
    Serial.println(" - NEGRO");
  } else if (countRed > 25 && countRed < 40 && countBlue < 50) {
    Serial.println(" - BLANCO");
  } else {
    Serial.println(" - Escaneando...");
  }

  delay(500); // Un poco más de tiempo para ver los datos
}

void readColor() {
  // Configurar para leer ROJO
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  countRed = pulseIn(out, LOW);
  delay(20);

  // Configurar para leer AZUL
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  countBlue = pulseIn(out, LOW);
  delay(20);

  // Configurar para leer VERDE
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  countGreen = pulseIn(out, LOW);
  delay(20);
}