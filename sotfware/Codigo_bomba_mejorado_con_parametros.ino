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

// Variables globales
volatile bool infusing = false;
volatile bool resetting = false;
int stepsRequired = 0;
long intervalMicroseconds = 0;
int stepsTaken = 0;
unsigned long lastStepTime = 0;

// Semáforos para sincronización
SemaphoreHandle_t infusionSemaphore;
SemaphoreHandle_t resetSemaphore;

// Prototipos de funciones
void displayWelcomeMessage();
void displayMainMenu();
void requestInfusionParameters();
void handleInfusion(void *parameter);
void handleReset(void *parameter);
void buttonTask(void *parameter);
bool processUserInput(String input);
int calculateSteps(float ml);
long calculateInterval(int steps, int timeMinutes);
void startInfusion(int steps, long interval);
void stopInfusion();
void displayError(String message);

void setup() {
  Serial.begin(115200);

  // Configuración de pines
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  // Inicializar pantalla
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  displayWelcomeMessage();

  // Crear semáforos
  infusionSemaphore = xSemaphoreCreateBinary();
  resetSemaphore = xSemaphoreCreateBinary();

  // Crear tareas
  xTaskCreate(buttonTask, "ButtonTask", 2048, NULL, 1, NULL);
  xTaskCreate(handleInfusion, "InfusionTask", 2048, NULL, 2, NULL);
  xTaskCreate(handleReset, "ResetTask", 2048, NULL, 2, NULL);
}

void loop() {
  // El bucle principal está vacío; FreeRTOS maneja las tareas
}

// Tarea para manejar los botones
void buttonTask(void *parameter) {
  for (;;) {
    if (digitalRead(BUTTON1_PIN) == LOW && !infusing && !resetting) {
      requestInfusionParameters();
    }

    if (digitalRead(BUTTON2_PIN) == LOW && !infusing && !resetting) {
      resetting = true;
      xSemaphoreGive(resetSemaphore);
    }

    if (digitalRead(BUTTON3_PIN) == LOW) {
      if (infusing) {
        stopInfusion();
      } else if (resetting) {
        resetting = false;
        displayMainMenu();
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Tarea para manejar la infusión
void handleInfusion(void *parameter) {
  for (;;) {
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
    }
    vTaskDelay(1);
  }
}

// Tarea para manejar el reinicio del motor
void handleReset(void *parameter) {
  const int resetSteps = stepsPerRevolution * 20;
  const long resetInterval = 1000;
  for (;;) {
    if (xSemaphoreTake(resetSemaphore, portMAX_DELAY)) {
      stepsTaken = 0;
      digitalWrite(DIR_PIN, LOW);

      for (int i = 0; i < resetSteps; i++) {
        if (!resetting) break;

        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(STEP_PIN, LOW);
        vTaskDelay(resetInterval / 1000 / portTICK_PERIOD_MS);
      }

      resetting = false;
      displayMainMenu();
    }
  }
}

// Función para mostrar el mensaje de bienvenida
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

// Función para mostrar el menú principal
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

// Función para solicitar parámetros de infusión
void requestInfusionParameters() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 20);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Ingrese volumen (mL)");
  tft.println("y tiempo (min), sep.");
  tft.println("por ',' Ejemplo: 5,2");
  delay(2000);

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    if (processUserInput(input)) {
      startInfusion(stepsRequired, intervalMicroseconds);
    } else {
      displayError("Entrada invalida");
    }
  }
}

// Función para procesar la entrada del usuario
bool processUserInput(String input) {
  int commaIndex = input.indexOf(',');
  if (commaIndex != -1) {
    float mlRequested = input.substring(0, commaIndex).toFloat();
    int timeRequested = input.substring(commaIndex + 1).toInt();

    if (mlRequested > 0 && timeRequested > 0) {
      stepsRequired = calculateSteps(mlRequested);
      intervalMicroseconds = calculateInterval(stepsRequired, timeRequested);
      return true;
    }
  }
  return false;
}

// Función para iniciar la infusión
void startInfusion(int steps, long interval) {
  infusing = true;
  stepsTaken = 0;
  lastStepTime = millis();
  digitalWrite(DIR_PIN, HIGH);

  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("INFUSION EN PROCESO");
  tft.setCursor(5, 90);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Pres. Boton 3 para detener");
}

// Función para detener la infusión
void stopInfusion() {
  infusing = false;
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("INFUSION DETENIDA");
  delay(2000);
  displayMainMenu();
}


// Función para mostrar un mensaje de error
void displayError(String message) {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 50);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.println(message);
  delay(2000);
  displayMainMenu();
}

void resumeInfusion() 
{
  Serial.println("Reanudando infusión...");
  
  // Restaurar estado previo
  stepsTaken = pausedStepsTaken; 
  float remainingMl = mlRequested - pausedMlInfused; 
  stepsRequired = calculateSteps(remainingMl); // Calcular pasos restantes
  startTime = millis(); // Reiniciar temporizador
  lastStepTime = startTime;

  infusing = true;
  infusionPaused = false;

  // Mostrar mensaje en pantalla
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
  tft.println("REANUDANDO...");
  tft.setCursor(5, 90);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Pres. Boton 3 para parar");
}
// Función para calcular pasos necesarios
int calculateSteps(float ml) {
  return (int)((ml / mlPerRevolution) * stepsPerRevolution);
}

// Función para calcular intervalo entre pasos
long calculateInterval(int steps, int timeMinutes) {
  return (long)((timeMinutes * 60000000L) / steps);
}