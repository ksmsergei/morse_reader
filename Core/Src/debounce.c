/*
 * debounce.c
 *
 *  Created on: Oct 18, 2024
 *      Author: ksm
 */

#include "debounce.h"

//Initialization of the button structure with time and port settings
void init_button(Button_t *button, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin,
                 uint16_t check_msec, uint16_t press_msec, uint16_t release_msec, bool inverted) {
    button->GPIO_Port = GPIO_Port;
    button->GPIO_Pin = GPIO_Pin;

    button->check_msec = check_msec;
    button->press_msec = press_msec;
    button->release_msec = release_msec;

    button->inverted = inverted;

    button->counter = release_msec / check_msec;
    button->debounced_key_press = false;
}

//Function for reading the state of the button pin with regard to inversion
static bool raw_key_pressed(Button_t *button) {
    bool raw = HAL_GPIO_ReadPin(button->GPIO_Port, button->GPIO_Pin);
    return button->inverted ? !raw : raw;  //Invert the value if the button uses pull-up
}

//Button debounce treatment
void debounce_button(Button_t *button, bool *key_changed, bool *key_pressed) {
    *key_changed = false;
    *key_pressed = button->debounced_key_press;

    bool raw_state = raw_key_pressed(button);  //Reading the current pin status

    if (raw_state == button->debounced_key_press) {
        //The status has not changed, reset the counter
        if (button->debounced_key_press) {
            button->counter = button->release_msec / button->check_msec;
        } else {
            button->counter = button->press_msec / button->check_msec;
        }
    } else {
        //The status has changed, waiting for button to stabilize.
        if (--button->counter == 0) {
            //Timer expired - accept the change
            button->debounced_key_press = raw_state;
            *key_changed = true;
            *key_pressed = button->debounced_key_press;

            //Resetting the counter depending on the new state
            if (button->debounced_key_press) {
                button->counter = button->release_msec / button->check_msec;
            } else {
                button->counter = button->press_msec / button->check_msec;
            }
        }
    }
}
