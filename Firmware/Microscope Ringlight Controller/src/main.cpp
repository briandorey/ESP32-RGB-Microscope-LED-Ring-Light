#include <Arduino.h>
#include <WiFi.h>
#include "LEDController.h"
#include "WebController.h"
#include <ESPmDNS.h>
#include "Debug.h"
#include "SPIFFS.h"

// Status LEDs
const uint8_t LED_D1 = 13; // D1 Status LED
const uint8_t LED_D2 = 15; // D2 Status LED

// Information about the rotary encoder
const uint8_t ENCODER_A_PIN = 25; // ESP32 pin GPIO25 connected to encoder pin A
const uint8_t ENCODER_B_PIN = 26; // ESP32 pin GPIO26 connected to encoder pin B
const uint8_t ENCODER_SWITCH_PIN = 27; // ESP32 pin GPIO27 connected to encoder switch
static volatile uint64_t encoderLastSwitchTime = 0;
static volatile uint64_t encoderSwitchStartTime = 0;

enum EncoderStates{Stationary, CounterClockwise, Clockwise};
static volatile EncoderStates encoderRotationFlag = Stationary; // signals a change in the encoder rotation
static volatile bool encoderSwitchChangeStateFlag = false; // signals a switch press
static volatile bool encoderSwitchPressedFlag = false; // signals a switch press
static volatile bool encoderSwitchLongPressedFlag = false; // signals a switch long press

// Information about the web server
const char* HOST_NAME = "microscope";

LEDController ledController;
WebController webController(ledController);

const char* ssid = "wifissid";
const char* password = "wifipassword";

void IRAM_ATTR ISR_encoder_rotation() {
    uint8_t pinAState = digitalRead(ENCODER_A_PIN);
    uint8_t pinBState = digitalRead(ENCODER_B_PIN);

    if (pinAState == HIGH && pinBState == HIGH) {
        // the encoder is rotating in counter-clockwise direction
        if (encoderRotationFlag == Stationary) {
            encoderRotationFlag = CounterClockwise;
        }
    } else if (pinAState == LOW && pinBState == HIGH) {
        // the encoder is rotating in clockwise direction
        if (encoderRotationFlag == Stationary) {
            encoderRotationFlag = Clockwise;
        }
    }
}

void IRAM_ATTR ISR_encoder_switch() {
    if ((millis() - encoderLastSwitchTime) < 50) { // debounce time is 50ms
        return;
    }
    encoderSwitchChangeStateFlag = true;
    encoderLastSwitchTime = millis();
}

void setup() {
    #ifdef DEBUG
        Serial.begin(115200);
    #endif

    // Rotary Encoder Setup

    // configure encoder pins as inputs
    pinMode(ENCODER_A_PIN, INPUT);
    pinMode(ENCODER_B_PIN, INPUT);
    pinMode(ENCODER_SWITCH_PIN, INPUT);

    // configure LED pins as outputs
    pinMode(LED_D1, OUTPUT);
    pinMode(LED_D2, OUTPUT);

    digitalWrite(LED_D1, HIGH); // Set D1 Led high to show the device is powered

    bool failed = false; // flag for checking if setup completes successfully

    // call ISR_encoder() when CLK pin changes from LOW to HIGH
    attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), ISR_encoder_rotation, RISING);

    // call ISR_encoder() when CLK pin changes from HIGH to LOW
    attachInterrupt(digitalPinToInterrupt(ENCODER_SWITCH_PIN), ISR_encoder_switch, CHANGE);

    ledController.begin(); // Initialize the LED controller object

    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        TRACE("An Error has occurred while mounting SPIFFS")
        failed = true;
        ledController.showError(LEDController::ErrorFlashMem);
    }

    // Initialize the Wi-Fi module
    WiFiClass::mode(WIFI_STA);

    WiFiClass::setHostname(HOST_NAME);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        TRACE("WiFi Failed!\n")
        failed = true;
        ledController.showError(LEDController::ErrorNoWifi);
    }
    else{ // Wi-Fi connected
        TRACE("IP Address: ")
        TRACELN(WiFi.localIP())
    }

    if(!MDNS.begin(HOST_NAME)) {
        TRACELN("Error starting mDNS")
        failed = true;
        ledController.showError(LEDController::ErrorGeneralException);
    }
    else{
        // Advertise the services
        MDNS.addService("http", "tcp", 80);
    }

    if (failed) {
        digitalWrite(LED_D2, HIGH); // Set D2 LED high to show an error has occurred.
    }
    else{
        webController.begin();
    }
}

void loop() {
    if (encoderSwitchChangeStateFlag) {
        delay(10); // wait 10ms for button to stabilize
        if (digitalRead(ENCODER_SWITCH_PIN) == 0){ // button is pressed
            if (!encoderSwitchPressedFlag){
                encoderSwitchStartTime = millis();
                encoderSwitchPressedFlag = true;
                encoderSwitchLongPressedFlag = false;
            }
        }
        else{ // button is released
            encoderSwitchStartTime = 0;
            encoderSwitchPressedFlag = false;
            if (!encoderSwitchLongPressedFlag && ledController.currentMode != LEDController::ModeOff) {
                TRACELN("short press")
                ledController.changeMode();
            }
            encoderSwitchLongPressedFlag = false;
        }

        encoderSwitchChangeStateFlag = false;
        encoderLastSwitchTime = millis();
    }

    if (encoderSwitchPressedFlag){
        if (millis() - encoderSwitchStartTime > 2000){ // button pressed for more than 2 seconds
            TRACELN("Long press")
            if (ledController.currentMode == LEDController::ModeOff) {
                LEDController::On();
            } else {
                LEDController::Off(); // turn light off
            }
            encoderSwitchStartTime = 0;
            encoderSwitchPressedFlag = false;
            encoderSwitchLongPressedFlag = true;
        }
    }

    if (encoderRotationFlag == Clockwise){ // Encoder has rotated clockwise
        delay(10); // wait 10ms for button to stabilize
        ledController.Up();
        encoderRotationFlag = Stationary;
    }
    else if (encoderRotationFlag == CounterClockwise){ // Encoder has rotated counter-clockwise
        delay(10); // wait 10ms for button to stabilize
        ledController.Down();
        encoderRotationFlag = Stationary;
    }

    ledController.Process(); // process the next event in the LED operations queue
}