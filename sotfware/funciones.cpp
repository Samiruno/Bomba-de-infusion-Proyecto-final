#include "../include/motor_display.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

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

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
bool infusing = false;
bool resetting = false;
int stepsRequired = 0;
long intervalMicroseconds = 0;
unsigned long lastStepTime = 0;
int stepsTaken = 0;

void setupPins() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
}

void setupDisplay() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
}

void setupMotor() {
    digitalWrite(DIR_PIN, LOW);
}

bool isButton1Pressed() {
    return digitalRead(BUTTON1_PIN) == LOW;
}

bool isButton2Pressed() {
    return digitalRead(BUTTON2_PIN) == LOW;
}

bool isButton3Pressed() {
    return digitalRead(BUTTON3_PIN) == LOW;
}

void displayWelcomeMessage() {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(5, 30);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.println("Bienvenido!");
    tft.setCursor(5, 70);
    tft.println("Sistema de Infusion");
    delay(3000);
    displayMainMenu();
}

void displayMainMenu() {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(5, 10);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.println("Opciones:");
    tft.setTextSize(1);
    tft.setCursor(5, 40);
    tft.println("1: Iniciar Infusion");
    tft.setCursor(5, 70);
    tft.println("2: Reiniciar Posicion");
    tft.setCursor(5, 100);
    tft.println("3: Volver al Menu");
}

void handleInfusionRequest() {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 20);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.println("Ingrese volumen (mL) y");
    tft.setCursor(5, 50);
    tft.println("tiempo (min) separado ','");
    tft.setCursor(5, 90);
    tft.setTextSize(2);
    tft.println("Ejemplo:2,30");

    String input = Serial.readStringUntil('\n');
    int commaIndex = input.indexOf(',');
    if (commaIndex != -1) {
        float mlRequested = input.substring(0, commaIndex).toFloat();
        int timeRequested = input.substring(commaIndex + 1).toInt();
        if (mlRequested > 0 && timeRequested > 0) {
            stepsRequired = calculateSteps(mlRequested);
            intervalMicroseconds = calculateInterval(stepsRequired, timeRequested);
            startInfusion(stepsRequired, intervalMicroseconds);
        } else {
            tft.fillScreen(ST7735_BLACK);
            tft.println("Entrada invalida.");
        }
    }
}

void startInfusion(int steps, long interval) {
    infusing = true;
    stepsTaken = 0;
    lastStepTime = millis();
    digitalWrite(DIR_PIN, HIGH);
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(5, 30);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(2);
    tft.println("Infusion en Progreso");
}

void stopCurrentOperation() {
    infusing = false;
    resetting = false;
    displayMainMenu();
}

void resetMotor() {
    resetting = true;
    stepsTaken = 0;
    digitalWrite(DIR_PIN, LOW);
    lastStepTime = millis();
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(5, 30);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(2);
    tft.println("Reiniciando Motor");
}

void handleInfusion() {
    if (infusing) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastStepTime >= intervalMicroseconds / 1000) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(STEP_PIN, LOW);
            lastStepTime = currentMillis;
            stepsTaken++;
            if (stepsTaken >= stepsRequired) {
                stopCurrentOperation();
            }
        }
    }
}

void handleReset() {
    if (resetting) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastStepTime >= intervalMicroseconds / 1000) {
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(10);
            digitalWrite(STEP_PIN, LOW);
            lastStepTime = currentMillis;
            stepsTaken++;
        }
    }
}

int calculateSteps(float ml) {
    return (int)((ml / mlPerRevolution) * stepsPerRevolution);
}

long calculateInterval(int steps, int timeMinutes) {
    return (long)((timeMinutes * 60000000L) / steps);
}
