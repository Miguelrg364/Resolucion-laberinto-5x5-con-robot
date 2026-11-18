#include <Braccio.h>
#include <Servo.h>

Servo base; Servo shoulder; Servo elbow; 
Servo wrist_ver; Servo wrist_rot; Servo gripper;

void setup() {
  // Inicialización de seguridad: coloca el brazo vertical [cite: 131-132]
  Braccio.begin();
  Serial.begin(9600);
}

void loop() {
  // Posición inicial de referencia (Vertical, pinza abierta)
  // Estructura: (delay, M1, M2, M3, M4, M5, M6)
  
  // 1. Mover la PINZA (M6)
  // Rango recomendado: 10 (cerrada) a 73 (abierta)
  Braccio.ServoMovement(25, 90, 90, 90, 90, 90, 10); // Cierra
  Braccio.ServoMovement(25, 90, 90, 90, 90, 90, 73); // Abre
  Braccio.ServoMovement(25, 90, 90, 90, 90, 90, 40); // Posición central (medio cerrada) [cite: 160]

  delay(1000);

  // 2. Mover la MUÑECA ROTATORIA (M5) 
  // Rango recomendado: 10 a 170
  Braccio.ServoMovement(25, 90, 90, 90, 90, 10,  40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 90, 90, 170, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 90, 90, 90,  40); // Central 

  delay(1000);

  // 3. Mover la MUÑECA VERTICAL (M4) 
  Braccio.ServoMovement(25, 90, 90, 90, 10,  90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 90, 170, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 90, 90,  90, 40); // Central

  delay(1000);

  // 4. Mover el CODO (M3) 
  Braccio.ServoMovement(25, 90, 90, 10,  90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 170, 90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90, 90,  90, 90, 40); // Central

  delay(1000);

  // 5. Mover el HOMBRO (M2) 
  Braccio.ServoMovement(25, 90, 10,  90, 90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 170, 90, 90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90, 90,  90, 90, 90, 40); // Central

  delay(1000);

  // 6. Mover la BASE (M1) 
  Braccio.ServoMovement(25, 10,  90, 90, 90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 170, 90, 90, 90, 90, 40);
  delay(1000);
  Braccio.ServoMovement(25, 90,  90, 90, 90, 90, 40); // Central

  delay(1000);

  // Final: Brazo totalmente vertical 
  // Nota: Ajustamos la pinza a 73 (abierta) para la posición vertical de reposo [cite: 130]
  Braccio.ServoMovement(25, 90, 90, 90, 90, 90, 73); 

  Serial.println("Ciclo completado. Esperando 5 segundos...");
  delay(5000); // Esperar 5 segundos antes de reiniciar 
}