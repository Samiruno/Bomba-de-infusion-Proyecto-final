#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>

// Inicialización de módulos
void initDisplay();
void initButtons();
void initMotor();

// Funciones de visualización
void displayWelcomeMessage();
void displayMainMenu();

// Funciones del motor
void moveMotor(int steps, bool direction, int interval);
int calculateSteps(float ml);
long calculateInterval(int steps, int timeMinutes);

// Configuración de tareas FreeRTOS
void setupTasks();
void handleInfusionTask(void *parameter);
void handleResetTask(void *parameter);

#endif
