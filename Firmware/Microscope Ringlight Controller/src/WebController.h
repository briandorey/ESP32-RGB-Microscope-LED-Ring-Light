#ifndef MICROSCOPE_RINGLIGHT_CONTROLLER_WEBCONTROLLER_H
#define MICROSCOPE_RINGLIGHT_CONTROLLER_WEBCONTROLLER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include "LEDController.h"
#include "Debug.h"

class WebController {
public:
    // Constructor that initializes the ledController reference
    explicit WebController(LEDController& controller) : ledController(controller) {}
    void begin();
private:
    LEDController& ledController; // Declare a reference to LEDController
    static void notFound(AsyncWebServerRequest *request);
    String LightsData() const;
};


#endif //MICROSCOPE_RINGLIGHT_CONTROLLER_WEBCONTROLLER_H
