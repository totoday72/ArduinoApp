const int inputPin = 2;
const int motor= 53;
int value = 0;
#define trigPin 8
#define echoPin 7
void setup() {
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(motor,OUTPUT);
  digitalWrite(motor,1);

  //sensor de distancia
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}
 
void loop(){
  value = digitalRead(inputPin);  //lectura digital de pin
 
  //mandar mensaje a puerto serie en función del valor leido
  if (value == HIGH) {
      Serial.println("Puerta Abierta");
      while(value==HIGH){
        value = digitalRead(inputPin);
      }
      Serial.println("Puerta Cerrada");
      Serial.println("Desinfectando");
      digitalWrite(motor,LOW);
      delay(5000);
      digitalWrite(motor,HIGH);
  }
  else {
      Serial.println("En espera");
      medirDistancia();
  }
  delay(200);
}



void medirDistancia(){
  long duration;
  float distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  //delayMicroseconds(1000);
  delayMicroseconds(4);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
    //if(distance > 255)
    //distance = 255;
  
  Serial.println(distance);

}
