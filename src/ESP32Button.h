#ifndef ESP32_BUTTON_H
#define ESP32_BUTTON_H

#include <Arduino.h>
#include <map>
#include "SubjectObserverTemplate.h"

/**
 * @class ESP32Button
 * @brief A debounced button handler for ESP32 with singleton pattern.
 *
 * This class provides functionality to debounce a button and track its state
 * using a singleton pattern to ensure only one instance per GPIO pin.
 */
class ESP32Button: public Subject<ESP32Button>{
public:
    /**
     * @brief Gets an instance of the button for a given GPIO pin.
     * @param[in] pin GPIO pin number.
     * @param[in] activeLow If true, the button is active-low (default: true).
     * @param[in] debounceTimeMs Debounce time in milliseconds (default: 20ms).
     * @return Pointer to the ESP32Button instance.
     */
    static ESP32Button* getInstance(uint8_t pin, bool activeLow = true, uint16_t debounceTimeMs = 20);

    /**
     * @brief Initializes the button with appropriate pull-up or pull-down configuration.
     */
    void begin();

    /**
     * @brief Gets the current state of the button.
     * @return True if the button is HIGH, false otherwise.
     */
    bool currentState() const;

    /**
     * @brief Checks if the button state has changed since the last update and resets the flag.
     * @return True if the state has changed, false otherwise.
     */
    bool stateHasChanged();

    /**
     * @brief Checks if the button is currently pressed.
     * @return True if the button is pressed, false otherwise.
     */
    bool isPressed() const;

    /**
     * @brief Checks if the button is currently released.
     * @return True if the button is released, false otherwise.
     */
    bool isReleased() const;

    /**
     * @brief Updates the button state and applies debounce logic.
     *
     * This function should be called frequently in the loop to ensure accurate debouncing.
     * It sets the stateChangedFlag when a state change is detected and retains it until
     * stateChanged() is called to check and reset the flag.
     */
    void doUpdate();

    /**
     * @brief Sets a new debounce time for the button.
     * @param[in] timeMs New debounce time in milliseconds.
     */
    void setDebounceTime(uint16_t timeMs);

    void StateChanged (uint32_t eventtype) {notify(eventtype);}

private:
    ESP32Button(uint8_t pin, bool activeLow, uint16_t debounceTimeMs);
    static std::map<uint8_t, ESP32Button*> instances;

    uint8_t pin;          ///< GPIO pin number.
    bool activeLow;       ///< Indicates if the button is active-low.
    uint16_t debounceTime; ///< Debounce time in milliseconds.
    bool state;           ///< Current button state.
    bool lastState;       ///< Previous button state.
    bool stateChangedFlag; ///< Flag to indicate state change, retained until checked.
    uint32_t lastDebounceTime; ///< Last debounce timestamp.
};

#endif // ESP32_BUTTON_H
