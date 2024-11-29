#include "../include/motor_display.h"

void setup() {
    setupPins();           
    setupDisplay();        
    setupMotor();          
    displayWelcomeMessage();
}

void loop() {
    if (isButton1Pressed()) {
        handleInfusionRequest();
    }

    if (isButton2Pressed()) {
        resetMotor();
    }

    if (isButton3Pressed()) {
        stopCurrentOperation();
    }

    handleInfusion(); 
    handleReset();   
}
