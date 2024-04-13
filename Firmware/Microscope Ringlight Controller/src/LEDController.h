#ifndef MICROSCOPE_RINGLIGHT_CONTROLLER_LEDCONTROLLER_H
#define MICROSCOPE_RINGLIGHT_CONTROLLER_LEDCONTROLLER_H

#include <FastLED.h>
#include <Preferences.h>
#include "Debug.h"

#define LED_PIN     32
#define LED_COUNT    46
#define CHIPSET     WS2812B

class LEDController {
public:
    // Error states
    enum ErrorState{ErrorNoWifi = 1, ErrorFlashMem = 2, ErrorGeneralException = 3};
    // Program modes
    enum  Mode {ModeBrightness = 0, ModeTemperature = 1, ModeDirection = 2, ModeOff = 3};
    volatile Mode currentMode = ModeOff;

    // LED Variables
    volatile uint8_t currentBrightness = 255;
    volatile uint8_t currentDirection = 0;
    volatile uint16_t currentTemperature = 5000;

    void Process();
    void begin();
    static void Off();
    static void On();
    static void setTemperature(uint16_t kelvin);
    static void setBrightness(uint16_t brightness);
    static void setDirection(uint16_t direction);
    void changeMode();
    void Up();
    void Down();
    void showError(ErrorState error);

private:


    void flashLEDs(CRGB colour) const;
    void saveState() const;
    void TemperatureEvent(uint16_t kelvin);
    void BrightnessEvent(uint16_t brightness);
    void DirectionEvent(uint16_t direction) ;
    void PowerEvent(bool state);
};

#endif //MICROSCOPE_RINGLIGHT_CONTROLLER_LEDCONTROLLER_H
