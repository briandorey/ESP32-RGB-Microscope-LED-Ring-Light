#include <Arduino.h>

#ifndef MICROSCOPE_RINGLIGHT_CONTROLLER_DEBUG_H
#define MICROSCOPE_RINGLIGHT_CONTROLLER_DEBUG_H


//#define DEBUG

#ifdef DEBUG
#define TRACE(x) Serial.print(x);
#define TRACELN(x) Serial.println(x);
#else
#define TRACE(x)
#define TRACELN(x)
#endif

#endif //MICROSCOPE_RINGLIGHT_CONTROLLER_DEBUG_H
