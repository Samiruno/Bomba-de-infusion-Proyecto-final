#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_DC 12
#define TFT_CS 13
#define TFT_MOSI 14
#define TFT_CLK 15
#define TFT_RST 0
#define TFT_MISO -1

#define BUTTON1_PIN 16
#define BUTTON2_PIN 0
#define BUTTON3_PIN 2

#define DIR_PIN D1
#define STEP_PIN D2

const int stepsPerRevolution = 1000;
const float mlPerRevolution = 0.5;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

bool infusing = false;
bool awaitingInput = false;
int stepsRequired = 0;
float mlRequested = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  tft.initR(INITR_BLACKTAB);
  displayMainMenu();
}

void loop() {
  if (digitalRead(BUTTON1_PIN) == LOW && !infusing) {
    requestInfusionVolume();
  }

  if (digitalRead(BUTTON2_PIN) == LOW && infusing) {
    stopInfusion();
  }

  if (digitalRead(BUTTON3_PIN) == LOW) {
    displayMainMenu();
  }

  if (awaitingInput && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    mlRequested = input.toFloat();

    if (mlRequested > 0) {
      stepsRequired = calculateSteps(mlRequested);
      startInfusion(mlRequested, stepsRequired);
    } else {
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(5, 50);
      tft.println("Valor invalido. Reintente.");
      delay(2000);
      requestInfusionVolume();
    }
  }
}

void displayMainMenu() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Bienvenido!");
  tft.setCursor(5, 50);
  tft.println("Que deseas realizar?");
  tft.setCursor(5, 70);
  tft.println("1: Iniciar Infusion");
  tft.setCursor(5, 90);
  tft.println("2: Detener Infusion");
  tft.setCursor(5, 110);
  tft.println("3: Volver al Menu");
  awaitingInput = false;
  infusing = false;
}

void requestInfusionVolume() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Ingrese volumen en mL:");
  tft.setCursor(5, 50);
  tft.println("Esperando entrada...");
  delay(2000);
  awaitingInput = true;
  infusing = false;
}

void startInfusion(float mlRequested, int steps) {
  Serial.print("Iniciando infusión de ");
  Serial.print(mlRequested);
  Serial.println(" mL...");
  infusing = true;
  awaitingInput = false;
  digitalWrite(DIR_PIN, HIGH);
  spinMotor(steps);
  Serial.println("Infusión completada.");
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 50);
  tft.println("Infusion completada.");
  delay(2000);
  displayMainMenu();
}

void stopInfusion() {
  Serial.println("Infusion detenida.");
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 50);
  tft.println("Infusion detenida.");
  infusing = false;
  delay(2000);
  displayMainMenu();
}

void spinMotor(int steps) {
  for (int x = 0; x < steps; x++) {
    if (digitalRead(BUTTON2_PIN) == LOW) {
      break;
    }
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(5000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
}

int calculateSteps(float ml) {
  return (int)(ml * stepsPerRevolution / mlPerRevolution);
}

