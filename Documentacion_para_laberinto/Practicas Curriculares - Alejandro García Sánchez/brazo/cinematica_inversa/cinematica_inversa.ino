#include <InverseK.h>
#include <Braccio.h>
#include <Servo.h>

Servo base, shoulder, elbow, wrist_rot, wrist_ver, gripper;

// Usamos la misma estructura de tu código para pasar la configuración
struct PuntoConfig {
  float x, y, z;
  int offM1, offM2, offM3, offM4, offM5; // Mantenemos los offsets en 0 para coordenadas libres
};

const int PINZA_ABIERTA = 10;
const int PINZA_CERRADA = 73;
const int OFFSET_BAJADA_HOMBRO = 20;

// Valores actuales que tendrán los motores para guardar sus estados
int actualM1, actualM2, actualM3, actualM4, actualM5, actualM6 = 10;
String ultimaOrdenEnviada = "";

String penultimoComando = "";
String ultimoComando = "";

void setup() {
  Serial.begin(9600);
  Braccio.begin();
  Link base1, upperarm, forearm, hand;
  base1.init(0, b2a(0.0), b2a(180.0));
  upperarm.init(125, b2a(15.0), b2a(165.0));
  forearm.init(125, b2a(0.0), b2a(180.0));
  hand.init(190, b2a(0.0), b2a(180.0));
  InverseK.attach(base1, upperarm, forearm, hand);
  
  // Mensaje inicial de la terminal interactiva
  Serial.println("--- TERMINAL DE COORDENADAS INTERACTIVA ---");
  Serial.println("Introduce las coordenadas con el formato: X,Y,Z (Ejemplo: 120,100,30) y pulsa Enter:");
}

void loop() {
  if (Serial.available() > 0) {
    // Leemos la línea introducida por el usuario
    String entrada = Serial.readStringUntil('\n');
    entrada.trim(); // Eliminamos espacios en blanco o basura

    // Buscamos las comas para separar X, Y y Z
    int primeraComa = entrada.indexOf(',');
    int segundaComa = entrada.indexOf(',', primeraComa + 1);

    // Validación básica de formato: deben existir al menos dos comas
    if (primeraComa != -1 && segundaComa != -1) {
      
      // Extraemos las subcadenas y las convertimos a números flotantes
      float destinoX = entrada.substring(0, primeraComa).toFloat();
      float destinoY = entrada.substring(primeraComa + 1, segundaComa).toFloat();
      float destinoZ = entrada.substring(segundaComa + 1).toFloat();

      Serial.print("Procesando movimiento hacia -> X: ");
      Serial.print(destinoX);
      Serial.print(", Y: ");
      Serial.print(destinoY);
      Serial.print(", Z: ");
      Serial.println(destinoZ);

      // Creamos la configuración del punto usando la misma estructura de tu código original
      // Dejamos los offsets individuales en 0 para que el movimiento sea directo y teórico
      PuntoConfig puntoUsuario = { destinoX, destinoY, destinoZ, 0, 0, 0, 0, 0 };

      // Reutilizamos tu función nativa de cinemática inversa
      irPuntoInversa(puntoUsuario);

    } else {
      // Mensaje de error si el formato introducido no es correcto
      Serial.println("Error: Formato incorrecto. Recuerda usar el formato X,Y,Z sin espacios (Ej: 100,100,40)");
    }

    Serial.println("\nIntroduce nuevas coordenadas (X,Y,Z):");
    
    // Limpiamos cualquier residuo en el búfer serie
    while(Serial.available() > 0) Serial.read();
  }
}

// Función que mueve el brazo a unas coordenadas
void irPuntoInversa(PuntoConfig p) {
  float a0, a1, a2, a3;
  // Resolvemos la posición teórica utilizando tu librería InverseK
  if(InverseK.solve(p.x, p.y, p.z, a0, a1, a2, a3)) {
    
    // 1. Convertimos a ángulos Braccio
    int baseM1 = a2b(a0);
    int shoulderM2 = a2b(a1);
    int elbowM3 = a2b(a2);
    int wirst_verM4 = a2b(a3);
    int wirst_rotM5 = 0;

    // 2. Aplicamos los offsets para cada articulación (en este caso irán en 0)
    actualM1 = constrain(baseM1 + p.offM1, 0, 180);
    actualM2 = constrain(shoulderM2 + p.offM2, 0, 180);
    actualM3 = constrain(elbowM3 + p.offM3, 0, 180);
    actualM4 = constrain(wirst_verM4 + p.offM4, 0, 180);
    actualM5 = constrain(wirst_rotM5 + p.offM5, 0, 180);
    
    // 3. Movemos manteniendo la pinza en su estado actual (actualM6)
    Braccio.ServoMovement(20, actualM1, actualM2, actualM3, actualM4, actualM5, actualM6);
    
    Serial.print("¡Posicionado con éxito en! ("); Serial.print(p.x); Serial.print(","); Serial.print(p.y); Serial.println(")");
    Serial.print("Valores calculados motores: "); Serial.print(actualM1); Serial.print(","); Serial.print(actualM2);Serial.print(","); Serial.print(actualM3); 
    Serial.print(","); Serial.print(actualM4); Serial.print(","); Serial.print(actualM5); Serial.print(","); Serial.println(actualM6);
  } else {
    Serial.println("Error: Las coordenadas solicitadas están en una zona físicamente imposible para el brazo.");
  }
}


float b2a(float b){ return b / 180.0 * PI - HALF_PI; }
float a2b(float a) { return (a + HALF_PI) * 180 / PI; }