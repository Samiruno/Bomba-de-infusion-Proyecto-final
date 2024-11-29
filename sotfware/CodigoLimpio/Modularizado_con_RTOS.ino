#include "functions.h"

void setup() {
    Serial.begin(115200);

    // Inicialización de módulos
    initDisplay();
    initButtons();
    initMotor();

    // Configuración de tareas
    setupTasks();

    displayWelcomeMessage(); // Mostrar mensaje de bienvenida
}

void loop() {
    // El bucle principal no hace nada, todo es manejado por FreeRTOS
}
