const int inputPin = 2;
const int motor = 53;
int value = 0;
#define trigPin 8
#define echoPin 7
#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = A0;
const int LOADCELL_SCK_PIN = A1;
const float depositonivelalto =10;
const float depositonivelmedio = 35;
const float depositonivelbajo = 55;
const float depositourgente=65;
float niveldeposito=0;
HX711 scale;

void setup() {
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, 1);

  //sensor de distancia
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  //iNICIALIZACIÓN DE CARGA
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(198837.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  //scale.tare();               // reset the scale to 0
}

void loop() {
  value = digitalRead(inputPin);  //lectura digital de pin

  //mandar mensaje a puerto serie en función del valor leido
  if (value == HIGH) {
Inicio:
    Serial.println("Puerta Abierta");
    while (value == HIGH) {
      value = digitalRead(inputPin);
    }
    Serial.println("Puerta Cerrada");
    Serial.println("Esperando a desinfectar");
    float peso = pesar();
    Serial.println(peso);
    if (peso > 10) {
      if (medirDistancia() < depositourgente) {
        desinfectar();
      }
    } else {
      Serial.println("No hay objeto");
    }
  }
  else {
    Serial.println("En espera");
    value = digitalRead(inputPin);
    if (value == HIGH) {
      Serial.println("ir a inicio");
      goto Inicio;
    }
  }
  delay(200);
}

float pesar() {
  Serial.print("average:\t");
  float peso = (scale.get_units(10) * 454) - 327.17;
  Serial.print(abs(peso), 0);
  Serial.println(" Gramos");
  scale.power_down();              // put the ADC in sleep mode
  delay(100);
  scale.power_up();
  return abs(peso);
}


void desinfectar() {
  Serial.println("Desinfectando");
  digitalWrite(motor, LOW);
  delay(5000);
  digitalWrite(motor, HIGH);
}

float medirDistancia() {
  long duracion;
  float distancia = 0;
  float distanciapromedio = 0;
  for (int i = 0; i < 100; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(1);
    digitalWrite(trigPin, HIGH);
    //delayMicroseconds(1000);
    delayMicroseconds(4);
    digitalWrite(trigPin, LOW);
    duracion = pulseIn(echoPin, HIGH);
    distancia = (duracion / 2) / 29.1;
    distanciapromedio += distancia;
    //Serial.println(distancia);
  }
  distanciapromedio /= 100;
  if (distanciapromedio <= depositonivelalto)
  {
    niveldeposito = distanciapromedio;
  }
  else if (distanciapromedio <= depositonivelmedio) {
    niveldeposito = distanciapromedio;
  } else if (distanciapromedio <= depositonivelbajo) {
    //avisarUsuario();
    niveldeposito = distanciapromedio;
  }
  Serial.print ("Distancia en Promedio: ");
  Serial.println(distanciapromedio);
  return distanciapromedio;

}
