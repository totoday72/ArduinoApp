#include "HX711.h" // sensor de peso
#include <SoftwareSerial.h> // para habilitar el WIFICON emulando rx y tx
//#include <ESP8266WIFI.h>  // para ESP8266 habilitar esta linea
//#include <WIFICON.h>           // para shield ESP32 habilitar esta linea
#include "WiFiEsp.h" // para ESP8266 con arduino Nano o Mega
#include <MQTTClient.h> // para conexion MQTT
#include "ThingSpeak.h"
// sensores de linea
const int lineaizq = 9;
const int lineacentro = 8;
const int lineader = 7;

// Salidas de  motor
const int IN1 = 52;
const int IN2 = 53;
int vm1 = 2;
const int IN3 = 50;
const int IN4 = 51;
int vm2 = 3;
//Sensor de peso
const int cargaDOUT = A0;
const int cargaSCK = A1;
const int pesominimo = 50;  //peso minimo en gramos
const float distanciaminima = 15.00f;
//sensor ultrasonico
#define triggerUltrasonico 6  //pin trigger del ultrasonico
#define echoUltrasonico 5     //pin echo del ultrasonico
#define DEBUG true
#define WIFICONled 13 //LED is connected to Pin 11 of Arduino
#define AlertLed 27
String activado = "0";


//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************
HX711 scale;
unsigned long tiempo;
WiFiEspClient net;
WiFiEspClient net2;
MQTTClient client;
SoftwareSerial WIFICON(11, 12); //Pin 11 & 12 of Arduino as tx and rx. Connect TX and RX of WIFICON respectively.
#define ESP_BAUDRATE  9600
unsigned long lastUploadedTime = 0;
//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************


//****************Variables a MANDAR***********************
unsigned long tiempodeentrega=0;
float pesopaquete =0;

//****************Variables a MANDAR***********************

//***************** INICIO DE DEFINICIONES DE VARIABLES PARA CONEXION ****************************
char ssid2[] = "sdfsadfasdfasdfasdfasdfasdfasdfa"; //  Nombre de red WIFICON, SSID (name).
char ssid1[] = "AndroidAP";
char pass[] = "KKBGVVJK";    // contraseña de WIFICON

//------ MQTT broker settings and topics
/*
const char* broker = "mqtt.thingspeak.com"; //Server para mqtt app data
char mqttUserName[] = "Totoday72";       // Nombre de usuario (puede ser cualqauiera)
char mqttPass[] = "EV0EFZGZ6PL29HBH";   // colocar el codigo MQTT API Key desde Account > MyProfile.

//-- published settings
char publish_writeAPIKey[] = "P6CW3EZ9W7HS2MHA";// api key para escribir en el canal
long publishChannelID = 1129609;       // numero de canal donde se va a guardar los datos
String publishTopic = "channels/" + String( publishChannelID ) + "/publish/" + String(publish_writeAPIKey);   // no cambiar

//-- subscribed settings Virtuino command 1
long suscribeChannelID_1 = 1117297; // numero de canal donde se van a leeer los datos que se guardaran con la app movil
String subscribeTopicFor_Command_1 = "channels/" + String(suscribeChannelID_1) + "/subscribe/fields/field1"; // que campo se leer autoaticamente
//String subscribeTopicFor_Command_2="channels/"+String(suscribeChannelID_1)+"/subscribe/fields/field2";
//-- subscribed settings Virtuino command 2
//long suscribeChannelID_2 = 123456;
//String subscribeTopicFor_Command_2="channels/"+String(suscribeChannelID_2)+"/subscribe/fields/field1";   // motor

const unsigned long postingInterval = 20L * 1000L; // Post data every 20 seconds.
*/

const char* broker = "mqtt.thingspeak.com"; //Server para mqtt app data
char mqttUserName[] = "Totoday72";       // Nombre de usuario (puede ser cualqauiera)
char mqttPass[] = "RXRMKFZ36GMDVRGB";   // colocar el codigo MQTT API Key desde Account > MyProfile.

//-- published settings
char publish_writeAPIKey[] = "8NCUJ7OGZ0KS1Q5F";// api key para escribir en el canal
long publishChannelID = 1117472;       // numero de canal donde se va a guardar los datos
String publishTopic = "channels/" + String( publishChannelID ) + "/publish/" + String(publish_writeAPIKey);   // no cambiar

//-- subscribed settings Virtuino command 1
long suscribeChannelID_1 = 1135982; // numero de canal donde se van a leeer los datos que se guardaran con la app movil
String subscribeTopicFor_Command_1 = "channels/" + String(suscribeChannelID_1) + "/subscribe/fields/field1"; // que campo se leer autoaticamente
//String subscribeTopicFor_Command_2="channels/"+String(suscribeChannelID_1)+"/subscribe/fields/field2";
//-- subscribed settings Virtuino command 2
//long suscribeChannelID_2 = 123456;
//String subscribeTopicFor_Command_2="channels/"+String(suscribeChannelID_2)+"/subscribe/fields/field1";   // motor

const unsigned long postingInterval = 20L * 1000L; // Post data every 20 seconds.
//***************** FIN DE DEFINICIONES DE VARIABLES PARA CONEXION ****************************

//***************** SETUP ****************************
//***************** SETUP ****************************
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);
  pinMode( vm1, OUTPUT);
  pinMode( vm2, OUTPUT);
  tiempo = millis();
  Serial.println("Iniciando ...");
  pinMode(WIFICONled, OUTPUT);
  digitalWrite(WIFICONled, 0);
  //iNICIALIZACIÓN DE CARGA
  scale.begin(cargaDOUT, cargaSCK);
  scale.set_scale(198837.f);                      // this estadodePuerta is obtained by calibrating the scale with known weights; see the README for details
  //scale.tare();               // reset the scale to 0

  setEspBaudRate(ESP_BAUDRATE);
  WIFICON.print("AT+RST\r\n");

  /*
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo native USB port only
    }
  */
  Serial.print("Searching for ESP8266...");
  // initialize ESP module
  WiFi.init(&WIFICON);
  Serial.print("Searching for ESP8266...");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WIFICON shield not present");
    // don't continue
    while (true);
  }
  //wifiSerial("AT+CWJAP =\"abc\", \"0123456789\"",800);
  Serial.println("found it!");
  WiFi.begin(ssid1, pass); // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino. You need to set the IP address directly.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to second SSID");
    WiFi.begin(ssid2, pass);
    delay(500);
  }

  ThingSpeak.begin(net2);
  client.begin(broker, net);
  //client.onMessage(messageReceived);
  connect();
  Serial.println("Cliente conectado");
}
//***************** SETUP ****************************
//***************** SETUP ****************************

//***************** LOOP ****************************
//***************** LOOP ****************************
void loop() {
  //client.loop();
  //LeerValor();
  //delay(postingInterval);  // <- fixes some issues with WIFICON stability
  if (!client.connected()) {
    connect();
  } else {
    if (activado == "1") { //pesominimo
      float peso = pesar();
      Serial.print("Peso agregado es de ");
      Serial.print(peso);
      Serial.println("Gramos");
      if (peso > pesominimo) {
        //publicarEncamino();
        //hacerRecorrido();
        if (millis() - lastUploadedTime > postingInterval) { // The uploading interval must be > 15 seconds
          float sensorValue_1 = peso; // replace with your sensor value
          //int sensorValue_2 = analogRead(A0); // replace with your sensor value
          //int sensorValue_3=random(100);  // if you want to use three sensors enable this line

          String dataText = String("field1=" + String(sensorValue_1)); //+ "&field2=" + String(sensorValue_2));
          //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
          boolean result = client.publish(publishTopic, dataText);

          if (result) Serial.println("Data has been published succesfully");
          else Serial.println("Unable to publish data");

          lastUploadedTime = millis();
        }
      }
    }
  }
  LeerValor();
  //publicarInicio();
  delay(6000);
  /*
    client.loop();
    if (millis() - lastUploadedTime > postingInterval) { // The uploading interval must be > 15 seconds
    int sensorValue_1 = analogRead(A0); // replace with your sensor value
    int sensorValue_2= analogRead(A0);  // replace with your sensor value
    //int sensorValue_3=random(100);  // if you want to use three sensors enable this line

    String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2));
    //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
    boolean result = client.publish(publishTopic, dataText);

    if (result) Serial.println("Data has been published succesfully");
    else Serial.println("Unable to publish data");

    lastUploadedTime = millis();
    }
  */
}
//***************** LOOP ****************************
//***************** LOOP ****************************

void hacerRecorrido() {
  
}

//***************** MENSAJES ****************************
void publicarInicio(float peso, long tiempodeviaje,int cantidaddeObjetos,String estado,String mensaje) {
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  String dataText = String("field1=" + String(peso)); //+ "&field2=" + String(sensorValue_2));
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  boolean result = client.publish(publishTopic, dataText);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
}

void publicarEntrega(float peso, long tiempodeviaje,int cantidaddeObjetos,String estado,String mensaje) {
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  String dataText = String("field1=" + String(peso)); //+ "&field2=" + String(sensorValue_2));
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  boolean result = client.publish(publishTopic, dataText);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
}
void publicarEncamino(float peso, long tiempodeviaje,int cantidaddeObjetos,String estado,String mensaje) {
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  String dataText = String("field1=" + String(peso)); //+ "&field2=" + String(sensorValue_2));
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  boolean result = client.publish(publishTopic, dataText);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
}

void publicarHayObstaculo(float peso, long tiempodeviaje,int cantidaddeObjetos,String estado,String mensaje) {
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  String dataText = String("field1=" + String(peso)); //+ "&field2=" + String(sensorValue_2));
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  boolean result = client.publish(publishTopic, dataText);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
}
//***************** MENSAJES ****************************


/****************** lECTURAS NO SIRVEN ****************************
String wifiSerial(String command, const int timeout)
{
  String response = "";
  WIFICON.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (WIFICON.available())
    {
      char c = WIFICON.read();
      response += c;
    }
  }

  Serial.print(response);

  return response;
}

void datosver() {
  const char host[] = "api.thingspeak.com";

  WiFiEspClient client;

  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/channels/1117297/fields/1.json?results=1";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  String channelData = "";
  while (client.available()) {
    channelData += char(client.read());

    // do something with the data

  }
  Serial.println(channelData);
}


void enviarDatos() {
  Serial.println("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80");
  WIFICON.println("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80");
  if (WIFICON.find("OK")) {
    Serial.println("OK");
  } else {
    Serial.println("ERROR");
  }

  char header[] = "GET /channels/1117297/fields/1.json?results=1";
  char host[] = " HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\n\r\n";
  int tamaniopeticion = strlen(header) + strlen(host);
  Serial.println("tamaño de peticion es:" + String(tamaniopeticion));
  char url[tamaniopeticion] = "";
  strcat(url, header);
  strcat(url, host);
  Serial.println(url);
  int tam = strlen(url);
  Serial.println(tam);
  String strTam;
  strTam = + tam;
  //wifiSerial("AT+CIPSEND=4," + strTam+"\r\n", 5000);
  Serial.println("AT+CIPSEND=4," + strTam);
  WIFICON.println("AT+CIPSEND=4," + strTam);

  if (WIFICON.find(">")) {
    Serial.print(">");
    Serial.print(url);
    WIFICON.print(url);
  } else {
    Serial.print("ERROR");
  }

  String channelData = "";
  while (WIFICON.available()) {
    channelData += char(WIFICON.read());
  }
  Serial.println(channelData);
  //delay(15000);

}
//***************** lECTURAS NO SIRVEN ****************************/

//***************** LECTURA DE CAMPO PARA ACTIVACION ****************************
//***************** LECTURA DE CAMPO PARA ACTIVACION ****************************
void LeerValor() {
  int dato = (ThingSpeak.readIntField(suscribeChannelID_1, 1));
  // Check the status of the read operation to see if it was successful
  int statusCode = ThingSpeak.getLastReadStatus();
  if (statusCode == 200) {
    activado = String(dato);
    Serial.println("Valor de la variable: " + String(activado) + " ---- ");
  }
  else {
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode));
  }
}
//***************** LECTURA DE CAMPO PARA ACTIVACION ****************************
//***************** LECTURA DE CAMPO PARA ACTIVACION ****************************

//***************** PESAR ****************************
//***************** PESAR ****************************
float pesar() {
  float peso = (scale.get_units(10) * 454) - 327.17;
  Serial.print("Peso Minimo para activar el sistema: ");
  Serial.print(pesominimo);
  Serial.println(" Gramos");
  Serial.print("Peso detectado:");
  Serial.print(abs(peso), 0);
  Serial.println(" Gramos");
  scale.power_down();              // put the ADC in sleep mode
  delay(100);
  scale.power_up();
  return abs(peso);
}
//***************** PESAR ****************************
//***************** PESAR ****************************

//***************** DISTANCIA ****************************
//***************** DISTANCIA ****************************
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
  if (distanciapromedio < distanciaminima) {
    //detener();
    //publicarHayObstaculo();
  }
  return distanciapromedio;
}

//***************** DISTANCIA ****************************
//***************** DISTANCIA ****************************

//***************** WIFICON ****************************
//***************** WIFICON ****************************
void setEspBaudRate(unsigned long baudrate) {
  long rates[6] = {115200, 74880, 57600, 38400, 19200, 9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for (int i = 0; i < 6; i++) {
    WIFICON.begin(rates[i]);
    delay(100);
    WIFICON.print("AT+UART_DEF=");
    WIFICON.print(baudrate);
    WIFICON.print(",8,1,0,0\r\n");
    delay(100);
  }
  WIFICON.begin(baudrate);
}


void connect() {
  Serial.print("\nConectando a WIFI.\nEsperando\n");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("WIFICON Conectado.\n");
  //--- create a random MQTT client id
  char clientID[] = "ESP8266_0000000000"; // For random generation of client ID.
  for (int i = 8; i < 18 ; i++) clientID[i] =  char(48 + random(10));
  Serial.print("\nconnecting to broker...");
  while (!client.connect(clientID, mqttUserName, mqttPass)) {
    Serial.print(".");
    digitalWrite(WIFICONled, 0);
    delay(5000);
  }
  Serial.println("\nMQTT broker nconnected!");
  digitalWrite(WIFICONled, 1);
  //client.subscribe(subscribeTopicFor_Command_1);
  //client.subscribe(subscribeTopicFor_Command_2);

}
//***************** WIFICON ****************************
//***************** WIFICON ****************************

//========================================= messageReceived
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  activado = payload;
  Serial.print("El circuito esta:");
  Serial.println(activado);
  /*
    //-- check for Virtuino Command 1
    if (topic==subscribeTopicFor_Command_1){
         Serial.println("Command 1 = "+payload);
         int v = payload.toInt();
         if (v>0) digitalWrite(LED_BUILTIN,HIGH);
         else digitalWrite(LED_BUILTIN,LOW);
    }

    -- check for Virtuino Command 1
    if (topic==subscribeTopicFor_Command_2){
         Serial.println("Command 2 = "+payload);
    }
  */

}



void detener () {
  analogWrite(vm1, 0);
  analogWrite(vm2, 0);
}

void adelante(int velocidad) {
  analogWrite(vm1, velocidad);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

}

void retroceso (int velocidad) {
  analogWrite(vm2, velocidad);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
