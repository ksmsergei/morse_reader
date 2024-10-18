/*
 * debounce.h
 *
 *  Created on: Oct 18, 2024
 *      Author: ksm
 */

#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

//Structure for storing button data and settings
typedef struct {
    GPIO_TypeDef *GPIO_Port;	//Port to which the button is connected
    uint16_t GPIO_Pin;			//Button pin

    bool debounced_key_press;	//Debounced button state
    bool inverted;				//Logic inversion flag

    uint8_t counter;			//Counter for delay

    uint16_t check_msec;		//Time between button status checks
    uint16_t press_msec;		//Time for press debouncing
    uint16_t release_msec;		//Time for release debouncing
} Button_t;

void init_button(Button_t *button, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin,
                 uint16_t check_msec, uint16_t press_msec, uint16_t release_msec, bool inverted);

void debounce_button(Button_t *button, bool *key_changed, bool *key_pressed);

#endif // DEBOUNCE_H


