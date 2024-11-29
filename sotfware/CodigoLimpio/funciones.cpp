#include "functions.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <FreeRTOS.h>

// Pines de la pantalla y motor
#define TFT_DC 12
#define TFT_CS 13
#define TFT_MOSI 14
#define TFT_CLK 15
#define TFT_RST 0 
#define BUTTON1_PIN 4
#define BUTTON2_PIN 0
#define BUTTON3_PIN 2
#define DIR_PIN D0
#define STEP_PIN D1

// Configuración de motor
const int stepsPerRevolution = 200;
const float mlPerRevolution = 0.5;

// Variables globales
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
SemaphoreHandle_t infusionSemaphore;
SemaphoreHandle_t resetSemaphore;

// Funciones de inicialización
void initDisplay() {
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
}

void initButtons() {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
}

void initMotor() {
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
}

// Funciones de visualización
void displayWelcomeMessage() {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(5, 30);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.println("Bienvenido!");
    tft.setTextSize(2);
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

// Funciones del motor
void moveMotor(int steps, bool direction, int interval) {
    digitalWrite(DIR_PIN, direction);
    for (int i = 0; i < steps; i++) {
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(interval);
    }
}

int calculateSteps(float ml) {
    return (int)((ml / mlPerRevolution) * stepsPerRevolution);
}

long calculateInterval(int steps, int timeMinutes) {
    return (long)((timeMinutes * 60000000L) / steps);
}

// Configuración de tareas FreeRTOS
void setupTasks() {
    infusionSemaphore = xSemaphoreCreateBinary();
    resetSemaphore = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(handleInfusionTask, "InfusionTask", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(handleResetTask, "ResetTask", 2048, NULL, 1, NULL, 1);
}

void handleInfusionTask(void *parameter) {
    for (;;) {
        if (xSemaphoreTake(infusionSemaphore, portMAX_DELAY)) {
            int steps = calculateSteps(2.0); // Ejemplo: 2 mL
            moveMotor(steps, true, calculateInterval(steps, 1)); // Ejemplo: 1 minuto
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Evitar watchdog
    }
}

void handleResetTask(void *parameter) {
    for (;;) {
        if (xSemaphoreTake(resetSemaphore, portMAX_DELAY)) {
            moveMotor(200, false, 500); // Movimiento de reinicio
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
