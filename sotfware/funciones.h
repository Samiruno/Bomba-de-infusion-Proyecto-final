#ifndef MOTOR_DISPLAY_H
#define MOTOR_DISPLAY_H

#include <Arduino.h>


void setupPins();
void setupDisplay();
void setupMotor();


bool isButton1Pressed();
bool isButton2Pressed();
bool isButton3Pressed();


void displayWelcomeMessage();
void displayMainMenu();


void handleInfusionRequest();
void startInfusion(int steps, long interval);
void stopCurrentOperation();
void resetMotor();
void handleInfusion();
void handleReset();
int calculateSteps(float ml);
long calculateInterval(int steps, int timeMinutes);

#endif
