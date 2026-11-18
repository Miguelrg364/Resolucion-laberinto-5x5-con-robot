// Programa sigue la línea

// Pines sensores ópticos línea
#define CNY8 1
#define CNY7 2
#define CNY6 3
#define CNY5 4

// Pines PWM
#define APWM 9
#define BPWM 16

// Pines dirección
#define AIN_ 10
#define BIN_ 11

// Pin StandBy
#define STBY 15

// Pines ultrasonido
#define TRIGGER 13
#define ECHO 12

// Valores que puede devolver la función lectura_sensor()
#define negro 0
#define blanco 1
#define sin_superficie 2

// Valores de referencia para usar en la función lectura_sensor()
#define referencia_negro 800
#define referencia_sinSuperficie 300

float distancia;
unsigned long tiempo;
short cn5, cn6, cn7, cn8;

void setup() {
  pinMode(APWM, OUTPUT);
  pinMode(AIN_, OUTPUT);
  pinMode(BIN_, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, LOW); // Dormimos al motor inicialmente (disminuimos consumo)

  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(CNY8, INPUT);
  pinMode(CNY7, INPUT);
  pinMode(CNY6, INPUT);
  pinMode(CNY5, INPUT);
}

void loop() {
  // Primero vemos si hay un obstáculos
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  // Medimos tiempo para calcular luego la distancia
  tiempo = pulseIn(ECHO, HIGH);

  // Calculamos distancia sabiendo la ecuación
  distancia = (tiempo/2)/29.2;

  // Si hay un obstáculo a 10 cm, el robot se para
  if (distancia <= 10.0) {
    analogWrite(APWM, 0);
    analogWrite(BPWM, 0);
    delay(500);
  } else { // Si no, que avance
    digitalWrite(STBY, HIGH);  // Despertamos el motor
    analogWrite(APWM, 100);
    analogWrite(BPWM, 100); // Establecemos velocidades

    digitalWrite(AIN_, HIGH); // Establecemos sentedio de giro
    digitalWrite(BIN_, HIGH);

    cn5 = lectura_sensor(CNY5);
    cn6 = lectura_sensor(CNY6);
    cn7 = lectura_sensor(CNY7);
    cn8 = lectura_sensor(CNY8);

    // Si hay curva hacia la derecha, gira hacia la derecha
    if(cn5 == blanco && cn6 == negro && cn7 == negro && cn8 == negro) {
      analogWrite(APWM, 120);
      analogWrite(BPWM, 80);
    }
    // Si hay una curva hacia la derecha pronunciada, gira hacia la derecha más
    else if(cn5 == blanco && cn6 == blanco && cn7 == negro && cn8 == negro) {
      analogWrite(APWM, 140);
      analogWrite(BPWM, 60);
    }
    // Si hay una curva hacia la derecha muy pronunciada, gira hacia la derecha aún más
    else if(cn5 == blanco && cn6 == blanco && cn7 == blanco && cn8 == negro) {
      analogWrite(APWM, 140);
      analogWrite(BPWM, 60);
    }
    // Si hay una curva hacia la izquierda, gira hacia la izquierda
    else if(cn5 == negro && cn6 == negro && cn7 == negro && cn8 == blanco) {
      analogWrite(APWM, 80);
      analogWrite(BPWM, 120);
    }
    // Si hay una curva hacia la izquierda pronunciada, gira hacia la izquierda más
    else if(cn5 == negro && cn6 == negro && cn7 == blanco && cn8 == blanco) {
      analogWrite(APWM, 60);
      analogWrite(BPWM, 140);
    }
    // Si hay una curva hacia la izquierda muy pronunciada, gira hacia la izquierda aún más
    else if(cn5 == negro && cn6 == blanco && cn7 == blanco && cn8 == blanco) {
      analogWrite(APWM, 60);
      analogWrite(BPWM, 140);
    }
    // Si los sensores del centro hay negro, puede seguir hacia delante
    else if(cn6 == negro && cn7 == negro) {
      analogWrite(APWM, 100);
      analogWrite(BPWM, 100);
    }
    // Si todos los sensores no detectan la línea, haremos que se pare
    else if(cn5 == blanco && cn6 == blanco && cn7 == blanco && cn8 == blanco){
      analogWrite(APWM, 0);
      analogWrite(BPWM, 0);
    }
    
    // Espera hasta la siguiente lectura
    delay(200);
  }

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

