#include <InverseK.h>
#include <Braccio.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define out 2
#define s2 7
#define s3 8

#define RX A2
#define TX A3

int countRed = 0;
int countGreen = 0;
int countBlue = 0;

// Estructura para guardar los rangos de cada color mapeado
struct RangoColor {
  int rMin, rMax;
  int gMin, gMax;
  int bMin, bMax;
};

RangoColor rangoRojo, rangoNegro, rangoBlanco;

Servo base, shoulder, elbow, wrist_rot, wrist_ver, gripper;

// Agrupams coordenadas y offsets para que cada bloque sea independiente
struct PuntoConfig {
  float x, y, z;                          // Coordenadas teóricas
  int offM1, offM2, offM3, offM4, offM5;  // Correcciones para este punto exacto
};

// Calibración manual
// Aquí ajustamos los offsets para que en cada bloque la pinza actúe perfectamente
//                    { x ,      y ,    z , base, hombro, codo, muñecaV, muñecaR }
PuntoConfig bloque1 = { 180, 100, 20,  5,  -5,  0,  0,  120 };
PuntoConfig bloque2 = { 220, -30, 20,  0,  0,  0,  0,  80 };
PuntoConfig bloque3 = { 150, 0, 0,  0,  0,  0,  0,  90 };
PuntoConfig tolva   = { 100, 165, 50,  5,  10,  -10,  0,  140 };

const int PINZA_ABIERTA = 73;
const int PINZA_CERRADA = 10;
const int OFFSET_BAJADA_HOMBRO = 20;

// Valores actuales que tendrán los motores para guardar sus estados
int actualM1, actualM2, actualM3, actualM4, actualM5, actualM6; 

// Variable para comparar el estado anterior
String ultimaOrdenEnviada = "";

String penultimoComando = "";
String ultimoComando = "";

// Configuración Bluetooth
SoftwareSerial btSerial(RX, TX);


void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  Braccio.begin();

  Link base1, upperarm, forearm, hand;
  base1.init(0, b2a(0.0), b2a(180.0));
  upperarm.init(125, b2a(15.0), b2a(165.0));
  forearm.init(125, b2a(0.0), b2a(180.0));
  hand.init(190, b2a(0.0), b2a(180.0));

  InverseK.attach(base1, upperarm, forearm, hand);

  // Sensor RGB 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  calibrarSensorColor();

  Serial.println("Escriba un comando (B1, B2, B3, COGER, SOLTAR):");
}


void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    // Validar y ejecutar comandos conocidos
    bool comandoValido = false;

    if (comando == "B1") { irPuntoInversa(bloque1); comandoValido = true; }
    else if (comando == "B2") { irPuntoInversa(bloque2); comandoValido = true; }
    else if (comando == "B3") { irPuntoInversa(bloque3); comandoValido = true; }
    else if (comando == "TOLVA") { irPuntoInversa(tolva); comandoValido = true; }
    else if (comando == "COGER") { cogerDirecta(); comandoValido = true; }
    else if (comando == "SOLTAR") { soltarDirecta(); comandoValido = true; }
    
    if (comandoValido) {
      // Desplazar el historial de comandos
      penultimoComando = ultimoComando;
      ultimoComando = comando;
      
      // IMPLEMENTACIÓN TODO 2: Comprobar si la secuencia es TOLVA y luego SOLTAR
      if (penultimoComando == "TOLVA" && ultimoComando == "SOLTAR") {
        bool ordenProcesada = false;
        
        while (!ordenProcesada) {
          Serial.println("Secuencia detectada (TOLVA -> SOLTAR). Escaneando color...");
          String colorDetectado = leerColorTolva();
          
          // Extraer la parte del color legible limpiando los prefijos "INICIO_" de tu lógica original
          String colorLimpio = colorDetectado;
          if (colorDetectado.startsWith("INICIO_")) {
            colorLimpio = colorDetectado.substring(7); // Quita "INICIO_"
          }
          
          Serial.print("SE HA DETECTADO EL COLOR ");
          Serial.print(colorLimpio);
          Serial.println(", ¿DESEAS ENVIAR LA ORDEN? (S/N)");
          
          // Esperar la respuesta del usuario por el monitor de serie
          while (Serial.available() == 0) {
            delay(10);
          }
          
          String respuesta = Serial.readStringUntil('\n');
          respuesta.trim();
          respuesta.toUpperCase();
          
          if (respuesta == "S") {
            // MODIFICACIÓN: Extraemos la primera letra ('R', 'B', 'N') para enviársela al coche optimizado
            char letraComando = colorLimpio.charAt(0); 
            btSerial.print(letraComando); // Envía solo el carácter
            
            Serial.print("Orden '");
            Serial.print(letraComando);
            Serial.println("' enviada por Bluetooth. Volviendo a esperar órdenes...");
            ordenProcesada = true;
          } else if (respuesta == "N") {
            Serial.println("Orden rechazada. Re-escaneando...");
          } else {
            Serial.println("Respuesta no válida. Por favor introduce 'S' o 'N'.");
          }
          
          // Limpiar el búfer serie restante
          while(Serial.available() > 0) Serial.read();
        }
        
        // Resetear el historial de secuencia para evitar bucles infinitos en la próxima iteración
        penultimoComando = "";
        ultimoComando = "";
      }
    }

    while(Serial.available() > 0) Serial.read();
  }

}

void irPuntoInversa(PuntoConfig p) {
  float a0, a1, a2, a3;
  
  // Resolvemos la posición teórica
  if(InverseK.solve(p.x, p.y, p.z, a0, a1, a2, a3)) {
    
    // 1. Convertimos a ángulos Braccio
    int baseM1 = a2b(a0);
    int shoulderM2 = a2b(a1);
    int elbowM3 = a2b(a2);
    int wirst_verM4 = a2b(a3);
    int wirst_rotM5 = 0; // Valor por defecto

    // 2. Aplicamos los offsets para cada articulación
    actualM1 = constrain(baseM1 + p.offM1, 0, 180);
    actualM2 = constrain(shoulderM2 + p.offM2, 0, 180);
    actualM3 = constrain(elbowM3 + p.offM3, 0, 180);
    actualM4 = constrain(wirst_verM4 + p.offM4, 0, 180);
    actualM5 = constrain(wirst_rotM5 + p.offM5, 0, 180);

    // 3. Movemos manteniendo la pinza como esté (actualM6)
    Braccio.ServoMovement(20, actualM1, actualM2, actualM3, actualM4, actualM5, actualM6);
    
    Serial.print("Posicionado en ("); Serial.print(p.x); Serial.print(","); Serial.print(p.y); Serial.println(")");
    Serial.print("Valores motores: "); Serial.print(actualM1); Serial.print(","); Serial.print(actualM2);Serial.print(","); Serial.print(actualM3); 
    Serial.print(","); Serial.print(actualM4); Serial.print(","); Serial.print(actualM5); Serial.print(","); Serial.println(actualM6);
  } else {
    Serial.println("Error: El bloque está en una zona físicamente imposible.");
  }
}

void soltarDirecta() {
  Serial.println("Bajando hombro para soltar...");
  int m2_abajo = constrain(actualM2 - OFFSET_BAJADA_HOMBRO, 15, 165);
  
  Braccio.ServoMovement(20, actualM1, m2_abajo, actualM3, actualM4, actualM5, PINZA_ABIERTA);
  delay(800);
  actualM6 = PINZA_CERRADA;
  Braccio.ServoMovement(20, actualM1, m2_abajo, actualM3, actualM4, actualM5, actualM6);
  delay(800);
  Braccio.ServoMovement(20, actualM1, actualM2, actualM3, actualM4, actualM5, actualM6);
  delay(800);
}

void cogerDirecta() {
  Serial.println("Bajando para coger...");
  int m2_abajo = constrain(actualM2 - OFFSET_BAJADA_HOMBRO, 15, 165);
  
  Braccio.ServoMovement(20, actualM1, m2_abajo, actualM3, actualM4, actualM5, PINZA_CERRADA);
  delay(800);
  actualM6 = PINZA_ABIERTA;
  Braccio.ServoMovement(20, actualM1, m2_abajo, actualM3, actualM4, actualM5, actualM6);
  delay(800);
  Braccio.ServoMovement(20, actualM1, actualM2, actualM3, actualM4, actualM5, actualM6);
  delay(800);
}

String leerColorTolva() {
  // Leer canal ROJO
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  countRed = pulseIn(out, LOW);
  delay(20);
  
  // Leer canal AZUL
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  countBlue = pulseIn(out, LOW);
  delay(20);
  
  // Leer canal VERDE
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  countGreen = pulseIn(out, LOW);
  delay(20);
  
  Serial.print("Lectura actual -> R:"); Serial.print(countRed);
  Serial.print(" G:"); Serial.print(countGreen);
  Serial.print(" B:"); Serial.println(countBlue);

  // ¿Coincide con los rangos guardados para el ROJO?
  if (countRed   >= rangoRojo.rMin && countRed   <= rangoRojo.rMax &&
      countGreen >= rangoRojo.gMin && countGreen <= rangoRojo.gMax &&
      countBlue  >= rangoRojo.bMin && countBlue  <= rangoRojo.bMax) { 
    return "INICIO_ROJO";
  } 
  
  // ¿Coincide con los rangos guardados para el NEGRO?
  else if (countRed   >= rangoNegro.rMin && countRed   <= rangoNegro.rMax &&
           countGreen >= rangoNegro.gMin && countGreen <= rangoNegro.gMax &&
           countBlue  >= rangoNegro.bMin && countBlue  <= rangoNegro.bMax) {
    return "INICIO_NEGRO";
  } 
  
  // ¿Coincide con los rangos guardados para el BLANCO?
  else if (countRed   >= rangoBlanco.rMin && countRed   <= rangoBlanco.rMax &&
           countGreen >= rangoBlanco.gMin && countGreen <= rangoBlanco.gMax &&
           countBlue  >= rangoBlanco.bMin && countBlue  <= rangoBlanco.bMax) {
    return "INICIO_BLANCO";
  }
  
  else {
    return "NINGUNO";
  }
}

void esperarTecla() {
  while (Serial.available() > 0) Serial.read(); // Limpiar búfer
  while (Serial.available() == 0) {
    delay(10); // Esperar a que el usuario pulse Enter/Envíe algo
  }
  while (Serial.available() > 0) Serial.read(); // Limpiar respuesta
}

void calibrarSensorColor() {
  Serial.println("--- ASISTENTE DE CALIBRACIÓN DE COLORES ---");

  // 1. CALIBRAR ROJO
  Serial.println("1. Coloque el objeto ROJO frente al sensor y pulse ENTER...");
  esperarTecla();
  digitalWrite(s2, LOW); digitalWrite(s3, LOW); int rRojo = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, LOW); digitalWrite(s3, HIGH); int bRojo = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, HIGH); digitalWrite(s3, HIGH); int gRojo = pulseIn(out, LOW); delay(20);
  
  rangoRojo.rMin = rRojo - 20; rangoRojo.rMax = rRojo + 20;
  rangoRojo.gMin = gRojo - 20; rangoRojo.gMax = gRojo + 20;
  rangoRojo.bMin = bRojo - 20; rangoRojo.bMax = bRojo + 20;
  Serial.println("-> ¡Rojo calibrado!");

  // 2. CALIBRAR NEGRO
  Serial.println("2. Coloque el objeto NEGRO frente al sensor y pulse ENTER...");
  esperarTecla();
  digitalWrite(s2, LOW); digitalWrite(s3, LOW); int rNegro = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, LOW); digitalWrite(s3, HIGH); int bNegro = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, HIGH); digitalWrite(s3, HIGH); int gNegro = pulseIn(out, LOW); delay(20);
  
  rangoNegro.rMin = rNegro - 20; rangoNegro.rMax = rNegro + 20;
  rangoNegro.gMin = gNegro - 20; rangoNegro.gMax = gNegro + 20;
  rangoNegro.bMin = bNegro - 20; rangoNegro.bMax = bNegro + 20;
  Serial.println("-> ¡Negro calibrado!");

  // 3. CALIBRAR BLANCO
  Serial.println("3. Coloque el objeto BLANCO frente al sensor y pulse ENTER...");
  esperarTecla();
  digitalWrite(s2, LOW); digitalWrite(s3, LOW); int rBlanco = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, LOW); digitalWrite(s3, HIGH); int bBlanco = pulseIn(out, LOW); delay(20);
  digitalWrite(s2, HIGH); digitalWrite(s3, HIGH); int gBlanco = pulseIn(out, LOW); delay(20);
  
  rangoBlanco.rMin = rBlanco - 20; rangoBlanco.rMax = rBlanco + 20;
  rangoBlanco.gMin = gBlanco - 20; rangoBlanco.gMax = gBlanco + 20;
  rangoBlanco.bMin = bBlanco - 20; rangoBlanco.bMax = bBlanco + 20;
  Serial.println("-> ¡Blanco calibrado!\n--- CALIBRACIÓN FINALIZADA ---\n");
}

float b2a(float b){ return b / 180.0 * PI - HALF_PI; }
float a2b(float a) { return (a + HALF_PI) * 180 / PI; }