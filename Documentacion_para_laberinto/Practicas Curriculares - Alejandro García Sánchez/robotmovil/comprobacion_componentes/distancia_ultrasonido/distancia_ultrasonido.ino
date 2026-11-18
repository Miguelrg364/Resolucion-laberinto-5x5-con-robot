// Programa que mide la distancia en cent metros usando un ultrasonido

// Pines ultrasonido
#define TRIGGER 13
#define ECHO 12

float distancia;        // Variable para almacenar la distancia
unsigned long tiempo;   // Variable para almacenar el tiempo

void setup() {
  pinMode(TRIGGER, OUTPUT);   // El trigger es salida
  pinMode(ECHO, INPUT);       // El echo es entrada 

  Serial.begin(9600);         // Para imprimir por pantalla
}


void loop() {
  // Apagamos el trigger
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  // Durante 10 microsegundos, el trigger manda pulsos y tras esto lo apagamos
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  // Medimos el tiempo que tarda el sonido desde que sale del trigger, rebota y llega al echo
  tiempo = pulseIn(ECHO, HIGH); // pulseIn() espera a detectar en el pin echo un HIGH y cronometra
  // cuanto tarda en pasar a LOW

  // El sonido tarda aprox. 29,2 microsegundos po centimetro y debemos dividir entre 2 porque sino
  // cuenta la distancia de ida y vuelta
  distancia = (tiempo/2)/29.2; // cm

  Serial.println(distancia); // Imprimimos la distancia por pantalla

  delay(1000);
}
