#include <Braccio.h>
#include <Servo.h>

Servo base, shoulder, elbow, wrist_ver, wrist_rot, gripper;

// Longitudes físicas del Braccio
const float L1 = 71.5; 
const float L2 = 125.0;
const float L3 = 125.0;
const float L4_5 = 192.0;

// Posiciones iniciales (Brazo estirado hacia arriba)
int m1 = 90; // Base
int m2 = 90; // Hombro
int m3 = 90; // Codo
int m4 = 90; // Muñeca Vertical
int m5 = 90; // Muñeca Rotatoria
int m6 = 73; // Pinza (Abierta)

int paso = 5; // Grados que se mueve con cada pulsación

void setup() {
  Braccio.begin();
  Serial.begin(9600);
  
  Serial.println("==========================================");
  Serial.println("  CONTROL REMOTO + LECTOR DE COORDENADAS  ");
  Serial.println("==========================================");
  Serial.println("Teclas de control (+5 grados / -5 grados):");
  Serial.println("  Base:       'Q' (Izda)  / 'A' (Dcha)");
  Serial.println("  Hombro:     'W' (Sube)  / 'S' (Baja)");
  Serial.println("  Codo:       'E' (Sube)  / 'D' (Baja)");
  Serial.println("  Muneca V:   'R' (Sube)  / 'F' (Baja)");
  Serial.println("  Muneca Rot: 'T' (Gira)  / 'G' (Gira)");
  Serial.println("  Pinza:      'Y' (Abre)  / 'H' (Cierra)");
  Serial.println("==========================================\n");
  
  // Imprimir posición inicial
  imprimirEstado();
}

void loop() {
  if (Serial.available() > 0) {
    char tecla = Serial.read();
    
    if (tecla == '\n' || tecla == '\r') return;
    tecla = toupper(tecla); 

    // Lógica de movimiento
    if (tecla == 'Q') m1 += paso;
    if (tecla == 'A') m1 -= paso;
    if (tecla == 'W') m2 += paso;
    if (tecla == 'S') m2 -= paso;
    if (tecla == 'E') m3 += paso;
    if (tecla == 'D') m3 -= paso;
    if (tecla == 'R') m4 -= paso;
    if (tecla == 'F') m4 += paso;
    if (tecla == 'T') m5 += paso;
    if (tecla == 'G') m5 -= paso;
    if (tecla == 'Y') m6 -= paso;
    if (tecla == 'H') m6 += paso;

    // Protecciones mecánicas para evitar roturas
    m1 = constrain(m1, 0, 180);
    m2 = constrain(m2, 0, 180); 
    m3 = constrain(m3, 0, 180);
    m4 = constrain(m4, 0, 180);
    m5 = constrain(m5, 0, 180);
    m6 = constrain(m6, 0, 180);  

    // Mover el robot físicamente
    Braccio.ServoMovement(10, m1, m2, m3, m4, m5, m6);

    // Calcular e imprimir todo
    imprimirEstado();
  }
}

// Función que calcula y muestra ángulos y coordenadas
void imprimirEstado() {
  Serial.println("\n------------------------------------------");
  
  // 1. Mostrar Ángulos
  Serial.print("ANGULOS -> M1:"); Serial.print(m1);
  Serial.print(" M2:"); Serial.print(m2);
  Serial.print(" M3:"); Serial.print(m3);
  Serial.print(" M4:"); Serial.print(m4);
  Serial.print(" M5:"); Serial.print(m5);
  Serial.print(" Pinza:"); Serial.println(m6);

  // 2. Calcular Cinemática Directa sincronizada con InverseK
  // Revertimos el mapeo de ángulos a los radianes puros de la librería
  float a0 = m1 * PI / 180.0;
  float a1 = (180.0 - m2) * PI / 180.0;
  float a2 = (m3 - 90.0) * PI / 180.0;
  float a3 = (m4 - 90.0) * PI / 180.0;

  // Calculamos el "alcance horizontal" (radio) y la altura relativa al hombro
  float r = L2 * cos(a1) + L3 * cos(a1 + a2) + L4_5 * cos(a1 + a2 + a3);
  float z_hombro = L2 * sin(a1) + L3 * sin(a1 + a2) + L4_5 * sin(a1 + a2 + a3);

  // Convertimos a Coordenadas Cartesianas 3D
  float x = r * cos(a0);
  float y = r * sin(a0);
  float z = z_hombro + L1; // Sumamos L1 porque la mesa es el nivel Z=0

  // 3. Mostrar Coordenadas X, Y, Z
  Serial.print("COORDENADAS (X, Y, Z) -> ");
  Serial.print("X: "); Serial.print(x, 2);
  Serial.print("  |  Y: "); Serial.print(y, 2);
  Serial.print("  |  Z: "); Serial.print(z, 2);
  Serial.println(" mm");
}