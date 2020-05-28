#include "terminal.h"
#include "keyboard.h"
#include "sys_call.h"
#include "paging.h"

volatile uint8_t current_term_id;
term_t terms[TERM_COUNT];

/*
*   term_init
*   description: init all 3 terminals
*   inputs: none
*   outputs: none
*   side effects: init 3 terminals
*/
void term_init() {
	uint8_t i;
	uint32_t j;
	for (i = 0; i < TERM_COUNT; i++) {
		// init all terminal
		terms[i].activate = 0;
		terms[i].xcopy = 0;
		terms[i].ycopy = 0;
		terms[i].buffer_idx = 0;
		terms[i].return_flag = 0;
		terms[i].id = i;
		terms[i].active_process_num = -1;
		// init keyboard buffer to be null
		for (j = 0; j < BUFFER_LEN; j++) {
			terms[i].keyboard_buffer[j] = '\0';
		}
		// set up paging
		PageTableToPage(_100MB, _100MB+((i+1)*_4KB), i+1);
		terms[i].video_mem = (uint8_t *)_100MB+((i+1)*_4KB);
		// clear video mem
		for (j = 0; j < NUM_ROWS*NUM_COLS; j++) {
			*(uint8_t *)(terms[i].video_mem + (j << 1)) = ' ';
			*(uint8_t *)(terms[i].video_mem + (j << 1) + 1) = 0xf;
		}
	}
	// set up first terminal and execute shell
	keyboard_buffer = terms[0].keyboard_buffer;
	restore_term(0);
	current_term_id = 0;
	execute((uint8_t*)"shell");
}

/*
*		terminal_launch
*   description: launch a new terminal
*   inputs: term_id -- which terminal to be launched
*   outputs: 0 if success
*		side effect: lauch a new terminal
*/
int32_t terminal_launch(uint8_t term_id) {
	cli();
	// do nothing if same term
	if (term_id == current_term_id) {
		return 0;
	}
	// check input
	if (term_id >= TERM_COUNT) {
		return -1;
	}
	// if term already active, just restore and switch the content
	if (terms[term_id].activate == 1) {
		if (switch_term(current_term_id, term_id) == -1) {
			return -1;
		}
		keyboard_buffer = terms[term_id].keyboard_buffer;
		current_term_id = term_id;
    uint8_t * screen_start;
    vidmap(&screen_start);
		return 0;
	}
	// if term not active, need to do execute another shell
	save_term(current_term_id);
	current_term_id = term_id;
	pcb_t * old_pcb = get_cur_pcb_process(terms[current_term_id].active_process_num);
	keyboard_buffer = terms[term_id].keyboard_buffer;
	restore_term(term_id);

    asm volatile("			\n\
                 movl %%ebp, %%eax 	\n\
                 movl %%esp, %%ebx 	\n\
                 "
                 :"=a"(old_pcb->ebp_val), "=b"(old_pcb->esp_val)
	);
	sti();
	// execute shell in new term
	execute((uint8_t*)"shell");
	return 0;
}

/*
* terminal_read
* description: terminal read function
* input : fd -- fd number
					buf -- buffer to read
					n_bytes -- #bytes to read
* outputs: # bytes read
* side effects: none
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t n_bytes) {
	return_flag = 0;
	while (return_flag == 0);
	// clip length
	if (n_bytes > BUFFER_LEN) {
		n_bytes = (BUFFER_LEN);
	}
	// copy from buffer
	uint32_t i;
	int8_t * buffer = (int8_t * )buf;
	for (i = 0; i < n_bytes; i++)	{
		if (keyboard_buffer[i] == '\0') {
			n_bytes = i;
			break;
		}
		else {
			 buffer[i] = keyboard_buffer[i];
			 }
	}

    for (i = 0; i < buffer_idx; i++)
  		keyboard_buffer[i] = '\0';

  buffer_idx = 0;								// clean idx
	return_flag = 1;							// set flag

	return n_bytes;
};

/*
* terminal_write
* description: terminal wirte function
* input : fd -- fd index
					buf -- buf to write
					n_bytes -- #bytes to write
* outputs: # bytes write to buffer
* side effects: none
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t n_bytes) {
	uint32_t i;
	uint32_t count = 0;
	int8_t * buffer = (int8_t * )buf;
	for (i = 0; i < n_bytes; i++) {
		int xcopy = get_y();
		if (buffer_idx < BUFFER_LEN-1) {
			// handle a new line
			if (buffer_idx == NUM_COLS - 1) {
				int ycopy = get_y();
				putc(buffer[i]);  // output first
				putc('\n');
				if (ycopy == NUM_ROWS - 1) {	// check need for scroll
					scroll_up();
		            set_x(0);
		            set_y(NUM_ROWS - 1);
				}
			}
			else
				putc(buffer[i]);

			if (xcopy == NUM_COLS - 1)
				update_cursor(get_x(), get_y() + 1);
			else
				update_cursor(get_x(), get_y());

			count++;
		}
	}
	return count;
};

/*
* int32_t terminal_open(int32_t fd){
* Description: open
* Inputs : filename
* Outputs: return 0 on success
* Effects: open
*/
int32_t terminal_open(const uint8_t *filename){
   return 0;
};

/*
* int32_t terminal_close(int32_t fd){
* Description: close
* Inputs : fd--file descriptor
* Outputs: return 0 on success
* Effects: close
*/
int32_t terminal_close(int32_t fd){
   return 0;
};

/*
* int32_t switch_term
* description: switch terms
* Inputs : old_term_id -- old term index
					 new_term_id -- new term index
* Outputs: 0 if success, -1 otherwise
* side effect: switch term
*/
int32_t switch_term(uint8_t old_term_id, uint8_t new_term_id) {
	if (save_term(old_term_id) == -1 || restore_term(new_term_id) == -1) {
		return -1;
	}
	return 0;
}

/*
* int32_t save_term
* description: save terminal content
* Inputs : term_id -- term index
* Outputs: 0 if success, -1 otherwise
* side effect: none
*/
int32_t save_term(uint8_t term_id) {
	// save keyboard buffer, position and screen
	terms[term_id].buffer_idx = buffer_idx;
	terms[term_id].xcopy = get_x();
	terms[term_id].ycopy = get_y();
	memcpy((uint8_t *)terms[term_id].video_mem, (uint8_t *)VIDEO, 2*NUM_ROWS*NUM_COLS);
	return 0;
}

/*
* int32_t restore_term
* description: restore terminal content
* Inputs : term_id -- term index
* Outputs: 0 if success
* side effect: none
*/
int32_t restore_term(uint8_t term_id) {
	// set up keyboard buffer, coordinate position and video mem
	buffer_idx = terms[term_id].buffer_idx;
	set_display_coord(terms[term_id].xcopy, terms[term_id].ycopy);
	memcpy((uint8_t *)VIDEO, (uint8_t *)terms[term_id].video_mem, 2*NUM_ROWS*NUM_COLS);
	return 0;
}
