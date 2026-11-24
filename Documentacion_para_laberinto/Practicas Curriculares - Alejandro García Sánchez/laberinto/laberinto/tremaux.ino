#include <Servo_megaTinyCore.h>

// --- ASIGNACIÓN DE PINES ---
#define TRIG 13
#define ECHO 12
#define PIN_SERVO 14
#define BUZZER 5    

// Motores
#define STBY 15
#define AIN 10   
#define APWM 16  
#define BIN 11   
#define BPWM 9   

// CNY
#define CNY1 4
#define CNY2 3
#define CNY3 2
#define CNY4 1

// --- CONSTANTES DEL SISTEMA ---
#define UMBRAL_DISTANCIA 20 
#define VELOCIDAD 120
#define VEL_CORRECCION 60  
#define VEL_GIRO 100 
#define UMBRAL_NEGRO 600    
#define OFFSET_MECANICO 120 
#define TAM_LABERINTO 5

// --- SISTEMA DE MAPEO Y MÁQUINA DE ESTADOS ---
enum Direccion { NORTE = 0, ESTE = 1, SUR = 2, OESTE = 3 };
const uint8_t MASCARA_DIR[4] = {1, 2, 4, 8}; 
const uint8_t MASCARA_VISITADO = 16;   
const uint8_t MASCARA_REC[4] = {1, 4, 16, 64};

uint8_t mapa[TAM_LABERINTO][TAM_LABERINTO] = {0}; 
uint8_t distancias[TAM_LABERINTO][TAM_LABERINTO] = {0};         
uint8_t recorridos[TAM_LABERINTO][TAM_LABERINTO] = {0};         //Cada par de bits | [0,1] => Norte, [2,3] => Este, [4,5] => Sur, [6,7] => Oeste | representa un estado:
                                                                // 00 => No recorrido nunca, 01 => Recorrido una sola vez, 11 => Recorrido dos veces
int8_t posX = 0;
int8_t posY = 0;
Direccion orientacion = NORTE;

// VARIABLES DE META (Dinámicas por Bluetooth, sin 'const')
int8_t metaX = 0; 
int8_t metaY = 0; 

// FASES DEL ROBOT:
// 0 = Esperando coordenadas por Bluetooth
// 1 = Explorando hacia meta desconocida (Mano derecha)   CAMBIAR MANO DERECHA POR EL ALGORITMO DE TRÉMAUX
// 2 = Regresando al inicio [0,0] (Flood Fill)            CAMBIAR FLOOD FILL POR ALGORITMO A*
// 3 = Speedrun hacia meta conocida (Flood Fill)
uint8_t estadoActual = 0; 

Servo radar;

void setup() {
  Serial.begin(9600);
  
  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT);
  pinMode(CNY1, INPUT); pinMode(CNY2, INPUT);
  pinMode(CNY3, INPUT); pinMode(CNY4, INPUT);
  pinMode(BUZZER, OUTPUT); 
  
  pinMode(STBY, OUTPUT); 
  pinMode(AIN, OUTPUT); pinMode(BIN, OUTPUT);
  pinMode(APWM, OUTPUT); pinMode(BPWM, OUTPUT);
  
  digitalWrite(STBY, HIGH); 
  pararMotores();

  radar.attach(PIN_SERVO);
  radar.write(90); 

  // Cerrar bordes virtuales en memoria  (LOS INDICES ESTAN PUESTOS COMO SI EL LABERINTO SE ENCONTRASE EN EL 1ºCUADRANTE: EL PRIMER INDICE EL VALOR DE X, Y EL SEGUNDO EL DE Y)
  for(int i=0; i<TAM_LABERINTO; i++) {
    mapa[i][TAM_LABERINTO-1] |= MASCARA_DIR[NORTE]; 
    mapa[i][0] |= MASCARA_DIR[SUR];   
    mapa[TAM_LABERINTO-1][i] |= MASCARA_DIR[ESTE];  
    mapa[0][i] |= MASCARA_DIR[OESTE]; 
  }

  // Asegurar que el punto de partida [0,0] no tenga paredes internas corruptas
  mapa[0][0] &= ~MASCARA_DIR[NORTE];
  mapa[0][0] &= ~MASCARA_DIR[ESTE];

  // Pitido de sistema listo
  pitar(1500, 100); delay(80); pitar(1500, 100);
}

void loop() {
  switch (estadoActual) {
    
    case 0:
      esperarComandoBluetooth();
      break;

    case 1:
    case 2:
    case 3:

      // Escanear físicamente solo si estamos explorando (Fase 1) y la casilla es nueva
      if (estadoActual == 1 && !esVisitada(posX, posY)) {
        escanearYMapear();
        marcarVisitado(posX, posY);
      }

      // Decidir rumbo según la fase
      if (estadoActual == 1) ejecutarDecisionExploracionTremaux();
      else                    ejecutarDecisionFloodFill();

      // Moverse al siguiente cruce
      avanzarHastaCruce();

      // Actualizar posición interna
      actualizarCoordenadas();
      if()
      
      // Verificar si hemos alcanzado hitos (Meta o Inicio)
      comprobarHitosDeLlegada();
      
      delay(200);
      break;
  }
}

void comprobarHitosDeLlegada() {
  
  // HITO 1: Llegada a la Meta (tanto si es es explorando como de manera directa)
  if ((estadoActual == 1 || estadoActual == 3) && posX == metaX && posY == metaY) {
    pararMotores();

    // Forzamos el escaneo y marcado de la casilla de meta antes de 
    // cambiar de estado para no olvidarla jamás en la memoria.
    if (!esVisitada(posX, posY)) {
      escanearYMapear();
      marcarVisitado(posX, posY);
    }
    
    pitar(1200, 150); delay(100);
    pitar(1500, 150); delay(100);
    pitar(1800, 400);
    
    delay(1000); 
    
    calcularFloodFill(0, 0); 
    estadoActual = 2;        
  }
  
  // HITO 2: Regreso Exitoso a la Casilla de Inicio [0,0]
  else if (estadoActual == 2 && posX == 0 && posY == 0) {
    pararMotores();
    delay(300);

    // AUTO-ALINEACIÓN: Volver a mirar al Norte obligatoriamente
    if (orientacion == ESTE)       girarCoche(-90); 
    else if (orientacion == SUR)   girarCoche(180); 
    else if (orientacion == OESTE)  girarCoche(90);  
    
    pararMotores();

    pitar(1200, 150); delay(100);
    pitar(1500, 150); delay(100);
    pitar(1800, 400);
    
    estadoActual = 0; 
  }
}

// --- TOMA DE DECISIONES ---    
// Exploracion por mano derecha (no la usamos)       
void ejecutarDecisionExploracion() {                            
  Direccion dirDer = (Direccion)((orientacion + 1) % 4);
  Direccion dirFrente = orientacion;
  Direccion dirIzq = (Direccion)((orientacion + 3) % 4);

  bool derLibre = !hayPared(posX, posY, dirDer);
  bool frenteLibre = !hayPared(posX, posY, dirFrente);
  bool izqLibre = !hayPared(posX, posY, dirIzq);

  // PRIORIDAD 1: Ir por caminos libres que NUNCA hemos pisado antes
  if (derLibre && !esVisitadaDir(dirDer))       girarCoche(90);
  else if (frenteLibre && !esVisitadaDir(dirFrente)) { }
  else if (izqLibre && !esVisitadaDir(dirIzq))  girarCoche(-90);
  
  // PRIORIDAD 2: Si estamos rodeados de casillas ya visitadas (Ej: al iniciar una segunda misión),
  // usamos la regla de la mano derecha pura para navegar por la zona conocida hasta encontrar algo nuevo.
  else if (derLibre)                            girarCoche(90);
  else if (frenteLibre)                              { }
  else if (izqLibre)                            girarCoche(-90);
  else                                          girarCoche(180); // Callejón sin salida
}


//Exploración por el algoritmo de Trémaux
void ejecutarDecisionExploracionTremaux(){ 
  Direccion dirFrente = orientacion;        
  Direccion dirDer = (Direccion)((orientacion + 1) % 4);
  Direccion dirDetras = (Direccion)((orientacion + 2) % 4);
  Direccion dirIzq = (Direccion)((orientacion + 3) % 4);

  bool frenteLibre = !hayPared(posX, posY, dirFrente);
  bool derLibre = !hayPared(posX, posY, dirDer);
  bool detrasLibre = !hayPared(posX, posY, dirDetras);
  bool izqLibre = !hayPared(posX, posY, dirIzq);

  // PRIORIDAD 1: Ir por caminos libres que NUNCA hemos pisado antes
  if (derLibre && !recorrido1Vez(dirDer)){
      girarCoche(90);
      marcarRecorrido1Vez(dirDer);
  }
  else if (frenteLibre && !recorrido1Vez(dirFrente)){ 
      marcarRecorrido1Vez(dirFrente);
  }
  else if (izqLibre && !recorrido1Vez(dirIzq)){
      girarCoche(-90);
      marcarRecorrido1Vez(dirIzq);
  }
  
  // PRIORIDAD 2: Ir por caminos que hemos pisado UNA SOLA VEZ
  else if (derLibre && !recorrido2Veces(dirDer)){
      girarCoche(90);
      marcarRecorrido2Veces(dirDer);
  }
  else if (frenteLibre && !recorrido2Veces(dirFrente)){ 
      marcarRecorrido2Veces(dirFrente);
  }
  else if (izqLibre && !recorrido2Veces(dirIzq)){
      girarCoche(-90);
      marcarRecorrido2Veces(dirIzq);
  }
  
  //No hay caminos libres o que no hayamos pasado por ellos 2 veces, así que nos damos la vuelta
  else{
    girarCoche(180);
    if(!recorrido1Vez(dirDetras)) marcarRecorrido1Vez(dirDetras);
    else if (!recorrido2Veces(dirDetras)) marcarRecorrido2Veces(dirDetras);
  }
}

//Direccion recorrida AL MENOS UNA vez
bool recorrido1Vez(Direccion dir){
  return((recorridos[posX][posY] & MASCARA_REC[dir]) != 0);
}

//Direccion recorrida 2 veces
bool recorrido2Veces(Direccion dir){
  return((recorridos[posX][posY] & MASCARA_REC[dir]*2) != 0);
}

void marcarRecorrido1Vez(Direccion dir){
  int8_t vx = obtenerVecinoX(dir);
  int8_t vy = obtenerVecinoY(dir);
  recorridos[posX][posY] |= MASCARA_REC[dir];
  recorridos[vx][vy] |= MASCARA_REC[(dir+2)%4];
}

void marcarRecorrido2Veces(Direccion dir){
  int8_t vx = obtenerVecinoX(dir);
  int8_t vy = obtenerVecinoY(dir);
  recorridos[posX][posY] |= MASCARA_REC[dir]*2;
  recorridos[vx][vy] |= MASCARA_REC[(dir+2)%4]*2;
}


// --- FUNCIONES DE MEMORIA ESPACIAL ---
int8_t obtenerVecinoX(Direccion dir) { return (dir == ESTE) ? posX + 1 : (dir == OESTE) ? posX - 1 : posX; }
int8_t obtenerVecinoY(Direccion dir) { return (dir == NORTE) ? posY + 1 : (dir == SUR) ? posY - 1 : posY; }

void actualizarCoordenadas() {
  posX = obtenerVecinoX(orientacion);
  posY = obtenerVecinoY(orientacion);
}

void guardarPared(int8_t x, int8_t y, Direccion dir) {
  if (x >= 0 && x < TAM_LABERINTO && y >= 0 && y < TAM_LABERINTO) {
    mapa[x][y] |= MASCARA_DIR[dir]; 
    
    int8_t vx = x; int8_t vy = y;
    Direccion dirEspejo;
    
    if (dir == NORTE) { vy++; dirEspejo = SUR; }
    else if (dir == SUR)   { vy--; dirEspejo = NORTE; }
    else if (dir == ESTE)  { vx++; dirEspejo = OESTE; }
    else if (dir == OESTE) { vx--; dirEspejo = ESTE; }
    
    if (vx >= 0 && vx < TAM_LABERINTO && vy >= 0 && vy < TAM_LABERINTO) {
      mapa[vx][vy] |= MASCARA_DIR[dirEspejo];
    }
  }
}

bool hayPared(int8_t x, int8_t y, Direccion dir) {
  // Comprobación estricta de salida del tablero físico
  if (dir == NORTE && y == 4) return true;
  if (dir == SUR   && y == 0) return true;
  if (dir == ESTE  && x == 4) return true;
  if (dir == OESTE && x == 0) return true;

  if (x < 0 || x >= TAM_LABERINTO || y < 0 || y >= TAM_LABERINTO) return true;
  return (mapa[x][y] & MASCARA_DIR[dir]) != 0; 
}

void marcarVisitado(int8_t x, int8_t y) { mapa[x][y] |= MASCARA_VISITADO; }
bool esVisitada(int8_t x, int8_t y) { return (mapa[x][y] & MASCARA_VISITADO) != 0; }

bool esVisitadaDir(Direccion dir) {
  int8_t ax = obtenerVecinoX(dir); int8_t ay = obtenerVecinoY(dir);
  if (ax < 0 || ax >= TAM_LABERINTO || ay < 0 || ay >= TAM_LABERINTO) return true;
  return esVisitada(ax, ay);
}

// --- FUNCIONES FÍSICAS ---
void avanzarHastaCruce() {
  mover(VELOCIDAD, VELOCIDAD);
  delay(60); 

  while (true) {
    int val1 = analogRead(CNY1); int val2 = analogRead(CNY2);
    int val3 = analogRead(CNY3); int val4 = analogRead(CNY4);

    // MANTENIDO: Operador && estricto para asegurar la cruz perfecta tal y como pides
    bool cruceIzquierda = (val1 >= UMBRAL_NEGRO && val2 >= UMBRAL_NEGRO);
    bool cruceDerecha   = (val4 >= UMBRAL_NEGRO && val3 >= UMBRAL_NEGRO);

    if (cruceIzquierda && cruceDerecha) {
      mover(VELOCIDAD, VELOCIDAD); delay(OFFSET_MECANICO);
      pararMotores();
      break;
    }

    if (val2 >= UMBRAL_NEGRO && val3 < UMBRAL_NEGRO)       mover(VEL_CORRECCION, VELOCIDAD);
    else if (val2 < UMBRAL_NEGRO && val3 >= UMBRAL_NEGRO)  mover(VELOCIDAD, VEL_CORRECCION);
    else                                                   mover(VELOCIDAD, VELOCIDAD);
  }
}

void girarCoche(int grados) {
  if (grados == 90)       { mover(VEL_GIRO, -VEL_GIRO); orientacion = (Direccion)((orientacion + 1) % 4); }
  else if (grados == -90)  { mover(-VEL_GIRO, VEL_GIRO); orientacion = (Direccion)((orientacion + 3) % 4); }
  else if (grados == 180)  { mover(VEL_GIRO, -VEL_GIRO); orientacion = (Direccion)((orientacion + 2) % 4); }
  
  delay(450); 

  // --- PARCHE DE 180 GRADOS (ESCAPE INTELIGENTE AL BLANCO) ---
  if (grados == 180) {
    // 1. Esperamos a detectar la línea intermedia (90º)
    while(true) {
      if (analogRead(CNY2) >= UMBRAL_NEGRO || analogRead(CNY3) >= UMBRAL_NEGRO) {
        break; 
      }
    }
    // 2. ESCAPE AL BLANCO: En lugar de un delay de tiempo, obligamos al coche 
    // a seguir girando hasta que ambos sensores centrales lean BLANCO.
    // Así nos aseguramos de que ha rebasado por completo el grosor de la cinta.
    while(true) {
      if (analogRead(CNY2) < UMBRAL_NEGRO && analogRead(CNY3) < UMBRAL_NEGRO) {
        break;
      }
    }
    // Pequeñísimo extra para que no roce el borde
    delay(350); 
  }

  // --- Búsqueda de la línea de destino final ---
  while(true) {
    int val2 = analogRead(CNY2);
    int val3 = analogRead(CNY3);
    
    if (val2 >= UMBRAL_NEGRO || val3 >= UMBRAL_NEGRO) {
      break; 
    }
  }
  
  pararMotores();
  delay(150); 
}

void mover(int velIzq, int velDer) {
  digitalWrite(AIN, velIzq >= 0 ? HIGH : LOW); analogWrite(APWM, abs(velIzq));
  digitalWrite(BIN, velDer >= 0 ? HIGH : LOW); analogWrite(BPWM, abs(velDer));
}

void pararMotores() { analogWrite(APWM, 0); analogWrite(BPWM, 0); }

void pitar(unsigned int frecuencia, unsigned long duracion) {
  unsigned long periodo_us = 1000000L / frecuencia;
  unsigned long mitadPeriodo = periodo_us / 2;
  unsigned long tiempoInicio = millis();
  while (millis() - tiempoInicio < duracion) {
    digitalWrite(BUZZER, HIGH); delayMicroseconds(mitadPeriodo);
    digitalWrite(BUZZER, LOW); delayMicroseconds(mitadPeriodo);
  }
}

long leerUltrasonido() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duracion = pulseIn(ECHO, HIGH, 20000); 
  if (duracion == 0) return 999; 
  return (duracion/2)/29.2; 
}

void escanearYMapear() {
  Direccion dirDer = (Direccion)((orientacion + 1) % 4);
  Direccion dirFrente = orientacion;
  Direccion dirIzq = (Direccion)((orientacion + 3) % 4);

  radar.write(0); delay(350); 
  if (leerUltrasonido() <= UMBRAL_DISTANCIA) guardarPared(posX, posY, dirDer);

  radar.write(90); delay(350); 
  if (leerUltrasonido() <= UMBRAL_DISTANCIA) guardarPared(posX, posY, dirFrente);

  radar.write(180); delay(350); 
  if (leerUltrasonido() <= UMBRAL_DISTANCIA) guardarPared(posX, posY, dirIzq);

  radar.write(90); delay(200); 
}
