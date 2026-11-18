#include <Braccio.h>
#include <MatrixMath.h>
#include <Servo.h>

Servo base; Servo shoulder; Servo elbow;
Servo wrist_ver; Servo wrist_rot; Servo gripper;

double L1 = 71.5; double L2 = 125; double L3 = 125;
double L4 = 60;   double L5 = 132;

// --- CAMBIO PARA MODO GRÚA ---
double qD[6];          // Ángulos objetivo (los que escribes)
double qActual[6] = {90, 90, 90, 90, 90, 73}; // Ángulos actuales (donde está el robot)

void setup() {
  Serial.begin(9600);
  Braccio.begin(); // Posición inicial vertical
  Serial.println("--- MODO GRÚA (CINEMÁTICA DIRECTA) ---");
  Serial.println("Introduce 6 ángulos: ");
}

void loop() {
  int i = 0;
  while (Serial.available() > 0 && i < 6) {
    qD[i] = Serial.parseInt(SKIP_ALL, '\n');
    qD[i] = constrain(qD[i], 0, 180);
    i++;
  }

  if (i == 6) {
    // --- LÓGICA DE MOVIMIENTO TIPO GRÚA ---
    Serial.println("Iniciando movimiento suave...");
    
    int pasos = 30; // Número de pasos intermedios
    for (int p = 1; p <= pasos; p++) {
      // Calculamos una posición intermedia para cada uno de los 6 motores
      double m1 = qActual[0] + (qD[0] - qActual[0]) * p / pasos;
      double m2 = qActual[1] + (qD[1] - qActual[1]) * p / pasos;
      double m3 = qActual[2] + (qD[2] - qActual[2]) * p / pasos;
      double m4 = qActual[3] + (qD[3] - qActual[3]) * p / pasos;
      double m5 = qActual[4] + (qD[4] - qActual[4]) * p / pasos;
      double m6 = qActual[5] + (qD[5] - qActual[5]) * p / pasos;

      // Movimiento con step delay corto (10) porque el paso es pequeño
      Braccio.ServoMovement(10, m1, m2, m3, m4, m5, m6);
    }

    // Actualizamos la posición actual para el siguiente movimiento
    for(int j=0; j<6; j++) qActual[j] = qD[j];

    // --- CÁLCULO DE COORDENADAS (Tu lógica original) ---
    double r1 = qD[0] * PI / 180.0;
    double r2 = qD[1] * PI / 180.0;
    double r3 = qD[2] * PI / 180.0;
    double r4 = qD[3] * PI / 180.0;
    double r5 = qD[4] * PI / 180.0;

    mtx_type A[4][4] = {{cos(r1), 0, sin(r1), 0}, {sin(r1), 0, -cos(r1), 0}, {0, 1, 0, L1}, {0, 0, 0, 1}};
    mtx_type B[4][4] = {{cos(r2), -sin(r2), 0, L2*cos(r2)}, {sin(r2), cos(r2), 0, L2*sin(r2)}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    mtx_type C[4][4] = {{cos(r3), -sin(r3), 0, L3*cos(r3)}, {sin(r3), cos(r3), 0, L3*sin(r3)}, {0, 0, 1, 0}, {0, 0, 0, 1}};
    mtx_type D[4][4] = {{cos(r4), 0, sin(r4), 0}, {sin(r4), 0, -cos(r4), 0}, {0, 1, 0, 0}, {0, 0, 0, 1}};
    mtx_type E[4][4] = {{cos(r5), -sin(r5), 0, 0}, {sin(r5), cos(r5), 0, 0}, {0, 0, 1, L4 + L5}, {0, 0, 0, 1}};

    mtx_type T1[4][4], T2[4][4], T3[4][4], T[4][4];
    Matrix.Multiply((mtx_type*) A, (mtx_type*) B, 4, 4, 4, (mtx_type*) T1);
    Matrix.Multiply((mtx_type*) T1, (mtx_type*) C, 4, 4, 4, (mtx_type*) T2);
    Matrix.Multiply((mtx_type*) T2, (mtx_type*) D, 4, 4, 4, (mtx_type*) T3);
    Matrix.Multiply((mtx_type*) T3, (mtx_type*) E, 4, 4, 4, (mtx_type*) T);

    Matrix.Print((mtx_type*)T, 4, 4, "T");  
  
    Serial.print("\nCOORDENADAS FINALES -> X: "); Serial.print(T[0][3]);
    Serial.print(" Y: "); Serial.print(T[1][3]);
    Serial.print(" Z: "); Serial.println(T[2][3]);
    Serial.println("Listo para nuevos angulos.");
  }
}