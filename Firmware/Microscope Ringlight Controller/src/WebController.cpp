#include "WebController.h"

AsyncWebServer webserver(80);

void WebController::begin(){
    // Initialize the web server

    // Route for root / web page
    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/www/index.html", "text/html");
    });

    // Route to load style.css file
    webserver.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/www/styles.css", "text/css");
    });

    // Route to load scripts.js file
    webserver.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/www/scripts.js", "text/javascript");
    });

    // GET Endpoints
    webserver.on("/getstate", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Send the response
        request->send(200, "application/json", LightsData());
    });

    // Route for receiving a POST request on "/power"
    webserver.on("/power", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Allocate the JSON document
        JsonDocument doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, (const char*)data);

        // Test if parsing succeeds
        if (error) {
            TRACELN(F("deserializeJson() failed: "))
            TRACELN(error.f_str())
            request->send(400, "application/json", R"({"message":"failed"})");
            return;
        }

        if (doc["power"] == 1){
            if (ledController.currentMode == LEDController::ModeOff) {
                LEDController::On();
            }
        }
        else{
            if (ledController.currentMode != LEDController::ModeOff) {
                LEDController::Off();
            }
        }
        request->send(200, "application/json", R"({"message":"success"})");
    });

    // Route for receiving a POST request on "/brightness"
    webserver.on("/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Allocate the JSON document
        JsonDocument doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, (const char*)data);

        // Test if parsing succeeds
        if (error) {
            TRACELN(F("deserializeJson() failed: "))
            TRACELN(error.f_str())
            request->send(400, "application/json", R"({"message":"failed"})");
            return;
        }

        uint8_t brightness = doc["brightness"];

        if (ledController.currentBrightness != brightness){
            LEDController::setBrightness(brightness);
        }

        request->send(200, "application/json", R"({"message":"success"})");
    });

    // Route for receiving a POST request on "/temperature"
    webserver.on("/temperature", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Allocate the JSON document
        JsonDocument doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, (const char*)data);

        // Test if parsing succeeds
        if (error) {
            TRACELN(F("deserializeJson() failed: "))
            TRACELN(error.f_str())
            request->send(400, "application/json", R"({"message":"failed"})");
            return;
        }

        uint16_t temperature = doc["temperature"];

        if (ledController.currentTemperature != temperature){
            LEDController::setTemperature(temperature);
        }

        request->send(200, "application/json", R"({"message":"success"})");
    });

    // Route for receiving a POST request on "/direction"
    webserver.on("/direction", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // Allocate the JSON document
        JsonDocument doc;

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, (const char*)data);

        // Test if parsing succeeds
        if (error) {
            TRACELN(F("deserializeJson() failed: "))
            TRACELN(error.f_str())
            request->send(400, "application/json", R"({"message":"failed"})");
            return;
        }

        uint8_t direction = doc["direction"];

        if (ledController.currentDirection != direction){
            LEDController::setDirection(direction);
        }

        request->send(200, "application/json", R"({"message":"success"})");
    });

    webserver.onNotFound(notFound);
    webserver.begin();
}

void WebController::notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String WebController::LightsData() const {
    /// Allocate the JSON document with a specific size
    JsonDocument doc;

    doc["numberOfLights"] = 26;
    // Create the lights array
    JsonArray lights = doc["lights"].to<JsonArray>();

    // Add an object to the array using the add<JsonObject>() method
    JsonObject light = lights.add<JsonObject>();
    if (ledController.currentMode == LEDController::ModeOff) {
        light["on"] = 0;  // Light is off
    } else {
        light["on"] = 1;  // Light is on
    }
    light["brightness"] = ledController.currentBrightness;  // Brightness level
    light["temperature"] = ledController.currentTemperature;  // Color temperature
    light["direction"] = ledController.currentDirection;  // Direction

    // Serialize JSON document to String
    String response;
    serializeJson(doc, response);

    return response;
}


