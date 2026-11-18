#include <Braccio.h>
#include <Servo.h>
#include <MatrixMath.h>

Servo base; 
Servo shoulder; 
Servo elbow; 
Servo wrist_ver; 
Servo wrist_rot; 
Servo gripper;

// Longitudes físicas del Braccio (mm)
double L1 = 71.5; 
double L2 = 125; 
double L3 = 125;
double L4 = 60; 
double L5 = 132;

int posM1 = 90, posM2 = 90, posM3 = 90, posM4 = 90, posM5 = 90, posM6 = 73;

void setup() {
  Braccio.begin(); // Movimiento inicial de seguridad (brazo vertical)
  Serial.begin(9600);
  Serial.println("--- CONTROL MANUAL CON CINEMATICA ---");
  Serial.println("Introduce angulo (10-170) o 'X' para mantener actual.");
}

void loop() {
  // Valores iniciales (vertical de seguridad)
  Serial.println("\n--- CONFIGURAR NUEVO MOVIMIENTO ---");
  
  // Pedimos los datos pasando el valor actual por si el usuario pulsa 'X'
  posM1 = pedirDato("M1 (Base)", posM1);
  posM2 = pedirDato("M2 (Hombro)", posM2);
  posM3 = pedirDato("M3 (Codo)", posM3);
  posM4 = pedirDato("M4 (Muneca V)", posM4);
  posM5 = pedirDato("M5 (Muneca R)", posM5);
  posM6 = pedirDato("M6 (Pinza)", posM6);

  Serial.println("\nEjecutando movimiento...");
  Braccio.ServoMovement(30, posM1, posM2, posM3, posM4, posM5, posM6);
  
  // Una vez movido, calculamos y mostramos la posición cartesiana
  calcularXYZ(posM1, posM2, posM3, posM4, posM5);
  
  delay(1000);
}

// Función modificada para aceptar 'X' o números
int pedirDato(String mensaje, int valorActual) {
  Serial.print(mensaje + " [Actual: " + String(valorActual) + "]: ");
  
  while (Serial.available() == 0); // Espera entrada
  
  String entrada = Serial.readStringUntil('\n');
  entrada.trim();
  entrada.toUpperCase();

  int resultado;
  if (entrada == "X") {
    Serial.println("Mantener (" + String(valorActual) + ")");
    resultado = valorActual;
  } else {
    resultado = entrada.toInt();
    // Seguridad: 10-170
    resultado = constrain(resultado, 0, 180);
    Serial.println(resultado);
  }

  // Limpiar buffer
  while(Serial.available() > 0) Serial.read();
  return resultado;
}

void calcularXYZ(double q1, double q2, double q3, double q4, double q5) {
  // Conversión a radianes
  double r1 = q1 * PI / 180.0;
  double r2 = q2 * PI / 180.0;
  double r3 = q3 * PI / 180.0;
  double r4 = q4 * PI / 180.0;
  double r5 = q5 * PI / 180.0;

  // Matrices DH
  mtx_type A[4][4] = {
    {cos(r1), 0, sin(r1), 0}, 
    {sin(r1), 0, -cos(r1), 0}, 
    {0, 1, 0, L1}, 
    {0, 0, 0, 1}
  };

  mtx_type B[4][4] = {
    {cos(r2), -sin(r2), 0, L2*cos(r2)}, 
    {sin(r2), cos(r2), 0, L2*sin(r2)}, 
    {0, 0, 1, 0}, 
    {0, 0, 0, 1}
  };

  mtx_type C[4][4] = {
    {cos(r3), -sin(r3), 0, L3*cos(r3)}, 
    {sin(r3), cos(r3), 0, L3*sin(r3)}, 
    {0, 0, 1, 0}, 
    {0, 0, 0, 1}
  };

  mtx_type D[4][4] = {
    {cos(r4), 0, sin(r4), 0}, 
    {sin(r4), 0, -cos(r4), 0}, 
    {0, 1, 0, 0}, 
    {0, 0, 0, 1}
  };

  mtx_type E[4][4] = {
    {cos(r5), -sin(r5), 0, 0}, 
    {sin(r5), cos(r5), 0, 0}, 
    {0, 0, 1, L4 + L5}, 
    {0, 0, 0, 1}
  };

  mtx_type T1[4][4], T2[4][4], T3[4][4], T[4][4];

  Matrix.Multiply((mtx_type*) A, (mtx_type*) B, 4, 4, 4, (mtx_type*) T1);
  Matrix.Multiply((mtx_type*) T1, (mtx_type*) C, 4, 4, 4, (mtx_type*) T2);
  Matrix.Multiply((mtx_type*) T2, (mtx_type*) D, 4, 4, 4, (mtx_type*) T3);
  Matrix.Multiply((mtx_type*) T3, (mtx_type*) E, 4, 4, 4, (mtx_type*) T);

  Serial.println("--- POSICION CARTESIANA DEL EFECTOR ---");
  Serial.print("X: "); Serial.print(T[0][3]); Serial.println(" mm");
  Serial.print("Y: "); Serial.print(T[1][3]); Serial.println(" mm");
  Serial.print("Z: "); Serial.print(T[2][3]); Serial.println(" mm");
}