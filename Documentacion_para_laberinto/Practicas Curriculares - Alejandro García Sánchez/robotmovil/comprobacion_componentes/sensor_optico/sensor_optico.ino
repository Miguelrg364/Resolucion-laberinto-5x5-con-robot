// Programa que lee el sensor óptico e imprime si estamos sobre superficie blanca, negra
// o no estamos sobre ninguna superficie.

// Pin del sensor
#define sensor1 1

// Valores que puede devolver la función lectura_sensor()
#define negro 0
#define blanco 1
#define sin_superficie 2

// Valores de referencia para usar en la función lectura_sensor()
#define referencia_negro 800
#define referencia_sinSuperficie 300

// Valores donde almacenar el valor que devuelve lectura_sensor()
short valor_leido; // Es tipo short porque como lectura_sensor() devuelve 0, 1 o 2,
// el rango de short es suficiente. Además ocupa menos espacio la variable

void setup() {
  pinMode(sensor1, INPUT);
  Serial.begin(230400); // Este es el baud rate al que funciona el micro que usamos para imprimir por pantalla
}

void loop() {
  valor_leido = lectura_sensor(sensor1);

  if(valor_leido == negro)
    Serial.println("Estoy sobre superficie negra");
  else if(valor_leido == blanco)
    Serial.println("Estoy sobre superficie blanca");
  else if(valor_leido == sin_superficie)
    Serial.println("No estoy sobre ninguna superficie");

  // Espera hasta la siguiente lectura
  delay(1000);
}

short lectura_sensor(unsigned int sensor) {
  // Leemos el sensor y dar un valor entre 0 y 1024
  int valor_leido = analogRead(sensor);

  Serial.println(valor_leido);

  // Variable donde almacenar el valor que devolveremos
  short resultado;

  // Si el valor que leemos está entre 0 y referencia_sinSuperficie, el sensor no está colocado
  //sobre una superficie
  if((valor_leido >= 0) && (valor_leido <= referencia_sinSuperficie)) {
    resultado = sin_superficie;
  }

  // Si el valor que leemos está entre referencia_sinSuperficie y referencia_blanco, el sensor está
  // sobre negro
  else if((valor_leido >= referencia_sinSuperficie) && (valor_leido <= referencia_negro)) {
    resultado = blanco;
  }

  // Si el valor que leemos está mayor que referencia_blanco, el sensor está sobre blanco
  else if((valor_leido >= referencia_negro)) {
    resultado = negro;
  }

  return resultado;
}
