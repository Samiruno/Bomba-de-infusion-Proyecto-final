#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_DC 12
#define TFT_CS 13
#define TFT_MOSI 14
#define TFT_CLK 15
#define TFT_RST 0 
#define TFT_MISO -1

#define BUTTON1_PIN 4
#define BUTTON2_PIN 0
#define BUTTON3_PIN 2

#define DIR_PIN D0
#define STEP_PIN D1

const int stepsPerRevolution = 200;
const float mlPerRevolution = 0.5;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

bool infusing = false;
bool awaitingInput = false;
int stepsRequired = 0;
long intervalMicroseconds = 0;
float mlRequested = 0;
int timeRequested = 0;
unsigned long startTime = 0;
unsigned long lastStepTime = 0;
int stepsTaken = 0;
bool infusionMessageDisplayed = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  tft.initR(INITR_BLACKTAB);
  displayWelcomeMessage();
}

void loop() {
  if (digitalRead(BUTTON1_PIN) == LOW && !infusing) {
    requestInfusionParameters();
  }
  if (digitalRead(BUTTON2_PIN) == LOW && infusing) {
    stopInfusion();
  }
  if (digitalRead(BUTTON3_PIN) == LOW) {
    displayMainMenu();
  }
  if (awaitingInput && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    if (processUserInput(input)) {
      stepsRequired = calculateSteps(mlRequested);
      intervalMicroseconds = calculateInterval(stepsRequired, timeRequested);
      Serial.print("Volumen solicitado: ");
      Serial.print(mlRequested);
      Serial.println(" mL");
      Serial.print("Tiempo solicitado: ");
      Serial.print(timeRequested);
      Serial.println(" minutos");
      Serial.print("Pasos calculados: ");
      Serial.println(stepsRequired);
      Serial.print("Intervalo entre pasos (us): ");
      Serial.println(intervalMicroseconds);
      startInfusion(stepsRequired, intervalMicroseconds);
    } else {
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(5, 50);
      tft.println("Entrada inválida. Intente de nuevo.");
      delay(2000);
      requestInfusionParameters();
    }
  }
  if (infusing) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastStepTime >= intervalMicroseconds / 1000) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(STEP_PIN, LOW);
      lastStepTime = currentMillis;
      stepsTaken++;
      if (stepsTaken >= stepsRequired) {
        stopInfusion();
      }
    }
    if (currentMillis - startTime >= (timeRequested * 60000L)) {
      stopInfusion();
    }
  }
}

void displayWelcomeMessage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.println("Bienvenido!");
  tft.setTextSize(1);
  tft.setCursor(5, 60);
  tft.println("Sistema de Infusión");
  delay(3000);
  displayMainMenu();
}

void displayMainMenu() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Opciones:");
  tft.setCursor(5, 50);
  tft.println("1: Iniciar Infusión");
  tft.setCursor(5, 70);
  tft.println("2: Detener Infusión");
  tft.setCursor(5, 90);
  tft.println("3: Volver al Menú");
  awaitingInput = false;
  infusing = false;
}

void requestInfusionParameters() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Ingrese volumen (mL) y");
  tft.setCursor(5, 50);
  tft.println("tiempo (min) separado por ',':");
  tft.setCursor(5, 70);
  tft.println("Ejemplo: 2,30");
  delay(2000);
  awaitingInput = true;
  infusing = false;
}

bool processUserInput(String input) {
  int commaIndex = input.indexOf(',');
  if (commaIndex != -1) {
    mlRequested = input.substring(0, commaIndex).toFloat();
    timeRequested = input.substring(commaIndex + 1).toInt();
    return (mlRequested > 0 && timeRequested > 0);
  }
  return false;
}

void startInfusion(int steps, long interval) {
  Serial.println("Iniciando infusión...");
  infusing = true;
  stepsTaken = 0;
  startTime = millis();
  lastStepTime = startTime;
  digitalWrite(DIR_PIN, HIGH);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("NO TOCAR");
  tft.setCursor(5, 60);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("REALIZANDO INFUSION");
  infusionMessageDisplayed = true;
}

void stopInfusion() {
  Serial.println("Infusión detenida.");
  tft.fillScreen(ST7735_BLACK);
  infusionMessageDisplayed = false;
  displayMainMenu();
}

int calculateSteps(float ml) {
  return (int)(ml * stepsPerRevolution / mlPerRevolution);
}

long calculateInterval(int steps, int time) {
  long totalTime = (long)time * 60000L;
  return steps > 0 ? totalTime * 1000L / steps : 1000000L;
}
