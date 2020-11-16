
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h> // para habilitar el WIFICON emulando rx y tx
//#include <ESP8266WIFI.h>  // para ESP8266 habilitar esta linea
//#include <WIFICON.h>           // para shield ESP32 habilitar esta linea
#include "WiFiEsp.h"
#include <virtuabotixRTC.h> //Libreria
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
const int rxmp3 = 11;
const int txmp3 = 12;
SoftwareSerial mySoftwareSerial(rxmp3, txmp3); // RX, TX

//*******************TECLADO *****************
#include "Adafruit_Keypad.h"

const byte ROWS = 4; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {36, 34, 32, 30}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {28, 26, 24, 22};  //22 AZUL MARCADO CON 1 connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Adafruit_Keypad teclado = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2
//*****************WIFI**************************
//#ifndef HAVE_HWSERIAL1
//SoftwareSerial wifiserial(5, 4); // TX, RX
//#endif
//#define ESP_BAUDRATE  9600
char ssid[] = "sdfsadfasdfasdfasdfasdfasdfasdfa";            // your network SSID (name)
char passw[] = "KKBGVVJK";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "api.taskycodes.com";
String fecha = "";
String hora = "";
// Initialize the Ethernet client object
WiFiEspClient client;
//*****************TECLADO**************************

#define DEBUG true
#define wifiled 13 //LED is connected to Pin 11 of Arduino
DFRobotDFPlayerMini mp3;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//sensor ultrasonico
#define triggerUltrasonico 6  //pin trigger del ultrasonico
#define echoUltrasonico 7     //pin echo del ultrasonico
#define motor 3
#define puerta 2
#define uv 4
const int switchpuerta = A0;
//*****************RTC**************************
virtuabotixRTC myRTC(8, 9, 10);// CLK -> 6, DAT -> 7, RST -> 8
void setup() {
  pinMode(switchpuerta, INPUT);
  pinMode(uv, OUTPUT);
  digitalWrite(uv, LOW);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, HIGH);
  pinMode(puerta, OUTPUT);
  digitalWrite(puerta, HIGH);
  pinMode(wifiled, OUTPUT);
  digitalWrite(wifiled, LOW);
  Serial.begin(9600);
  mlx.begin();
  mySoftwareSerial.begin(9600);
  teclado.begin();
  Serial1.begin(9600);
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Cargando FIRMWARE!.");
  lcd.setCursor(0, 1);
  lcd.print("Espere....");
  pinMode(triggerUltrasonico, OUTPUT);
  pinMode(echoUltrasonico, INPUT);
  //pinMode(rxmp3, INPUT);
  //pinMode(txmp3, OUTPUT);


  mp3.begin(mySoftwareSerial);   //Use softwareSerial to communicate with mp3.
  mp3.setTimeOut(1000); //Set serial communictaion time out 500ms
  mp3.reset();
  //----Set volume----
  mp3.volume(25);  //Set volume value (0~30).
  //mp3.volumeUp(); //Volume Up
  mp3.volumeDown(); //Volume Down
  //mp3.EQ(DFPLAYER_EQ_ROCK);
  //mp3.outputDevice(DFPLAYER_DEVICE_SD);
  //myDFPlayer.play(18);  //Play the first mp3
  //myDFPlayer.randomAll(); //Random play all the mp3.
  //myDFPlayer.advertise(3);
  lcd.setCursor(0, 1);
  lcd.print("iniciando wifi");

  //********************** SWICH ON WIFI COMUNICATION**********************************************
  WiFi.init(&Serial1); //en arduino mega se utiliza el serial 1 no se usa otros pines

  /*check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue

    } else {
    lcd.setCursor(0, 1);
    lcd.print("Wifi conectado....");
    }
  */
  //myRTC.setDS1302Time(00, 35, 14, 3, 11, 11, 2020); //// (segundos, minutos, hora, dia da semana, dia del mes, mes, año)::: SS, MM, HH, DW, DD, MM, YYYY
  conectarWifi(1);
  myRTC.updateTime();
  fecha =  String(myRTC.year) + "-" + String(myRTC.month) + "-" + String(myRTC.dayofmonth);
  //String hora = "15:10:13";
  Serial.println("Fecha: " + fecha);
  hora = String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds);
  Serial.println("hora: " + hora);


  //********************** SWICH ON WIFI COMUNICATION**********************************************
}

int tiemponumeros = 950;
const int tempambiente = 2;
const int tempcorporal = 1;
const int distanciaminima = 5;
const int distanciaminimausuario =7;
const int temperaturamaxima = 37; //temperatura en la cual es usuario ya no puede entrar

//**************************LOOOOP**************************
//**************************LOOOOP**************************
long timetoseeclock = 0;
boolean sonido = true;
void loop() {
  char caracter = leerTeclado();
  if (caracter == '#' || caracter == '*') {
    sonido = !sonido;
    lcd.setCursor(0, 1);
    lcd.print("sonido:" + String(sonido) + "                 ");
    delay(1000);
  }
inicio:
  float distancia = medirDistancia();
  if (distancia < distanciaminimausuario) {
    if (sonido) {
      mp3.play(49); // dice "ingrese sus pertenencias"
      delay(3000);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("coloque petenencias            ");
    }
    desinfectarTelefono(1000);
    delay(500);
    desinfectarTelefono(1000);
    String usuario =  ingresarUsuario();
    int contador = 0;
    if (usuario != "") {
      int temperatura = 0;
TomarTemperatura:
      long tiempo = millis();
      distancia = medirDistancia();
      while (distanciaminima < distancia) {
        distancia = medirDistancia();
        lcd.setCursor(0, 0);
        lcd.print("Acerquese a 3 CM        ");
        lcd.setCursor(0, 1);
        lcd.print("Actual: " + String(int(distancia)) + " cm     ");
        if ((millis() - tiempo) > 8000) {
          tiempo = millis();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Presione * o # para      ");
          lcd.setCursor(0, 1);
          lcd.print("cancelar la operacion ");
          if (sonido) {
            mp3.play(46); // dice que se acerque al sensor
            delay(3000);
          }
        }
        char caracter = leerTeclado();
        if (caracter == '#' || caracter == '*') {
          goto inicio;
        }
      }
      temperatura = decirTemperatura(tempcorporal); //temperatura corporal 1 y !=1 temperatura ambiente
      contador++;
      if (temperatura > temperaturamaxima) {
        switch (contador) {
          case 1:
            if (sonido) {
              mp3.play(18); //advertencia 1 de temperatura >36
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Temp. >37");
              lcd.setCursor(0, 1);
              lcd.print("Se repetira medir");
            }
            break;
          case 2:
            if (sonido) {
              mp3.play(19); //advertencia 2 de temperatura >36
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Temp. >37");
              lcd.setCursor(0, 1);
              lcd.print("Se repetira medir");
            }
            break;
          case 3:
            if (sonido) {
              mp3.play(20); //advertencia 3 de temperatura >36
            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Temp. >37");
              lcd.setCursor(0, 1);
              lcd.print("No Ingreso");
            }
            break;
        }
        if (sonido) {
          delay(13000);
        } else {
          delay(3000);
        }
        if (contador < 3) { //si los intentos son 1 o 2 se vuelve a tomar temperatura de ultimo la 3
          for (int i = 59; i >= 0; i--) {
            if (i <= 15) {
              if (sonido) {
                reproducirSonidoNumero(i, i, false);
              } else {
                delay(1000);
              }
            }
            else if (i < 20) {
              if (sonido) {
                reproducirSonidoNumero(i - 10, i, false);
              } else {
                delay(1000);
              }
            } else {
              delay(1000);
            } /*else if (i < 30) {
              reproducirSonidoNumero(i - 20, i, false);
            } else if (i < 40) {
              reproducirSonidoNumero(i - 30, i, false);
            } else if (i < 50) {
              reproducirSonidoNumero(i - 40, i, false);
            } else if (i < 60) {
              reproducirSonidoNumero(i - 50, i, false);
            }
*/
            lcd.setCursor(0, 0);
            lcd.print("Prueba " + String(contador + 1) + " en:" + String(i) + " S         ");
            lcd.setCursor(0, 1);
            lcd.print("T=" + String(temperatura) + "'C            ");
            char caracter = leerTeclado();
            if (caracter == '#' || caracter == '*') {
              int temperaturaAmbiente = int(mlx.readAmbientTempC());
              reportar(temperatura, temperaturaAmbiente, usuario);
              goto inicio;
            }
            if ((millis() - tiempo) > 8000) {
              tiempo = millis();
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Presione * o # para      ");
              lcd.setCursor(0, 1);
              lcd.print("cancelar la operacion      ");
              delay(100);
            }

          }
          goto TomarTemperatura;
        } else { // si en caso es la tercera no abrira la puerta y enviara los datos a al api
          int temperaturaAmbiente = int(mlx.readAmbientTempC());
          reportar(temperatura, temperaturaAmbiente, usuario);
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Usuario: " + usuario);
        lcd.setCursor(0, 1);
        lcd.print("No puede ingresar!");
        desinfectarPuerta();
        desinfectarTelefono(6000);
        if (sonido) {
          mp3.play(50); // dice "retire sus pertenencias"
          delay(3000);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Usuario: " + usuario);
          lcd.setCursor(0, 1);
          lcd.print("tome pertenencias!");
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Usuario: " + usuario);
        lcd.setCursor(0, 1);
        lcd.print("Temp: " + String(temperatura) + "'");
        int temperaturaAmbiente = int(mlx.readAmbientTempC());
        reportar(temperatura, temperaturaAmbiente, usuario);
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Usuario: " + usuario);
        lcd.setCursor(0, 0);
        lcd.print("¡BIENVENIDO!");
        desinfectarTelefono(6000);
        if (sonido) {
          mp3.play(47); // dice "aBRA LA PURETA"
          delay(2000);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Usuario: " + usuario);
          lcd.setCursor(0, 1);
          lcd.print("abriendo puerta!");
          delay(1000);
        }
        abrirPuerta();
        if (sonido) {
          mp3.play(52); // dice "desinfecte sus manos"
          delay(2000);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Usuario: " + usuario);
          lcd.setCursor(0, 1);
          lcd.print("Desinf. sus manos!");
        }
        desinfectarPuerta();
        if (sonido) {
          mp3.play(50); // dice "recoger sus pertenencias"
          delay(3000);

        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Usuario: " + usuario);
          lcd.setCursor(0, 1);
          lcd.print("tome pertenencias!");
          delay(2000);

        }
        verificarPuertaCerrada();
        desinfectarPuerta();

      }
    } else {
      decirTemperatura(tempambiente);
    }
    distancia = medirDistancia();
    while (distanciaminima > distancia) {
      delay(200);
      distancia = medirDistancia();
    }
    delay(5000);
  }
  lcd.setCursor(0, 0);
  lcd.print("Medicion Lista!        ");
  lcd.setCursor(0, 1);
  long resta = millis() - timetoseeclock;
  if (resta < 5000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Presion * o # para           ");
    lcd.setCursor(0, 1);
    lcd.print("Sonido:" + String(sonido) + "          ");
  } else if (resta < 20000) {
    lcd.print("D: " + String(int(distancia)) + " cm                ");
  } else if (millis() - timetoseeclock < 30000) {
    myRTC.updateTime();
    hora = String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds);
    lcd.print("Hora: " + hora + "           ");
  } else {
    timetoseeclock = millis();
  }
}
//**************************LOOOOP**************************
//**************************LOOOOP**************************
void verificarPuertaCerrada() {
  while (digitalRead(switchpuerta) == HIGH) {
    mp3.play(51); // dice "CIERRE LA PURETA"
    delay(5000);
    Serial.println("puerta estado:" + digitalRead(switchpuerta));
  }
}


void reportar(int temperaturaCorporal, int temperaturaAmbiente, String usuario) {
  myRTC.updateTime();
  String fecha =  String(myRTC.year) + "-" + String(myRTC.month) + "-" + String(myRTC.dayofmonth);
  //String hora = "15:10:13";
  Serial.println("Fecha: " + fecha);
  String hora = String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds);
  Serial.println("hora: " + hora);
  enviarDatos(temperaturaCorporal,  temperaturaAmbiente,  usuario,  fecha,  hora);
}


void abrirPuerta() {

  digitalWrite(puerta, LOW);
  delay(750);
  digitalWrite(puerta, HIGH);
}
void desinfectarTelefono(int tiempo) {
  digitalWrite(uv, HIGH);
  delay(tiempo);
  digitalWrite(uv, LOW);
}
void desinfectarPuerta() {

  digitalWrite(motor, LOW);
  delay(1000);
  digitalWrite(motor, HIGH);
}
char leerTeclado() {
  char caracter = 'x';
  teclado.tick();
  if (teclado.available()) {
    keypadEvent e = teclado.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) {
      caracter = (char)e.bit.KEY;
    }
  }
  return caracter;
}

//***************** INGRESAR USUARIO ****************************
String ingresarUsuario() {
  if (sonido) {
    mp3.play(28);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Usuario: ");
  lcd.setCursor(0, 1);
  lcd.print("N:");
  char caracter = 'x';
  String cadena = "";
  boolean seguro = false;
  while (!seguro) {
    caracter = 'x';
    cadena = "";
    while (caracter != '*') {
      teclado.tick();
      if (teclado.available()) {
        keypadEvent e = teclado.read();
        if (e.bit.EVENT == KEY_JUST_PRESSED) {
          caracter = (char)e.bit.KEY;
          if (caracter == '#' && cadena.length() == 0) {
            return "";
          }
          if (caracter == '#') {
            if (cadena.length() > 0) {
              cadena = cadena.substring(0, cadena.length() - 1);
            }
          } else if (caracter != '*' ) {
            if (caracter != '#') {
              cadena += caracter;
            }
          }
        } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
          if (sonido) {
            sonidotecla(caracter);
          }
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Usuario: ");
          lcd.setCursor(0, 1);
          lcd.print("N:" + cadena);
        }
      }

    }

    caracter = 'x';
    lcd.setCursor(0, 0);
    lcd.print("*Confirmar #Cancelar: ");
    if (sonido) {
      mp3.play(35);
      delay(tiemponumeros);
      for (int i = 0; i < cadena.length(); i++) {
        sonidotecla(cadena.charAt(i));
      }
      mp3.play(21);
      delay(tiemponumeros);
    }
    while (caracter != '*') {
      teclado.tick();
      if (teclado.available()) {
        keypadEvent e = teclado.read();
        if (e.bit.EVENT == KEY_JUST_PRESSED) {
          caracter = (char)e.bit.KEY;
          if (caracter == '*') {
            seguro = true;
          } else if (caracter == '#') {
            caracter = 'x';
            cadena = "";
            if (sonido) {
              mp3.play(28);
            }
            break;
          }
          Serial.println(" pressed");
        } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
          if (sonido) {
            sonidotecla(caracter);
          }
        }
      }
    }
  }
  return cadena;
}

//***************** INGRESAR USUARIO ****************************


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
  //Serial.print ("Distancia en Promedio: ");
  //Serial.println(distancia);
  delay(80);
  return distancia;
}

//***************** DISTANCIA ****************************
//***************** DISTANCIA ****************************

//***************** SONIDO ****************************
//***************** SONIDO ****************************

void sonidotecla(char tecla) {
  switch (tecla) {
    case '1':
      mp3.play(40);
      break;
    case '2':
      mp3.play(17);
      break;
    case '3':
      mp3.play(39);
      break;
    case '4':
      mp3.play(12);
      break;
    case '5':
      mp3.play(9);
      break;
    case '6':
      mp3.play(31);
      break;
    case '7':
      mp3.play(32);
      break;
    case '8':
      mp3.play(26);
      break;
    case '9':
      mp3.play(24);
      break;
    case '0':
      mp3.play(7);
      break;
    case '*':
      mp3.play(3);
      break;
    case '#':
      mp3.play(25);
      break;
    case 'A':
      mp3.play(2);
      break;
    case 'B':
      mp3.play(4);
      break;
    case 'C':
      mp3.play(5);
      break;
    case 'D':
      mp3.play(13);
      break;
  }
  delay(tiemponumeros);

}

int decirTemperatura(int modo) {
  int tempObjeto = 0;
  if (modo == 1) {
    if (sonido) {
      mp3.play(33);
      delay(2000);
    }
    tempObjeto = int(mlx.readObjectTempC());
  } else {
    if (sonido) {
      mp3.play(36);
      delay(2000);
    }
    tempObjeto = int(mlx.readAmbientTempC());
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  if (modo == tempcorporal) {
    lcd.print("Su Temperatura: ");
  } else {
    lcd.print("Temperatura Ambiente: ");
  }
  lcd.setCursor(0, 1);
  lcd.print("T=" + String(tempObjeto) + " 'C");
  if (sonido) {
    if (tempObjeto <= 15) {
      reproducirSonidoNumero(tempObjeto, tempObjeto, true);
    } else if (tempObjeto < 20) {
      reproducirSonidoNumero(tempObjeto - 10, tempObjeto, true);
    } else if (tempObjeto < 30) {
      reproducirSonidoNumero(tempObjeto - 20, tempObjeto, true);
    } else if (tempObjeto < 40) {
      reproducirSonidoNumero(tempObjeto - 30, tempObjeto, true);
    } else if (tempObjeto < 50) {
      reproducirSonidoNumero(tempObjeto - 40, tempObjeto, true);
    } else if (tempObjeto < 60) {
      reproducirSonidoNumero(tempObjeto - 50, tempObjeto, true);
    } else {
      mp3.play(14);
    }
    delay(2000);
  }
  return tempObjeto;
}

void reproducirSonidoNumero(int sonido, int numero, boolean decirgrados) {
  if (sonido == 0) {
    if (numero == 20) {
      mp3.play(45);
    } else if (numero == 30) {
      mp3.play(44);
    } else if (numero == 40) {
      mp3.play(43);
    } else if (numero == 50) {
      mp3.play(42);
    }
  } else {
    if (numero < 16) {
      // no hay sonido
    } else if (numero < 20 ) {
      mp3.play(14);
      delay(tiemponumeros);
    } else if (numero < 30) {
      mp3.play(41);
      delay(tiemponumeros);
    } else if (numero < 40) {
      mp3.play(38);
      delay(tiemponumeros);
    } else if (numero < 50) {
      mp3.play(11);
      delay(tiemponumeros);
    } else if (numero < 60) {
      mp3.play(10);
      delay(tiemponumeros);
    }
    switch (sonido) {
      case 0:
        mp3.play(7);
        break;
      case 1:
        mp3.play(40);
        break;
      case 2:
        mp3.play(17);
        break;
      case 3:
        mp3.play(39);
        break;
      case 4:
        mp3.play(12);
        break;
      case 5:
        mp3.play(9);
        break;
      case 6:
        mp3.play(31);
        break;
      case 7:
        mp3.play(32);
        break;
      case 8:
        mp3.play(26);
        break;
      case 9:
        mp3.play(24);
        break;
      case 10:
        mp3.play(15);
        break;
      case 11:
        mp3.play(27);
        break;
      case 12:
        mp3.play(16);
        break;
      case 13:
        mp3.play(37);
        break;
      case 14:
        mp3.play(6);
        break;
      case 15:
        mp3.play(30);
        break;
    }
  }
  delay(tiemponumeros - 100);
  if (decirgrados) {
    mp3.play(22);
  }
}

//***************** comunicacion ****************************
String response = "";
boolean enviarDatos(int temperaturaCorporal, int temperaturaAmbiente, String usuario, String fecha, String hora) {
  conectarWifi(0);
  // if you get a connection, report back via serial
  if (client.connect(server, 80)) {
    Serial.println("Connected to server");
    // Make a HTTP request
    client.println("GET /guardarproyecto2?usuario=" + usuario + "&fecha=" + fecha + "&hora=" + hora + "&temperaturaCorporal=" + String(temperaturaCorporal) + "&temperaturaAmbiente=" + String(temperaturaAmbiente) + " HTTP/1.1");
    client.println("Host: api.taskycodes.com");
    client.println("Connection: close");
    client.println();
    while (client.available()) {
      char c = client.read();
      response += c;
    }
    Serial.println("Disconnecting from server...");
    client.flush();
    client.stop();
    Serial.println(response);
    return true;
  } else {
    return false;
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:" + String(ip));
  lcd.setCursor(0, 1);
  lcd.print("S:" + String(rssi) + "dBm");
  delay(4500);
}
void conectarWifi(int tipo) {
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, passw);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wifi!.");
    lcd.setCursor(0, 1);
    lcd.print("R:Conectando");
  }
  if (tipo == 1) {
    Serial.println("You're connected to the network");
    printWifiStatus();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wifi conect!.");
    lcd.setCursor(0, 1);
    lcd.print("R:" + String(ssid));
    delay(3000);
  }
}
//***************** COMUNICACION ****************************
