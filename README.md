# morse_reader
A program for STM32 Blue Pill that decodes morse text into ASCII text, accepting button presses of different durations and outputs this text to the ST7920 screen.

# morse_decoder.c
For this program a **morse_decoder** module was made, which contains an array with the full English alphabet in morse code, and functions to work with the same.

### MorseChar_t
```c
typedef struct {
    uint8_t bits;
    uint8_t length;
} MorseChar_t;
```

This structure stores all the information about the symbol from morse code, namely:
- **bits** - byte containing the character itself consisting of dashes and dots, where dash is 1 and dot is 0.
- **length** - length of the symbol.

So, for example, the letter **<code>X</code>**, which looks like **<code>-..-</code>** in morse code, would be written as follows:
```c
MorseChar_t x_letter = {0b1001, 4};
```

### decode_morse_char
This function takes a pointer to **MorseChar_t** and returns the ASCII representation of that morse character. If no character was found, ‘\0’ is returned.
Example:
```c
MorseChar_t w_letter = {0b011, 3};  
char c = decode_morse_char(&w_letter); //c = 'W'
```

### encode_morse_char
This function accepts an ASCII character and returns a pointer to its corresponding MorseChar_t. If none is found, it returns NULL.
Example:
```c
char f_letter = 'F';
const MorseChar_t* c = encode_morse_char(f_letter); //c = {0b0010, 4}
```

### add_unit
This function takes a pointer to MorseChar_t, and a boolean indicating that a dash will be added when true, and a dot when false.
Example:
```c
MorseChar_t c = {0, 0};
add_unit(&c, false); //dot added; c = {0b0, 1} (E)
add_unit(&c, true); //dash added; c = {0b01, 2} (A)
```

## Adding new characters
All characters are stored in the **morse_code** array of **MorseCode_t**, which stores a **MorseChar_t**, and the char it represents:
```c
static const MorseCode_t morse_codes[] = {
	{{0b01, 2}, 'A'},
	{{0b1000, 4}, 'B'},
	{{0b1010, 4}, 'C'},
  ...
```
To add a new character, simply follow this principle to add a new element to the array.

# debounce.c
To solve the chattering problem, I used the algorithm from this [link](https://www.ganssle.com/debouncing-pt2.htm) (thanks for the solution!) and made this module based on it.

To use it, we first declare the Button_t structure and initialise it:
```c
  Button_t button;
  init_button(
    &button,
    GPIO_Port,
    GPIO_Pin,
    check_msec,
    press_msec,
    release_msec,
    inverted
  );
```
Where:
- **GPIO_Port** - port to which the button is connected
- **GPIO_Pin** - button pin
- **check_msec** - time, in milliseconds, between two debounce checks. For example, if you would place the **debounce_button** function in a timer with a period of 5 ms, then the check_msec should also be 5
- **press_msec** - time to stabilise the button press. For very noisy buttons 50 ms should be sufficient, usually 10 ms is sufficient. Must be a multiple of **check_msec**.
- **release_msec** - time to stabilise the button release. Must be a multiple of **check_msec**.
- **inverted** - when reading the state of a button, whether to invert its value or not (e.g. when pull-up, inverted should be true)

After that, you need to call **debounce_button** function passing a pointer to the previously initialised button in a loop where a **check_msec**, the value you specified during initialisation, will be passed between calls.

For example, a good solution would be to use a timer like TIM2 with a period of 5 ms:
```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    //Get debounced data
    bool key_changed = false;
    bool key_pressed = false;
    debounce_button(&button, &key_changed, &key_pressed);

    if (key_changed) {
      if (key_pressed) {
        //The button was pressed
        //DO SOMETHING
      } else {
        //The button was released
        //DO SOMETHING
      }
    }
  }
}
```
