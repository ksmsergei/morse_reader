/*
 * morse_decoder.c
 *
 *  Created on: Oct 18, 2024
 *      Author: ksm
 */

#include <stddef.h>
#include "morse_decoder.h"

typedef struct {
    MorseChar_t morse;
    char letter;
} MorseCode_t;

static const MorseCode_t morse_codes[] = {
    {{0b01, 2}, 'A'},
    {{0b1000, 4}, 'B'},
    {{0b1010, 4}, 'C'},
    {{0b100, 3}, 'D'},
    {{0b0, 1}, 'E'},
    {{0b0010, 4}, 'F'},
    {{0b110, 3}, 'G'},
    {{0b0000, 4}, 'H'},
    {{0b00, 2}, 'I'},
    {{0b0111, 4}, 'J'},
    {{0b101, 3}, 'K'},
    {{0b0100, 4}, 'L'},
    {{0b11, 2}, 'M'},
    {{0b10, 2}, 'N'},
    {{0b111, 3}, 'O'},
    {{0b0110, 4}, 'P'},
    {{0b1101, 4}, 'Q'},
    {{0b010, 3}, 'R'},
    {{0b000, 3}, 'S'},
    {{0b1, 1}, 'T'},
    {{0b001, 3}, 'U'},
    {{0b0001, 4}, 'V'},
    {{0b011, 3}, 'W'},
    {{0b1001, 4}, 'X'},
    {{0b1011, 4}, 'Y'},
    {{0b1100, 4}, 'Z'},
	{{0b01111, 5}, '1'},
	{{0b00111, 5}, '2'},
	{{0b00011, 5}, '3'},
	{{0b00001, 5}, '4'},
	{{0b00000, 5}, '5'},
	{{0b10000, 5}, '6'},
	{{0b11000, 5}, '7'},
	{{0b11100, 5}, '8'},
	{{0b11110, 5}, '9'},
	{{0b11111, 5}, '0'}
};

//Decodes a morse character and return ASCII character
char decode_morse_char(const MorseChar_t* morse_char) {
    for (int i = 0; i < sizeof(morse_codes) / sizeof(morse_codes[0]); i++) {
        if (morse_codes[i].morse.bits == morse_char->bits &&
            morse_codes[i].morse.length == morse_char->length) {
            return morse_codes[i].letter;
        }
    }

    return '\0'; //If no symbol was found
}

const MorseChar_t* encode_morse_char(char c) {
	for (int i = 0; i < sizeof(morse_codes) / sizeof(morse_codes[0]); i++) {
		if (morse_codes[i].letter == c) {
			return &(morse_codes[i].morse);
		}
	}

	return NULL; //If no char was found
}

//Add dot(0) or dash(1) to current morse character
MorseChar_t* add_unit(MorseChar_t* symbol, bool is_dash) {
	symbol->bits <<= 1;

	if (is_dash) {
		symbol->bits |= 1;
	} else {
		symbol->bits &= ~1;
	}


	symbol->length++;

	return symbol;
}
