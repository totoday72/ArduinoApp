
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
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
//*****************TECLADO**************************



DFRobotDFPlayerMini mp3;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//sensor ultrasonico
#define triggerUltrasonico 6  //pin trigger del ultrasonico
#define echoUltrasonico 7     //pin echo del ultrasonico

void setup() {
  Serial.begin(9600);
  teclado.begin();
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Cargando FIRMWARE!.");
  lcd.setCursor(0, 1);
  lcd.print("Espere....");
  pinMode(triggerUltrasonico, OUTPUT);
  pinMode(echoUltrasonico, INPUT);
  pinMode(rxmp3, INPUT);
  pinMode(txmp3, OUTPUT);
  pinMode(4, INPUT);
  mlx.begin();
  mySoftwareSerial.begin(9600);
  mp3.begin(mySoftwareSerial);   //Use softwareSerial to communicate with mp3.
  mp3.setTimeOut(500); //Set serial communictaion time out 500ms
  mp3.reset();
  //----Set volume----
  mp3.volume(30);  //Set volume value (0~30).
  //mp3.volumeUp(); //Volume Up
  //mp3.volumeDown(); //Volume Down
  mp3.EQ(DFPLAYER_EQ_ROCK);
  mp3.outputDevice(DFPLAYER_DEVICE_SD);
  //myDFPlayer.play(18);  //Play the first mp3
  //myDFPlayer.randomAll(); //Random play all the mp3.
  //myDFPlayer.advertise(3);
}
int tiemponumeros = 900;
const int tempambiente = 2;
const int tempcorporal = 1;
const int distanciaminima = 5;
const int temperaturamaxima = 37; //temperatura en la cual es usuario ya no puede entrar

//**************************LOOOOP**************************
//**************************LOOOOP**************************
void loop() {
inicio:
  float distancia = medirDistancia();
  if (distancia < distanciaminima) {
    String Usuario = ingresarUsuario();
    delay(100);
    int contador = 0;
    if (Usuario != "") {
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
          mp3.play(46); // dice que se acerque al sensor
          delay(3000);
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
            mp3.play(18); //advertencia 1 de temperatura >36
            break;
          case 2:
            mp3.play(19); //advertencia 2 de temperatura >36
            break;
          case 3:
            mp3.play(20); //advertencia 3 de temperatura >36
            break;
        }
        delay(13000);
        if (contador < 3) { //si los intentos son 1 o 2 se vuelve a tomar temperatura de ultimo la 3
          for (int i = 59; i >= 0; i--) {
            if (i <= 15) {
              reproducirSonidoNumero(i, i, false);
            } else if (i < 20) {
              reproducirSonidoNumero(i - 10, i, false);
            } else if (i < 30) {
              reproducirSonidoNumero(i - 20, i, false);
            } else if (i < 40) {
              reproducirSonidoNumero(i - 30, i, false);
            } else if (i < 50) {
              reproducirSonidoNumero(i - 40, i, false);
            } else if (i < 60) {
              reproducirSonidoNumero(i - 50, i, false);
            }
            lcd.setCursor(0, 0);
            lcd.print("Prueba " + String(contador+1) + " en:" + String(i) + " S         ");
            lcd.setCursor(0, 1);
            lcd.print("T=" + String(temperatura) + "'C            ");
            char caracter = leerTeclado();
            if (caracter == '#' || caracter == '*') {
              reportarEnfermo(Usuario, temperatura);
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
            delay(60);
          }
          goto TomarTemperatura;
        } else { // si en caso es la tercera no abrira la puerta y enviara los datos a al api
          reportarEnfermo(Usuario, temperatura);
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Usuario: " + Usuario);
        lcd.setCursor(0, 1);
        lcd.print("Temp: " + String(temperatura) + "'");
        enviarDatosAPI(Usuario, temperatura);
        abrirPuerta();
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
  lcd.print("D: " + String(int(distancia)) + " cm     ");
  //delay(1000);

}
//**************************LOOOOP**************************
//**************************LOOOOP**************************
void reportarEnfermo(String Usuario, int temperatura) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Usuario: " + Usuario);
  lcd.setCursor(0, 1);
  lcd.print("No puede ingresar");
  enviarDatosAPI(Usuario, temperatura);
}


void abrirPuerta() {
  Serial.println("Se abrira la puerta!");
}

void enviarDatosAPI(String usuario, int temperatura) {
  Serial.println("Se enviara a API!, Usuario:" + usuario + " temp:" + String(temperatura));
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
  mp3.play(28);
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
          sonidotecla(caracter);
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
    lcd.print("Confirmar Usuario: ");
    mp3.play(35);
    delay(tiemponumeros);
    for (int i = 0; i < cadena.length(); i++) {
      sonidotecla(cadena.charAt(i));
    }
    mp3.play(21);
    delay(tiemponumeros);
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
            mp3.play(28);
            break;
          }
          Serial.println(" pressed");
        } else if (e.bit.EVENT == KEY_JUST_RELEASED) {
          sonidotecla(caracter);
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
  Serial.print ("Distancia en Promedio: ");
  Serial.println(distancia);
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
    mp3.play(33);
    delay(2000);
    tempObjeto = int(mlx.readObjectTempC());
  } else {
    mp3.play(36);
    delay(2000);
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

//***************** SONIDO ****************************
//***************** SONIDO ****************************
