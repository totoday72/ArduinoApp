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
const int IN1 = 26;
const int IN2 = 27;
int vm1 = 2;
const int IN3 = 28;
const int IN4 = 29;
int vm2 = 3;
//Sensor de peso
const int cargaDOUT = A0;
const int cargaSCK = A1;
const int pesominimo = 50;  //peso minimo en gramos
const float distanciaminima = 5;
//sensor ultrasonico
#define triggerUltrasonico 6  //pin trigger del ultrasonico
#define echoUltrasonico 5     //pin echo del ultrasonico
#define DEBUG true
#define WIFICONled 13 //LED is connected to Pin 11 of Arduino
#define AlertLed 27
String activado = "0";


//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************
HX711 scale;
WiFiEspClient net;
WiFiEspClient net2;
MQTTClient client;
SoftwareSerial WIFICON(11, 12); //Pin 11 & 12 of Arduino as tx and rx. Connect TX and RX of WIFICON respectively.
#define ESP_BAUDRATE  9600
unsigned long lastUploadedTime = 0;
//***************** INICIO DE DEFINICIONES DE OTRAS VARIABLES ****************************


//****************Variables a MANDAR***********************
unsigned long tiempoviaje = 0;
const int ida = 1; //cosntante que indica un viaje de ida
const int regreso = 2;//cosntante que indica un viaje de regreso
const int velocidadultrarapida = 255; //velocidad maxima para los motores
const int velocidadrapida = 200; //velocidad maxima para los motores
const int velocidadmedia = 160; // velocidad media para los motores
const int velocidadlenta = 140; // velocidad lenta para los motores
int ultimoladovistolinea = 0; // el lado en que se vio la linea por ultima vez
int ultimovalorvm1 = 0; //indica con que velocidad estaban los motores antes de detectar un obstaculo
int ultimovalorvm2 = 0;
int obstaculosEncontrados = 0; //cantidad de obstaculos encontrados de ida o de regreso
const int valorlineanegra = 0; //valor de la linea negra 1; negro 0 blanco
float peso = 0;//cosntante que dice cuanto es el peso
const int retraso = 1;
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

const unsigned long postingInterval = 16L * 1000L; // Post data every 20 seconds.
//***************** FIN DE DEFINICIONES DE VARIABLES PARA CONEXION ****************************

//***************** SETUP ****************************
//***************** SETUP ****************************
void setup() {
  // put your setup code here, to run once:
  pinMode(triggerUltrasonico, OUTPUT);
  pinMode(echoUltrasonico, INPUT);
  pinMode(lineaizq, INPUT);
  pinMode(lineacentro, INPUT);
  pinMode(lineader, INPUT);
  Serial.begin(9600);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);
  pinMode( vm1, OUTPUT);
  pinMode( vm2, OUTPUT);
  Serial.println("Iniciando ...");
  pinMode(WIFICONled, OUTPUT);
  digitalWrite(WIFICONled, 0);
  //iNICIALIZACIÓN DE CARGA
  scale.begin(cargaDOUT, cargaSCK);
  scale.set_scale(198837.f);                      // this estadodePuerta is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

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
  WiFi.begin(ssid2, pass); // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino. You need to set the IP address directly.
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to second SSID");
    WiFi.begin(ssid1, pass);
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
bool publicadoInicio = false;
void loop() {
  if(!publicadoInicio){
    publicarPuntoPartida("EnPuntodepartida:", "EnReposo", "-1", String(obstaculosEncontrados), "-1", "-1", "-1");
    publicadoInicio=true;
  }
  //client.loop();
  //hacerRecorrido(ida);
  //hacerRecorrido(regreso);
  peso = pesar();
  if (pesominimo < peso) {
    publicarEncamino("EnRecorrido:", "HaciaPuntoEntrega", "-1", "-1", String(peso), "-1", "-1");
    hacerRecorrido(ida);
    peso = pesar();
    while (peso > pesominimo) {
      Serial.println("Esperaaaaaandoooo");
      peso = pesar();
      digitalWrite(WIFICONled, 0);
      delay(200);
      digitalWrite(WIFICONled, 1);
      delay(200);
      digitalWrite(WIFICONled, 0);
      delay(200);
      digitalWrite(WIFICONled, 1)
      ; //mientras tenga peso no se mueve
    }
    publicarEncamino("EnRecorrido:", "RegresoaBuzon", "-1", "-1", "-1", "-1", "-1");
    hacerRecorrido(regreso);
  }
  /*LeerValor();
    //delay(postingInterval);  // <- fixes some issues with WIFICON stability
    if (!client.connected()) {
    connect();
    } else {
    if (activado == "1") { //pesominimo
      peso = pesar();
      Serial.print("Peso agregado es de ");
      Serial.print(peso);
      Serial.println("Gramos");
      if (pesominimo < peso) {
        tiempoviaje = millis();
        ////se detiene String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno
        publicarEncamino("EnRecorrido:", "HaciaPuntoEntrega", "-1", "-1", String(peso), "-1", "-1");
        hacerRecorrido(ida);
        while (peso > pesominimo) {
          peso = pesar();
          ; //mientras tenga peso no se mueve
        }
        publicarEncamino("EnRecorrido:", "RegresoaBuzon", "-1", "-1", "-1", "-1", "-1");
        hacerRecorrido(regreso);
      }
    }
    }
  */
  //LeerValor();
  delay(1000);
}


//***************** LOOP ****************************
//***************** LOOP ****************************
int accionanterior = -1;
void hacerRecorrido(int modo) { //const int lineaizq = 9; //const int lineacentro = 8; //const int lineader = 7;
  accionanterior = -1;
  int respuestasensor = verificarSensores();
  if (modo == regreso) {
    girarderecha(velocidadrapida);
    delay(1000);
    while (respuestasensor != 1) {
      girarderecha(velocidadrapida);
      respuestasensor = verificarSensores();
    }
  }
  while ( respuestasensor == 200) { //salir de zona de entrega o buzon
    avanzar(velocidadlenta); // se avanza por medio segundo
    respuestasensor = verificarSensores();
    Serial.println("Estado de sensores while ==200:" + String(respuestasensor));
  }
  while (respuestasensor != 1) { // se coloca el sensor central en la linea.
    girarderecha(velocidadlenta); //se busca la linea girando hacia la derecha hasta encontrarla
    respuestasensor = verificarSensores(); // se verifica que el sensor central este sobre la linea
    Serial.println("Estado de sensores While !=1:" + String(respuestasensor));
  }

  while (respuestasensor != 200) { //mientras no llegue la entrega o al buzon.
    respuestasensor = verificarSensores();
    if (respuestasensor == 0) { // si la linea se detecta con el sensor del lado izquierdo
      ultimoladovistolinea = 0;
      girarizquierda(velocidadrapida);
    } else if (respuestasensor == 2) {
      ultimoladovistolinea = 2;
      girarderecha(velocidadrapida);
    } else if (respuestasensor == 1) {
      ultimoladovistolinea = 1;
      avanzar(velocidadrapida);
    } else if (respuestasensor == -1) {
      if (ultimoladovistolinea == 0) {
        girarizquierda(velocidadrapida);
      } else if (ultimoladovistolinea == 2) {
        girarderecha(velocidadrapida);
      } else if (ultimoladovistolinea == 1) {
        avanzar(velocidadmedia);
      }
      //Serial.println("estoy en -1" + String(ultimoladovistolinea));
    }
    float distanciam = medirDistancia();
    if (distanciaminima > distanciam) { //si se detecta un objeto en el camino.
      detener(); //se detiene String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno
      publicarHayObstaculo("EnRecorrido:", "DetenidoObstaculo", "-1", "-1", "-1", "-1", "-1" ); //publica obstaculo falta como enviar la ubicacion
      obstaculosEncontrados++;

      while (distanciaminima > distanciam) { //mientras no se quite el obstaculo. no va a seguir
        Serial.println("obstaculos:" + String(obstaculosEncontrados));
        distanciam = medirDistancia();
        Serial.println("Distancia:" + String(distanciam) + "Distancia Min.:" + String(distanciaminima));
      }
      if (modo == ida) {
        Serial.println("publica en HaciaPuntoEntrega");
        publicarEncamino("EnRecorrido:", "HaciaPuntoEntrega", "-1", "-1", "-1", "-1", "-1");
      } else {
        Serial.println("regreso a buzon");
        publicarEncamino("EnRecorrido:", "RegresoaBuzon", "-1", "-1", "-1", "-1", "-1");
      }
      continuar(); // si ya no hay obstaculo continua solo encendiendo los motores
    }

    // verifica el estado de los sensores
    //Serial.println("Estado de sensores While!= 200:" + String(respuestasensor));
    detener();
    if (respuestasensor == 1) {
      detener();
      delay(43);//43
      
    } else {
      delay(40);
      
    }
    

    //respuestasensor = verificarSensores();

  }

  detener();//String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno
  if (modo == ida) {
    Serial.println("publica EnPuntoEntrega");
    publicarPuntoEntrega("EnPuntoEntrega:", "EnReposo", "1", String(obstaculosEncontrados), "-1", String(tiempoviaje / 1000), "-1"); //publica obstaculo falta como enviar la ubicacion
    peso = 0;
    obstaculosEncontrados = 0;
    tiempoviaje = 0;
  } else {
    Serial.println("publica EnPuntodepartida");
    publicarPuntoPartida("EnPuntodepartida:", "EnReposo", "-1", String(obstaculosEncontrados), "-1", "-1", String(tiempoviaje / 1000));
    peso = 0;
    obstaculosEncontrados = 0;
    tiempoviaje = 0;
  }

}

int verificarSensores() {
  int izq = digitalRead(lineaizq);
  int cnt = digitalRead(lineacentro);
  int der = digitalRead(lineader);
  //Serial.println("izq" + String(izq) + ":cnt" + String(cnt) + ":der" + String(der));
  //delay(500);
  if (izq == valorlineanegra && der == valorlineanegra && cnt == valorlineanegra) { //los tres tienen negro al mismo tiempo
    return 200; // indica que llego a punto de entrega
  }
  if (izq == valorlineanegra && cnt != valorlineanegra && der != valorlineanegra) { // la linea esta en el sensor izquierdo
    return 0;
  } else if (izq != valorlineanegra && cnt != valorlineanegra && der == valorlineanegra) { // la linea esta en el sensor derecho
    return 2;
  } else if ( izq != valorlineanegra && cnt == valorlineanegra && der != valorlineanegra) { // la linea esta en el sensor central
    return 1;
  } else if (izq == valorlineanegra && cnt == valorlineanegra && der != valorlineanegra) { //hay un cruce de 90 a la izq
    return 0;
  } else if (izq != valorlineanegra && cnt == valorlineanegra && der == valorlineanegra) { //hay un cruce de 90 a la deracha
    return 2;
  } else { // la linea no esta en ningun sensor, se procede a dar una vuelta para ver donde hay linea.
    return -1;
  }
  return -1;
}

void avanzar(int velocidad) {
  Serial.println("Avanzando:" + String(velocidad));
  ultimovalorvm1 = velocidad;
  ultimovalorvm2 = velocidad;
  adelante(velocidad);
}

void retroceder(int velocidad) {
  Serial.println("Retrocede:" + String(velocidad));
  ultimovalorvm1 = velocidad;
  ultimovalorvm2 = velocidad;
  retroceso(velocidad);
}

void girarderecha(int velocidad) {
  Serial.println("Gira derecga:" + String(velocidad));
  ultimovalorvm1 = velocidad;
  ultimovalorvm2 = velocidad;
  giroderecha(velocidad);
}

void girarizquierda(int velocidad) {
  Serial.println("Gira izquierda:" + String(velocidad));
  ultimovalorvm1 = velocidad;
  ultimovalorvm2 = velocidad;
  giroizquierda(velocidad);
}

// ******************** ACCIONES CON LOS MOTORES ***************************
// ******************** ACCIONES CON LOS MOTORES ***************************
void detener () {
  Serial.println("Detiene:");
  analogWrite(vm1, 0);
  analogWrite(vm2, 0);
}

void continuar() {
  analogWrite(vm1, ultimovalorvm1);
  analogWrite(vm2, ultimovalorvm2);
}

void adelante(int velocidad) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(vm1, velocidad);
  analogWrite(vm2, velocidad);

}

void retroceso (int velocidad) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  analogWrite(vm2, velocidad);
  analogWrite(vm1, velocidad);
}

void giroizquierda(int velocidad) {
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(vm2, velocidad);
  analogWrite(vm1, velocidad);
}
void giroderecha(int velocidad) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  analogWrite(vm2, velocidad);
  analogWrite(vm1, velocidad);
}

void motorizquierda(int velocidad) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(vm2, velocidad);
  analogWrite(vm1, 0);
}

void motorderecha(int velocidad) {
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);
  analogWrite(vm2, 0);
  analogWrite(vm1, velocidad);
}
// ******************** ACCIONES CON LOS MOTORES ***************************
// ******************** ACCIONES CON LOS MOTORES ***************************

/***************** MENSAJES ****************************
  field1  "ubicacion"
  field2  "estado_vehiculo"
  field3  "paquetes"
  field4  "obstaculos"
  field5  "peso"
  field6  "t_entrega"
  field7  "t_retorno"*/

void publicarPuntoPartida(String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno) {
  boolean result = false;
volverapublicar1:
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  connect();
  String dataText = String("field1=" + ubicacion + "&field2=" + estado + "&field3=" + paquetes + "&field4=" + obstaculos + "&field5=" + peso );// + "&field6=" + tiempoentrega );//+ "&field7=" + tiemporetorno);
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  result = client.publish(publishTopic, dataText);
  if (!result) {
    Serial.println("Erroro publicando");
    goto volverapublicar1;

  }
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  return result;
}

void publicarPuntoEntrega(String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno) {
  boolean result = false;
volverapublicar2:
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  connect();
  String dataText = String("field1=" + ubicacion + "&field2=" + estado + "&field3=" + paquetes + "&field4=" + obstaculos + "&field5=" + peso );// + "&field6=" + tiempoentrega );//+ "&field7=" + tiemporetorno);
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  result = client.publish(publishTopic, dataText);
  if (!result) {
    Serial.println("Erroro publicando");
    goto volverapublicar2;
  }
  if (result) Serial.println("Data has been published succesfully");
  else Serial.println("Unable to publish data");
  lastUploadedTime = millis();
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  return result;
}
void publicarEncamino(String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno) {
  tiempoentrega = "data";
  boolean result = false;
volverapublicar3:

  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  connect();
  String dataText = String("field1=" + ubicacion + "&field2=" + estado + "&field3=" + paquetes + "&field4=" + obstaculos + "&field5=" + peso );// + "&field6=" + tiempoentrega );//+ "&field7=" + tiemporetorno);
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  result = client.publish(publishTopic, dataText);
  if (!result) {
    Serial.println(String (tiempoentrega));
    Serial.println("Erroro publicando");
    goto volverapublicar3;
  }
  if (result) Serial.println("Data has been published succesfully");
  else Serial.println("Unable to publish data");

  lastUploadedTime = millis();
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  return result;
}

boolean publicarHayObstaculo(String ubicacion, String estado, String paquetes, String obstaculos, String peso, String tiempoentrega, String tiemporetorno) {
  boolean result = false;
volverapublicar4:
  while (millis() - lastUploadedTime < postingInterval) { // The uploading interval must be > 15 seconds
    ; // no hace nada hasta que sean mas de 15 segundos para publicar
  }
  connect();
  digitalWrite(WIFICONled, 1);
  String dataText = String("field1=" + ubicacion + "&field2=" + estado + "&field3=" + paquetes + "&field4=" + obstaculos + "&field5=" + peso );// + "&field6=" + tiempoentrega );//+ "&field7=" + tiemporetorno);
  //String dataText = String("field1=" + String(sensorValue_1)+ "&field2=" + String(sensorValue_2)+"&field3=" + String(sensorValue_3)); // example for publish tree sensors
  result = client.publish(publishTopic, dataText);
  if (!result) {
    Serial.println("Erroro publicando");
    goto volverapublicar4;
  }
  if (result) Serial.println("Data has been published succesfully");
  else Serial.println("Unable to publish data");

  lastUploadedTime = millis();
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  delay(50);
  digitalWrite(AlertLed, 1);
  delay(150);
  digitalWrite(AlertLed, 0);
  return result;
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
  float peso = (scale.get_units(10) * 454);// - 327.17;
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
float medirDistancia() {
  long duracion;
  float distancia = 0;
  digitalWrite(triggerUltrasonico, LOW);
  delayMicroseconds(5);
  digitalWrite(triggerUltrasonico, HIGH);
  //delayMicroseconds(1000);
  delayMicroseconds(10);
  digitalWrite(triggerUltrasonico, LOW);
  duracion = pulseIn(echoUltrasonico, HIGH);
  distancia = duracion * 0.01768115942 ;
  Serial.print ("Distancia en Promedio: ");
  Serial.println(distancia);
  return distancia;
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
    delay(2000);
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
