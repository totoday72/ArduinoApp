const int seguidorLinea = 7;
int IN1 = 52;
int IN2 = 53;
int vm1=2;
int IN3 = 50;
int IN4 = 51;
int vm2=3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);
  pinMode( vm1, OUTPUT);
  pinMode( vm2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(digitalRead(seguidorLinea));
  int c =digitalRead(seguidorLinea);
 analogWrite(vm1,254);
 analogWrite(vm2,254);
 delay(500);
  if((c == 1 )) 
  {
    analogWrite(vm1,255);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  }
  else if(c == 0)
  {
     analogWrite(vm2,255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  }
}
