#ifndef TERMINAL_H
#define TERMINAL_H


#include "lib.h"
#include "i8259.h"

#define BUFFER_LEN         128
#define TERM_COUNT         3


extern volatile uint8_t current_term_id;
/**TERMINAL STRUCT **/
typedef struct {
    // terminal id (ie. 0, 1, 2)
    uint8_t id;
	//active process number
	int8_t active_process_num;
    // whether terminal has a process activate
    uint8_t activate;
    // coord copy
    uint32_t xcopy;
    uint32_t ycopy;
    // key buffer for each terminal
    volatile uint8_t keyboard_buffer[BUFFER_LEN];
    volatile uint8_t buffer_idx;
    volatile uint8_t return_flag;
    //ptr to video memory for terminal
    uint8_t *video_mem;
} term_t;

/* Global Variables */
extern volatile uint8_t current_term_id;
extern term_t terms[TERM_COUNT];

/*Function Definitions */
void term_init(void);
int32_t terminal_launch(uint8_t term_id);
int32_t save_term(uint8_t term_id);
int32_t restore_term(uint8_t term_id);
int32_t switch_term(uint8_t old_term_id, uint8_t new_term_id);

/*Terminal System Calls */
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t n_bytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t n_bytes);

#endif
