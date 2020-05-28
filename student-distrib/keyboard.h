#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

/*for magic numbers*/
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_IRQ       1
#define OFF                0x00
#define UNPRESS            0x80
#define BUFFER_LEN         128
#define ENTER_ON           1
#define ENTER_OFF          0

/* the number of chars on a keyboard */
#define KEY_CHAR_NUM 59

/* functional keys for cp2 */
#define ENTER       0x1C
#define CAPSLOCK    0x3A
#define LSHIFT_PRESS    0x2A
#define RSHIFT_PRESS    0x36
#define LCTRL_PRESS     0x1D
#define LALT_PRESS      0x38
#define RALT_PRESS      0x38
#define RCTRL_PRESS     0x1D
#define BACKSPACE       0x0E
#define TAB             0x0F
#define SPACE           0x39
#define ESC             0x01

#define CAPSANDSHIFTS      ((CAPS_MASK | RSHIFT_MASK) | LSHIFT_MASK)
#define LETTERL            0x26
#define CTRLS_MASK         (LCTRL_MASK | RCTRL_MASK)
#define ALTS_MASK          (LALT_MASK | RALT_MASK)

// bit masks for held_keys
#define LCTRL_MASK         0x01
#define RCTRL_MASK         0x02
#define LALT_MASK          0x04
#define RALT_MASK          0x08
#define CAPS_MASK          0x10
#define LSHIFT_MASK        0x20
#define RSHIFT_MASK        0x40
#define F1_KEY		       0x3B
#define F2_KEY		       0x3C
#define F3_KEY		       0x3D
#define TERMINAL_ONE       0
#define TERMINAL_TWO       1
#define TERMINAL_THREE     2

extern volatile uint8_t *keyboard_buffer;
extern volatile uint8_t buffer_idx;
extern volatile uint8_t return_flag;

/* Initialize the keyboard */
extern void keyboard_init();
/* keyboard handler */
extern void keyboard_handler();
/* deal capitalizing key */
uint8_t capital(uint8_t held_keys);
/* helpers to deal writing process */
void update_cursor(int x, int y);
void scroll_up(void);
void update_limit(int new_x, int new_y);

#endif
