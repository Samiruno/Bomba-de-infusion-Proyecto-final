const int dirPin = D1;
const int stepPin = D2;
const int stepsPerRevolution = 800;

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("Ingrese 1 para giro horario, 2 para giro antihorario, 0 para detener:");
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();

    if (command == '1') {
      Serial.println("Girando en sentido horario...");
      digitalWrite(dirPin, HIGH);
      spinMotor();
    } 
    else if (command == '2') {
      Serial.println("Girando en sentido antihorario...");
      digitalWrite(dirPin, LOW);
      spinMotor();
    }
    else if (command == '0') {
      Serial.println("Motor detenido");
      noSpinMotor();
    }
  }
}

void spinMotor() {
  for (int x = 0; x < stepsPerRevolution; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
}

void noSpinMotor() {
}
