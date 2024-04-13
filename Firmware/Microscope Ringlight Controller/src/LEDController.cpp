#include "LEDController.h"
#include <queue>

static Preferences preferences;

// Information about the default program values
const uint8_t DEFAULT_BRIGHTNESS = 150;
const uint16_t DEFAULT_TEMPERATURE = 4500;

// Information about the colours the LEDs will flash when setting a mode
const CRGB MODE_BRIGHTNESS_COLOUR = CRGB(0, 0, 255); // blue
const CRGB MODE_TEMPERATURE_COLOUR = CRGB(255, 0, 255); // purple
const CRGB MODE_DIRECTION_COLOUR = CRGB(0, 255, 0); // green
const uint16_t MODE_FLASH_DURATION = 500; // 0.5s

// Event Operation
enum EventOperations{BrightnessOperation = 1, TemperatureOperation = 2, DirectionOperation = 3, PowerOperation = 4};

static CRGB LEDs[LED_COUNT];

// Define a struct to represent an event
struct LEDEvent {
    EventOperations name;
    uint16_t parameter;

    LEDEvent(EventOperations name, int parameter)
            : name(name), parameter(parameter) {}
};

std::queue<LEDEvent> LedEvents;

void LEDController::Process(){
    // Process the event queue
    while (!LedEvents.empty()) {
        LEDEvent ev = LedEvents.front(); // Get the operation at the front of the queue
        LedEvents.pop(); // Remove the operation from the queue

        switch (ev.name){
            case BrightnessOperation:
                BrightnessEvent(ev.parameter);
                break;
            case TemperatureOperation:
                TemperatureEvent(ev.parameter);
                break;

            case DirectionOperation:
                DirectionEvent(ev.parameter);
                break;

            case PowerOperation:
                PowerEvent(ev.parameter);
                break;
            default:
                break;
        }
    }
}

void LEDController::begin(){
    // Set variable values
    // LED Variables

    currentDirection = 0;

    // Initialize the LED ring
    CFastLED::addLeds<CHIPSET, LED_PIN, GRB>(LEDs, LED_COUNT);

    // Retrieve variables
    preferences.begin("storage", false);
    currentBrightness = preferences.getUChar("brightness", DEFAULT_BRIGHTNESS);
    uint16_t retrievedTemperature = preferences.getUShort("temperature", DEFAULT_TEMPERATURE);
    preferences.end();

    TRACELN("Loaded State")
    TRACE("Brightness: ")
    TRACE(currentBrightness)
    TRACE("\nTemperature: ")
    TRACE(retrievedTemperature)
    TRACE("\n\n")

    if (retrievedTemperature >= 1000 && retrievedTemperature <= 12000){
        currentTemperature = retrievedTemperature;
    }
    else{
        currentTemperature = DEFAULT_TEMPERATURE;
    }

    currentMode = ModeBrightness;
    setTemperature(currentTemperature);
    setBrightness(currentBrightness);
}

void LEDController::TemperatureEvent(uint16_t kelvin) {

    TRACELN("Temperature: ")
    TRACE(kelvin)
    TRACE("\n")
    // Sets kelvin temperature value between 700 and 12000
    if (kelvin < 700 || kelvin > 12000) {
        return;
    }

    uint16_t temp;
    temp = uint16_t(kelvin / 100.0);
    int red, green, blue;
    // Calculate Red
    if (temp <= 66) {
        red = 255;
    } else {
        red = temp - 60;
        red = int(329.698727446 * (pow(red, -0.1332047592)));
        red = std::min(std::max(red, 0), 255);
    }

    // Calculate Green
    if (temp <= 66) {
        green = temp;
        green =  int(99.4708025861 * log(green) - 161.1195681661);
        green = std::min(std::max(int(green), 0), 255);
    } else {
        green = temp - 60;
        green =  int(288.1221695283 * (pow(green, -0.0755148492)));
        green = std::min(std::max(green, 0), 255);
    }

    // Calculate Blue
    if (temp >= 66) {
        blue = 255;
    } else if (temp <= 19) {
        blue = 0;
    } else {
        blue = temp - 10;
        blue = int(138.5177312231 * log(blue) - 305.0447927307);
        blue = std::min(std::max(blue, 0), 255);
    }

    for(auto & led : LEDs) {
        // let's set an led value
        led = CRGB(red, green, blue);
    }

    currentTemperature = kelvin;
    FastLED.show();
    saveState();
}
void LEDController::BrightnessEvent(uint16_t brightness){
    TRACE("Brightness: ")
    TRACE(brightness)
    TRACE("\n")
    if (brightness >= 0 && brightness <= 255) {
        FastLED.setBrightness(brightness);
        FastLED.show();
        currentBrightness = brightness;
        saveState();
    }
}

void LEDController::DirectionEvent(uint16_t direction) {
    // direction value 0 to 26
    TRACE("Direction: ")
    TRACE(direction)
    TRACE("\n")

    uint8_t position;

    if (direction == 0){ // turn on all LEDs
        for(auto & LED : LEDs) {
            LED = CRGB(255,255,255);
        }
    }
    else{
        // turn off all LEDs
        for(auto & LED : LEDs) {
            // let's set an led value
            LED = CRGB(0,0,0);
        }

        // turn on 5 inner LEDs
        for(uint8_t i = 0; i < 5; i++) {
            // let's set an led value
            position = i + direction;
            if (position >= 26) position = position - 26;
            LEDs[position] = CRGB(255,255,255);
        }
        // turn on 4 outer LEDs
        for(uint8_t i = 0; i <  4; i++) {
            position = 46 - i - direction;
            if (direction > 5) position += 1; // offsets for bolts
            if (direction > 8) position += 1;
            if (direction > 13) position += 1;
            if (direction > 17) position += 1;
            if (direction > 21) position += 1;
            if (position < 26) position = position + 20;
            LEDs[position] = CRGB(255,255,255);
        }
    }
    currentDirection = static_cast<uint8_t>(direction);
    FastLED.show();
}

void LEDController::PowerEvent(bool state){
    if (state){ // turn on
        TRACELN("Power State: On")
        if (currentBrightness < 10) currentBrightness = 10;
        FastLED.setBrightness(currentBrightness);
        FastLED.show();
        currentMode = ModeBrightness;
    }
    else{ // turn off
        TRACELN("Power State: Off")
        saveState();
        FastLED.setBrightness(0);
        FastLED.show();
        currentMode = ModeOff;
    }
}

void LEDController::setTemperature(uint16_t kelvin){
    LedEvents.emplace(TemperatureOperation, kelvin);
}

void LEDController::setBrightness(uint16_t brightness){
    LedEvents.emplace(BrightnessOperation, brightness);
}

void LEDController::setDirection(uint16_t direction){
    LedEvents.emplace(DirectionOperation, direction);
}

void LEDController::Off(){
    LedEvents.emplace(PowerOperation, false);
}

void LEDController::On(){
    LedEvents.emplace(PowerOperation, true);
}

void LEDController::flashLEDs(CRGB colour) const{
    // Flash the LED ring for a short duration with a different colour
    FastLED.showColor(colour);
    delay(MODE_FLASH_DURATION);
    setTemperature(currentTemperature);
    setBrightness(currentBrightness);
}

void LEDController::showError(ErrorState error){
    // Flash the LEDs red to show an error has occured
    for (int i = 0; i < error; i++){
        flashLEDs(CRGB(255, 0 ,0));
    }
}

void LEDController::changeMode() {
    switch (currentMode){
        case ModeOff:
            currentMode = ModeBrightness;
            TRACELN("Mode: Brightness")
            break;
        case ModeBrightness:
            flashLEDs(MODE_TEMPERATURE_COLOUR);
            TRACELN("Mode: Temperature")
            currentMode = ModeTemperature;
            break;
        case ModeTemperature:
            flashLEDs(MODE_DIRECTION_COLOUR);
            TRACELN("Mode: Direction")
            currentMode = ModeDirection;
            break;
        case ModeDirection:
            flashLEDs(MODE_BRIGHTNESS_COLOUR);
            TRACELN("Mode: Brightness")
            currentMode = ModeBrightness;
            break;
        default:
            break;
    }
}

void LEDController::Up(){
    switch (currentMode){
        case ModeBrightness: // Increase the brightness by 5
            if (currentBrightness < 255) {
                // If not divisible by 5, increase to the nearest value divisible by 5
                if (currentBrightness % 5 != 0) {
                    currentBrightness += 5 - (currentBrightness % 5);
                }

                // Check if adding 5 exceeds 255
                if (currentBrightness <= 250) {
                    currentBrightness += 5;
                } else {
                    currentBrightness = 255;
                }

                setBrightness(currentBrightness); // update the brightness
            }
            break;
        case ModeTemperature: // Increase the colour temperature
            // check if temperature value is in range
            if (currentTemperature > 12000) currentTemperature = 12000;

            // If not divisible by 200, increase to the nearest value divisible by 200
            if (currentTemperature < 12000) {
                if (currentTemperature % 200 != 0) {
                    currentTemperature += 200 - (currentTemperature % 200);
                }

                // Check if adding 200 exceeds 12000
                if (currentTemperature <= 11800) {
                    currentTemperature += 200;
                } else {
                    currentTemperature = 12000;
                }

                setTemperature(currentTemperature);
            }
            break;
        case ModeDirection: // Rotate the direction clockwise
            if (currentDirection < 25) { currentDirection ++;}
            else{currentDirection = 0;}

            setDirection(currentDirection);
            break;
        default:
            break;
    }
}

void LEDController::Down(){
    switch (currentMode){
        case ModeBrightness:
            if (currentBrightness > 0) {
                // If not divisible by 5, decrease to the nearest value divisible by 5
                if (currentBrightness % 5 != 0) {
                    currentBrightness -= 5 - (currentBrightness % 5);
                }

                // Check if subtracting 5 exceeds 0
                if (currentBrightness >= 5) {
                    currentBrightness -= 5;
                } else {
                    currentBrightness = 0;
                }

                setBrightness(currentBrightness); // update the brightness
            }
            break;
        case ModeTemperature:
            // check if temperature value is in range
            if (currentTemperature < 1000) currentTemperature = 1000;

            // If not divisible by 200, increase to the nearest value divisible by 200
            if (currentTemperature > 1200) {
                if (currentTemperature % 200 != 0) {
                    currentTemperature -= 200 - (currentTemperature % 200);
                }

                // Check if subtracting 200 is less than 1000
                if (currentTemperature >= 1200) {
                    currentTemperature -= 200;
                } else {
                    currentTemperature = 1000;
                }

                setTemperature(currentTemperature); // update the brightness
            }
            break;
        case ModeDirection:
            if (currentDirection > 1) { currentDirection --;}
            else{currentDirection = 26;}
            setDirection(currentDirection);
            break;
        default:
            break;
    }
}

void LEDController::saveState() const{
    // Save changed values to solid state memory

    preferences.begin("storage", false);
    uint8_t savedBrightness = preferences.getUChar("brightness", DEFAULT_BRIGHTNESS);

    if (currentBrightness != savedBrightness) { // check if value has changed before saving to limit unnecessary writes
        preferences.putUChar("brightness", currentBrightness);
        TRACELN("Saved Brightness State")
    }

    uint16_t savedTemperature = preferences.getUShort("temperature", DEFAULT_TEMPERATURE);
    if (currentTemperature != savedTemperature) { // check if value has changed before saving to limit unnecessary writes
        preferences.putUShort("temperature", currentTemperature);
        TRACELN("Saved Temperature State")
    }

    preferences.end();
}