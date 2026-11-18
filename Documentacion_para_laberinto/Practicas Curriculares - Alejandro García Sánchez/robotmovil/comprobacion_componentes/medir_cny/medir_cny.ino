// Programa que mide los valores de los sensores ópticos CNY

void setup() { 
  Serial.begin(230400); 
  Serial.println("--- PROGRAMA DE CALIBRACIÓN ---");
  delay(3000);
}

void loop() {
  // Lectura de pines analógicos según el PinOut del ATtiny816
  int CNY1 = analogRead(4);
  int CNY2 = analogRead(3); 
  int CNY3 = analogRead(2);
  int CNY4 = analogRead(1);

  // Enviamos los datos en formato de columnas para copiar a Excel si quieres
  Serial.print("CNY1: "); Serial.print(CNY1);
  Serial.print("\tCNY2: "); Serial.print(CNY2);
  Serial.print("\tCNY3: "); Serial.print(CNY3);
  Serial.print("\tCNY4: "); Serial.println(CNY4);

  delay(300); 
}