#ifndef SYS_CALL_H_
#define SYS_CALL_H_


#include "lib.h"
#include "types.h"
#include "terminal.h"
#include "file_sys.h"
#include "rtc_handler.h"
#include "paging.h"
#include "x86_desc.h"
#define _100MB 0x6400000
#define _128MB 0x8000000
#define _136MB 0x8800000
#define LOAD_ADDR 0x8048000
#define _8MB 0x800000
#define _4MB 0x400000
#define _8KB 0x2000
#define _4KB 0x1000
#define MAX_COMMAND_SIZE 10
#define MAX_BUFFER_SIZE 100
#define BUFFER_SIZE 4
#define ASCII_DEL 0x7f
#define ASCII_E 0x45
#define ASCII_L 0x4c
#define ASCII_F 0x46
#define ASCII_NL 0x0A
#define MAX_PID   5
#define MIN_FD		2
#define MAX_FD		7
#define FILE_START 0x0000
#define KERNEL_DSP 0x83FFFFC
#define RANDOM_NUM 50000
#define PCB_MASK 0xFFFFE000
/* system call functions */
void EXEC_TO_USER(uint32_t ds,uint32_t v_addr,uint32_t cs, uint32_t ent);
// void IRET_RETURN(uint32_t status, uint32_t parent_ksp, uint32_t parent_kbp);

int32_t execute(const uint8_t* command);
int32_t halt(uint8_t status);
int32_t read(int32_t fd, void* buf, int32_t nBytes);
int32_t write(int32_t fd, const void* buf, int32_t nBytes);
int32_t vidmap(uint8_t ** screen_start);
int32_t open(const uint8_t* filename);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
int32_t close(int32_t fd);
int32_t fail_func();
#define PCB_MASK 0xFFFFE000

/* file operation table structure */
typedef struct {
  int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
} file_op_table;

/* file descriptor structure */
typedef struct {
    file_op_table jumptable;
    int32_t inode;
    int32_t file_position;
    int32_t flags;
} file_des_t;

/* pcb structure */
typedef struct {
    file_des_t fda[8];          // file desc array
    uint8_t arg_buf[100];        // arg buf
    uint32_t esp_val;           // esp reg value
    uint32_t ebp_val;           // ebp reg value
    uint32_t parent_ksp_val;    // parent vals
	  uint32_t parent_kbp_val;
    uint8_t process_num;
	  uint8_t parent_process_num;
    term_t * term;
} pcb_t;

extern uint8_t pid_array[6];
pcb_t* get_cur_pcb_process(uint32_t process);
pcb_t* get_cur_pcb();
#endif
