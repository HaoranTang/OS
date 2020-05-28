/* keyboard initialization*/

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

volatile uint8_t return_flag = 0;
// Index of the last item in the keyboard buffer
volatile uint8_t buffer_idx = 0;

// Key buffer terminated by return key
volatile uint8_t *keyboard_buffer;

/* caps and shift are not pressed*/
char scancode_array[59][4] = {
    {  0 , 0 , 0 , 0   }, // 0x00 Error (not a key)
    {  0 , 0 , 0 , 0   }, // 0x01 Esc
    { '1','!','1','!'  },
    { '2','@','2','@'  },
    { '3','#','3','#'  },
    { '4','$','4','$'  },
    { '5','%','5','%'  },
    { '6','^','6','^'  },
    { '7','&','7','&'  },
    { '8','*','8','*'  },
    { '9','(','9','('  },
    { '0',')','0',')'  },
    { '-','_','-','_'  },
    { '=','+','=','+'  },
    {  0 , 0 , 0 , 0   }, // 0x0E  Backspace
    { ' ',' ',' ',' '  }, // 0x0F  Tab is just one space...for now
    { 'q','Q','Q','q'  },
    { 'w','W','W','w'  },
    { 'e','E','E','e'  },
    { 'r','R','R','r'  },
    { 't','T','T','t'  },
    { 'y','Y','Y','y'  },
    { 'u','U','U','u'  },
    { 'i','I','I','i'  },
    { 'o','O','O','o'  },
    { 'p','P','P','p'  },
    { '[','{','[','{'  },
    { ']','}',']','}'  },
    {  0 , 0 , 0 , 0   }, // 0x1C Enter
    {  0 , 0 , 0 , 0   }, // 0x1D  Left Ctrl
    { 'a','A','A','a'  },
    { 's','S','S','s'  },
    { 'd','D','D','d'  },
    { 'f','F','F','f'  },
    { 'g','G','G','g'  },
    { 'h','H','H','h'  },
    { 'j','J','J','j'  },
    { 'k','K','K','k'  },
    { 'l','L','L','l'  },
    { ';',':',';',':'  },
    {'\'','"','\'','"' },  // 0x28  ' " (quotation marks)
    { '`','~','`','~'  },
    {  0 , 0 , 0 , 0   },  // 0x2A   Left Shift
    {'\\','|','\\','|' },
    { 'z','Z','Z','z'  },
    { 'x','X','X','x'  },
    { 'c','C','C','c'  },
    { 'v','V','V','v'  },
    { 'b','B','B','b'  },
    { 'n','N','N','n'  },
    { 'm','M','M','m'  },
    { ',','<',',','<'  },
    { '.','>','.','>'  },
    { '/','?','/','?'  },
    {  0 , 0 , 0 , 0   }, // 0x36    RShift
    {  0 , 0 , 0 , 0   }, // 0x37    Keypad *
    {  0 , 0 , 0 , 0   }, // 0x38    Left alt
    {' ',' ',' ',' '   }, // 0x39    Spacebar
    {  0 , 0 , 0 , 0   }, // 0x3A    CapsLock
};

static char* video_mem = (char *)VIDEO; // pointer to vid mem

int lim_x, lim_y;                       // local vars for backspace
uint8_t held_keys = OFF;                // local key-being-held var

/*
 * update_limit()
 *   DESCRIPTION: update the backspace limit
 *   INPUTS: new_x, new_y
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change the limit coord for backspace
 */
void update_limit(int new_x, int new_y){
    lim_x = new_x;
    lim_y = new_y;
}

/*
 * keyboard_init()
 *   DESCRIPTION: initialize the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ on PIC through enable_irq
 */
void keyboard_init() {
  /*initialize keyboard*/
  enable_irq(KEYBOARD_IRQ);
}

/*
 * keyboard_handler()
 *   DESCRIPTION: handle all key pressing cmds
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: execute key cmd
 */
void keyboard_handler(){
  /*critical area*/
  cli();
  char input;
  uint8_t scancode = inb(KEYBOARD_DATA_PORT);   //* get the input from port*/

  int xcopy = get_x();        // get current coord
  int ycopy = get_y();

  switch (scancode){

     case LSHIFT_PRESS:                        // handle L/R shift
        held_keys |= LSHIFT_MASK;
        break;
     case UNPRESS + LSHIFT_PRESS:
        held_keys &= ~LSHIFT_MASK;
        break;
     case RSHIFT_PRESS:
        held_keys |= RSHIFT_MASK;
        break;
     case UNPRESS + RSHIFT_PRESS:
        held_keys &= ~RSHIFT_MASK;
        break;

     case LCTRL_PRESS:                         // handle L/R ctrl
        held_keys |= LCTRL_MASK;
        break;
     case UNPRESS + LCTRL_PRESS:
        held_keys &= ~LCTRL_MASK;
        break;

     case LALT_PRESS:                    // handle left and right alt keys
        held_keys |= LALT_MASK;
        break;
     case UNPRESS + LALT_PRESS:
        held_keys &= ~LALT_MASK;
        break;

     case CAPSLOCK:                            // handle capslock
        if((held_keys & CAPS_MASK) == OFF){
           held_keys |= CAPS_MASK;
        }
        else{
           held_keys &= ~CAPS_MASK;
        }

     case UNPRESS + CAPSLOCK:
        break;

     case BACKSPACE:                        // handle backspace
        if (buffer_idx > 0) {
           set_x(xcopy-1); // move writing spot one step back
           keyboard_buffer[buffer_idx] = '\0'; // clean
           buffer_idx--;
           *(uint8_t *)(video_mem + ((NUM_COLS * ycopy  + xcopy-1) << 1)) = ' '; // clean char
           update_cursor(get_x(), ycopy); // not in last row update cursor
      }
        break;

     case ENTER:                       // handle enter
        return_flag = 1;
        if (ycopy == NUM_ROWS - 1){
           scroll_up();
           update_cursor(0,NUM_ROWS - 1);
           set_x(0);
           set_y(NUM_ROWS - 1);
           update_limit(1,NUM_ROWS-1); // update backspace limit
        }
        else{
           putc('\n');
           update_cursor(get_x(),get_y());
           update_limit(get_x() + 1,get_y()); // update backspace limit
        }
        keyboard_buffer[buffer_idx] = '\n';
        buffer_idx++;
        break;

     case ESC:                  // debugging use now
        set_x(0);
        set_y(NUM_ROWS -1);
        update_cursor(get_x(),get_y());
        break;
  }

  input = scancode_array[scancode][capital(held_keys)];   /* for cp1 lowercase*/

  if (input && scancode < 60) {							//Only print if valid character
      int xcopy = get_x();
      terminal_write(0,&input, 1);           // write to terminal
      keyboard_buffer[buffer_idx] = input;
      buffer_idx++;
      if (xcopy == NUM_COLS-1) {
    	  update_cursor(get_x(),get_y()+1);
      } else {
    	  update_cursor(get_x(),get_y());
      }

   }

   if (scancode == LETTERL && (held_keys & CTRLS_MASK) != OFF){  // if a CTRL key is held and L is pushed, clear screen
      clear();
      update_cursor(0,0);// move cursor to beginning of screen
      set_x(0);
      set_y(0);
      update_limit(-1,-1);

	   int i;
		for (i = 0; i < buffer_idx; i++) {
			keyboard_buffer[i] = '\0';
		}
		buffer_idx = 0;
   }
   /* handle terminal switches; max number of terminal is 3 */
   if (scancode == F1_KEY && (held_keys & ALTS_MASK) != OFF){
      send_eoi(1);
      terminal_launch(TERMINAL_ONE);
   }
   if (scancode == F2_KEY && (held_keys & ALTS_MASK) != OFF){
      send_eoi(1);
      terminal_launch(TERMINAL_TWO);
   }
   if (scancode == F3_KEY && (held_keys & ALTS_MASK) != OFF){
      send_eoi(1);
      terminal_launch(TERMINAL_THREE);
   }
	// Send EOI and unmask interrupts
	send_eoi(1);
	sti();
}

/*
* void update_cursor
* Description: update cursor position
* Inputs: int x, int y
* Outputs: None
* Effects: update cursor position
*/
void update_cursor(int x, int y){
	uint16_t pos = NUM_COLS*y + x;
	outw(0x000E | (pos & 0xFF00), 0x03D4);
	outw(0x000F | ((pos << 8) & 0xFF00), 0x03D4);
}

/*
* void scroll_up(void)
* Description: move screen up one row to scroll down
* Inputs: None
* Outputs: None
* Effects: Everything in Video Memory is shifted
*/
void scroll_up(void){
	int32_t x;
	int32_t y;
	int32_t old_line;
	int32_t new_line;

	// Move each row in the video memory up by 1
	for (y = 0; y < NUM_ROWS-1; y++) {
		for (x = 0; x < NUM_COLS; x++) {
			old_line = NUM_COLS*(y+1) + x;
			new_line = NUM_COLS*y + x;
			*(uint8_t *)(video_mem + (new_line << 1)) = *(uint8_t *)(video_mem + (old_line << 1));
		}
	}

	// Clear the bottom line
	for (x = 0; x < NUM_COLS; x++) {
		new_line = NUM_COLS*(NUM_ROWS-1) + x;
		*(uint8_t *)(video_mem + (new_line << 1)) = ' ';
	}
	set_x(0);
   update_limit(0, NUM_ROWS-2);
}

/*
* capital
* Description: helper handling capitalizing during input
* Inputs: held_keys
* Outputs: which column to use
* Effects: determine lower or upper case of the input
*/
uint8_t capital(uint8_t held_keys){
   int keys = held_keys & CAPSANDSHIFTS;
   switch(keys){

      case OFF:  // no capital, use column 0
         return 0;
      case LSHIFT_MASK:
      case RSHIFT_MASK:
      case LSHIFT_MASK | RSHIFT_MASK:
         return 1;    // shift pushed, use column 1
      case CAPS_MASK: // capsLock on, use column 2
         return 2;
      case (CAPS_MASK | LSHIFT_MASK):  //CapsLock on and Shift pushed, use column 3
      case (CAPS_MASK | RSHIFT_MASK):
      case (CAPS_MASK | RSHIFT_MASK | LSHIFT_MASK):
         return 3;
   }
   return 0;
}
