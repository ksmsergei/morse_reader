/*
 * morse_decoder.h
 *
 *  Created on: Oct 18, 2024
 *      Author: ksm
 */

#ifndef MORSE_DECODER_H
#define MORSE_DECODER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t bits;
    uint8_t length;
} MorseChar_t;

char decode_morse_char(const MorseChar_t* morse_char);
const MorseChar_t* encode_morse_char(char c);

MorseChar_t* add_unit(MorseChar_t* symbol, bool is_dash);

#endif // MORSE_DECODER_H
