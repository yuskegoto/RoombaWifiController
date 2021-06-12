#include <Arduino.h>
#include "Action.h"
#include "def.h"

void Action::forward(){
    motorR += MOTOR_SPEED_STEP;
    motorL += MOTOR_SPEED_STEP;
    updated = true;
}

void Action::back(){
    motorR += -MOTOR_SPEED_STEP;
    motorL += -MOTOR_SPEED_STEP;       
    updated = true;
}

void Action::stop(){
    motorR = 0;
    motorL = 0;
    // cleaning = false;
    updated = true;
}

void Action::turnRight(){
    motorR += -MOTOR_SPEED_STEP;
    motorL += MOTOR_SPEED_STEP;
    updated = true;
}

void Action::turnLeft(){
    motorR += MOTOR_SPEED_STEP;
    motorL += -MOTOR_SPEED_STEP;
    updated = true;
}

void Action::setXY2Speed(int cx, int cy)
{
    motorR = (-cy - cx) * MOTOR_SPEED_FACTOR;
    motorL = (-cy + cx) * MOTOR_SPEED_FACTOR;

    updated = true;
}

void Action::toggleCleaningMotors(){
    if (cleaning) cleaning = false;
    else cleaning = true;
    updated = true;

}