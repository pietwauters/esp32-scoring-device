#include "ESP32Button.h"

std::map<uint8_t, ESP32Button*> ESP32Button::instances;

ESP32Button* ESP32Button::getInstance(uint8_t pin, bool activeLow, uint16_t debounceTimeMs) {
    if (instances.find(pin) == instances.end()) {
        instances[pin] = new ESP32Button(pin, activeLow, debounceTimeMs);
    }
    return instances[pin];
}

ESP32Button::ESP32Button(uint8_t pin, bool activeLow, uint16_t debounceTimeMs)
    : pin(pin), activeLow(activeLow), debounceTime(debounceTimeMs), state(activeLow ? HIGH : LOW),
      lastState(state), stateChangedFlag(false), lastDebounceTime(0) {}

void ESP32Button::begin() {
    pinMode(pin, activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
}

bool ESP32Button::currentState() const {
    return state;
}

bool ESP32Button::stateHasChanged() {
    bool changed = stateChangedFlag;
    stateChangedFlag = false; // Clear the flag after checking
    return changed;
}

bool ESP32Button::isPressed() const {
    return activeLow ? !state : state;
}

bool ESP32Button::isReleased() const {
    return activeLow ? state : !state;
}

void ESP32Button::setDebounceTime(uint16_t timeMs) {
    debounceTime = timeMs;
}

void ESP32Button::doUpdate() {
    bool reading = digitalRead(pin);
    if (reading != lastState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceTime) {
        if (reading != state) {
            state = reading;
            stateChangedFlag = true;
        }
    }
    lastState = reading;
}
