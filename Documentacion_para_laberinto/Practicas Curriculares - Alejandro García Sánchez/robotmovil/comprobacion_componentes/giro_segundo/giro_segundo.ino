// Programa para que los motores cambien su sentido de giro cada 5 segundos

// Pines de PWM
#define M1 16
#define M2 9

// Pines de direccion
#define IN_M1 10 // 10 O 9
#define IN_M2 11 // 11 O 14

// Pin de StandBy
#define SLP_M1 15

void setup() {
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(IN_M1, OUTPUT);
  pinMode(IN_M2, OUTPUT);
  pinMode(SLP_M1, OUTPUT);

  digitalWrite(SLP_M1, LOW); // Dormimos al motor inicialmente (disminuimos consumo)
}

void loop() {
  // Movemos los motores en un sentido de giro
  digitalWrite(SLP_M1, HIGH); // Despertamos el driver
  analogWrite(M1, 150);       // Establecemos la velocidad de giro (valor entre 0-255)
  analogWrite(M2, 150);

  // Sentido de giro: HIGH en el pin de direccion de cada motor
  digitalWrite(IN_M1, HIGH);
  digitalWrite(IN_M2, HIGH);

  delay(5000); // Mantenemos el estado de los motores 5 segundos

  // Sentido de giro contrario: LOW en el pin de direccion de cada motor
  digitalWrite(IN_M1, LOW);
  digitalWrite(IN_M2, LOW);

  delay(5000); // Mantenemos el estado de los motores 5 segundos
}
