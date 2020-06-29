/**
    @author Yuske Goto
    Also thanks a lot for my old college Jochen for cleaning my code-chaos. Even though I'm eternaly C noob, I couldn't make it without your support
*/

#ifndef _ROBOT_DEBUG_H_
#define _ROBOT_DEBUG_H_

#include <Arduino.h>
#include "Status.h"
// #include "Movement.h"

#define DEBUG_GENERAL   0x01
#define DEBUG_SENSOR    0x02
#define DEBUG_MOVE      0x04
#define DEBUG_ACTION    0x08
#define DEBUG_CONTROL   0x10

class Debug{

    public:
        uint8_t outputLevel = 0;

        Debug(Stream *s, uint8_t);
        int getFreeRam();
        
        void println(const __FlashStringHelper*, uint8_t level);
        void print(const __FlashStringHelper*, uint8_t level);
        void println(String, uint8_t level);
        void print(String, uint8_t level);
        void println(const char*, uint8_t level);
        void print(const char*, uint8_t level);
        void print(char, uint8_t level);
        void print(uint8_t, uint8_t level);
        void println(uint8_t, uint8_t level);
        void print(int8_t, uint8_t level);
        void println(int8_t, uint8_t level);
        void print(int16_t, uint8_t level);
        void println(int16_t, uint8_t level);
        void print(uint16_t, uint8_t level);
        void println(uint16_t, uint8_t level);
        void ledOn();
        void ledOff();

        // void print(SensorData*);
        // void print(Motor*);

        // void writeXY(float, float, float, float);
        // void processingTrack(SensorData* data);

    private:
        Stream *stream;
        uint32_t loopTimer;

};

#endif
