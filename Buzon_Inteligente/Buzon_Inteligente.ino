//Arquitectura de Computadoras y Ensambladores 2
//Grupo 16
// 2do Semestre 2020
//***************** INCLUDES DE LIBRERIAS ****************************
#include "HX711.h"
#include <SoftwareSerial.h>
//********************FINAL DE INCLUDES ******************************

//***************** INICIO DE VARIABLES GLOBALES ****************************
const int puerta = 2; //pin de final de carrera para detectar estado de puerta
const int motor = 53; //pin para habilitar bomba de agua
int estadodePuerta = 0;        //VARIABLE para verificar el estado de puerta
const int cargaDOUT = A0;
const int cargaSCK = A1;
const float depositonivelalto = 7;
const float depositonivelmedio = 15;
const float depositonivelbajo = 23;
const float depositourgente = 30;
float niveldeposito = 0;
String strDato;
unsigned long tiempo;
unsigned long tiempoderepeticion = 3600; //tiempo para verificar deposito (segundos)
const int pesominimo=10;    //peso minimo en gramos

//***************** INCLUDES VARIABLES GLOBALES ****************************

//***************** INICIO DE DEFINICIONES ****************************
#define triggerUltrasonico 8  //pin trigger del ultrasonico
#define echoUltrasonico 7     //pin echo del ultrasonico
#define DEBUG true
#define led_pin 13 //LED is connected to Pin 11 of Arduino
//********************FINAL DE DEFINICIONES ******************************


//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************
HX711 scale;
SoftwareSerial wifi(12, 11); //Pin 2 & 3 of Arduino as tx and rx. Connect TX and RX of wifi respectively.


//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************

//***************** INICIO SETUP ****************************
void setup() {
  tiempo = millis();
  Serial.begin(9600);
  Serial.println("Iniciando ...");
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(puerta, INPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, 1);

  //sensor de distancia
  pinMode(triggerUltrasonico, OUTPUT);
  pinMode(echoUltrasonico, INPUT);


  //iNICIALIZACIÓN DE CARGA
  scale.begin(cargaDOUT, cargaSCK);
  scale.set_scale(198837.f);                      // this estadodePuerta is obtained by calibrating the scale with known weights; see the README for details
  //scale.tare();               // reset the scale to 0

  //********************** SWICH ON WIFI COMUNICATION**********************************************
  wifi.begin(9600); //Baud rate for communicating with wifi. Your's might be different.
  wifiSerial("AT+RST\r\n", 5000, DEBUG); // Reset the wifi
  //wifiSerial("AT+CWMODE=3\r\n", 5000, DEBUG); //Set station mode Operation
  wifiSerial("AT+CWJAP=\"sdfsadfasdfasdfasdfasdfasdfasdfa\",\"KKBGVVJK\"\r\n", 5000, DEBUG);//Enter your WiFi network's SSID and Password.

  while (!wifi.find("OK"))
  {
  }
  digitalWrite(led_pin, HIGH);
  wifiSerial("AT+CIFSR\r\n", 5000, DEBUG);//You will get the IP Address of the wifi from this command.
  wifiSerial("AT+CIPMUX=1\r\n", 5000, DEBUG);
  //wifiSerial("AT+CIPSERVER=1,80\r\n", 5000, DEBUG);
  //********************** SWICH ON WIFI COMUNICATION**********************************************
}
//***************** FIN SETUP ****************************

//***************** INICIO LOOP ****************************
void loop() {
  estadodePuerta = digitalRead(puerta);  //lectura digital de pin

  //mandar mensaje a puerto serie en función del valor leido
  if (estadodePuerta == HIGH) {
Inicio:
    Serial.println("Puerta Abierta");
    while (estadodePuerta == HIGH) {
      estadodePuerta = digitalRead(puerta);
    }
    Serial.println("Puerta Cerrada");
    Serial.println("Esperando a desinfectar");
    float peso = pesar();
    Serial.println(peso);
    if (peso > pesominimo) {
      float distanciaactualmedida = medirDistancia(0);
      if (distanciaactualmedida < depositourgente) {
        desinfectar();
        enviarDatos(String(peso, 3), String(distanciaactualmedida, 3), "PaqueteDesinfectado ");
      } else {
        enviarDatos(String(peso, 3), String(distanciaactualmedida, 3), "PaqueteNoDesinfectado ");
      }
    } else {
      Serial.println("No hay objeto");
    }
  }
  else {
    estadodePuerta = digitalRead(puerta);
    if (estadodePuerta == HIGH) {
      Serial.println("ir a inicio");
      goto Inicio;
    }
  }
  unsigned long resta = (millis() / 1000) - (tiempo / 1000);
  //Serial.println(resta);
  if (resta > tiempoderepeticion) {
    medirDistancia(1);
    tiempo = millis();
    Serial.println("SE PUSO A VERIFICAR LA DISTANCIA");
  }
}
//***************** FINAL LOOP ****************************

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




float medirDistancia(int modo) {
  long duracion;
  float distancia = 0;
  float distanciapromedio = 0;
  digitalWrite(triggerUltrasonico, LOW);
  delayMicroseconds(5);
  digitalWrite(triggerUltrasonico, HIGH);
  //delayMicroseconds(1000);
  delayMicroseconds(10);
  digitalWrite(triggerUltrasonico, LOW);
  duracion = pulseIn(echoUltrasonico, HIGH);
  distancia = duracion * 0.01768115942 ;
  distanciapromedio += distancia;
  Serial.println(distancia);
  Serial.print ("Distancia en Promedio: ");
  Serial.println(distanciapromedio);
  if (modo == 1) {
    if (distanciapromedio < depositonivelbajo) {
      enviarDatos("00", String(distanciapromedio, 3), "AlertaDeposito ");
    } else if (distanciapromedio > depositourgente) {
      enviarDatos("00", String(distanciapromedio, 3), "AlertaDeposito ");
    }
  }

  return distanciapromedio;

}


String wifiSerial(String command, const int timeout, boolean debug)
{
  String response = "";
  wifi.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (wifi.available())
    {
      char c = wifi.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}

void enviarDatos(String peso, String distancia, String advertencia) {
  Serial.println("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80");
  wifi.println("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80");
  if (wifi.find("OK")) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }

  int tampeso = peso.length();
  int tamdistancia = distancia.length();

  //Serial.println("El tamaño de peso es:" + String(tampeso) + " ** tamaño de distancia es:" + String(tamdistancia));

  //Serial.println("El  peso es:" + (peso) + " **la distancia es:" + (distancia));

  char header[] = "GET /update?api_key=";
  char apiKey[] = "8NCUJ7OGZ0KS1Q5F";//"01NC44KTO6OTZWV5"; //api key del proyecto
  char host[] = " HTTP/1.1\r\nHost: 184.106.153.149\r\nConnection: close\r\n\r\n";
  char field1[] = "&field1=";
  char field2[] = "&field2=";
  char field3[] = "&field3=";
  char field4[] = "&field4=";
  int tamaniopeticion = strlen(header) + strlen(apiKey) + strlen(host) + strlen(field1) + strlen(field2) + strlen(field3) + strlen(field3) + tampeso + tamdistancia + advertencia.length();
  Serial.println("tamaño de peticion es:" + String(tamaniopeticion));

  char url[tamaniopeticion] = "";
  strcat(url, header);
  strcat(url, apiKey);

  strcat(url, field1);
  char dato[tampeso];
  peso.toCharArray(dato, tampeso);
  strcat(url, dato);
  Serial.print("el dato1 es:");
  Serial.println(dato);

  strcat(url, field2);
  char dato2[tamdistancia];
  distancia.toCharArray(dato2, tamdistancia);
  strcat(url, dato2);
  Serial.print("el dato2 es:");
  Serial.print(dato2);
  Serial.println(strlen(dato2));

  strcat(url, field3);
  char dato3[advertencia.length()];
  advertencia.toCharArray(dato3, advertencia.length());
  strcat(url, dato3);
  //Serial.print("el dato3 es:");
  //Serial.print(advertencia);
  //Serial.print(dato3);
  //Serial.println(strlen(dato3));

  strcat(url, field4);
  String niveldedeposito = "";
  float tempdistancia = distancia.toFloat();
  if (tempdistancia < depositonivelalto) {
    niveldedeposito = "DepositoAlto ";
  } else if (tempdistancia < depositonivelmedio) {
    niveldedeposito = "DepositoMedio ";
  } else if (tempdistancia < depositonivelbajo) {
    niveldedeposito = "DepositoBajo ";
  }
  else {
    niveldedeposito = "DepositoMuyBajo ";
  }
  char dato4[niveldedeposito.length()];
  niveldedeposito.toCharArray(dato4, niveldedeposito.length());
  strcat(url, dato4);

  strcat(url, host);
  Serial.println(url);
  int tam = strlen(url);
  Serial.println(tam);
  String strTam;
  strTam = + tam;
  //wifiSerial("AT+CIPSEND=4," + strTam+"\r\n", 5000, DEBUG);
  Serial.println("AT+CIPSEND=4," + strTam);
  wifi.println("AT+CIPSEND=4," + strTam);
  if (wifi.find(">")) {
    Serial.print(">");
    Serial.print(url);
    wifi.print(url);
  } else {
    Serial.print("ERROR");
  }
  strTam = "";
  strDato = "";
  //delay(15000);
}
