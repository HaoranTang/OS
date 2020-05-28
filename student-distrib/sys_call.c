#include "sys_call.h"

/* initialize file operation table for system call read/write/open/close
 */

uint8_t pid_array [6] = { 0,0,0,0,0,0 };
file_op_table stdin_table = {terminal_read, fail_func, terminal_open, terminal_close};
file_op_table stdout_table = {fail_func, terminal_write, terminal_open, terminal_close};
file_op_table rtc_table = {rtc_read, rtc_write, rtc_opener, rtc_closer};
file_op_table dir_table = {directory_read, directory_write, directory_open, directory_close};
file_op_table file_table = {file_read, file_write, file_open, file_close};
file_op_table null_table = {fail_func, fail_func, fail_func, fail_func};
volatile uint32_t global_status;

/*	system call execute
 *	description: execute the input command
 * 	input: command -- the command to be executed
 * 	output: iret if successful, -1 if something goes wrong
 * 	side effect: execute the program
 */
int32_t execute(const uint8_t* command) {
    // cli(); //disable interrupt
	if (command == NULL) return -1;
	int i;
	uint8_t argument_buf[100];
	uint8_t parse_cmd[10];
	uint8_t cmd_start, cmd_end;
	dentry_t dentry;
	char buffer[BUFFER_SIZE];
	uint32_t entry;
	uint8_t magic[BUFFER_SIZE] = {0x7f, 0x45, 0x4c, 0x46};
	uint32_t v_addr = KERNEL_DSP;

	// parsing the command
	cmd_end = cmd_start = 0;
	// check if the first few are space
	while(command[cmd_start] == ' ') {
		cmd_start++;
	}

	cmd_end = cmd_start;
	// get the command end position
	while(command[cmd_end] != ' ' && command[cmd_end] != 0x0A && command[cmd_end] != '\0') {
		cmd_end++;
	}
	// get the parsed command
	for (i = cmd_start; i < cmd_end; i++) {
		parse_cmd[i - cmd_start] = (int8_t)command[i];
	}
	parse_cmd[cmd_end] = '\0';

	// argument
	cmd_start = cmd_end+1;
	cmd_end = cmd_start;
	// argument ending position
	while(command[cmd_end] != ' ' && command[cmd_end] != 0x0A && command[cmd_end] != '\0') {
		cmd_end++;
	}
	// get the parsed argument
	for(i = cmd_start; i < cmd_end; i++){
		argument_buf[i - cmd_start] = (int8_t)command[i];
	}
	argument_buf[cmd_end-cmd_start] = '\0';

	//read command
	if (read_dentry_by_name((uint8_t*)parse_cmd, &dentry) != 0) {
		// printf("read dentry fail in execute ");
		global_status = -1;
		return -1;
	}
	read_data(dentry.inode_num, 0, buffer, BUFFER_SIZE);
	//check validality
	for (i = 0; i < BUFFER_SIZE; i++) {
		if (buffer[i] != magic[i]) {
			// printf("file not executable ");
			return -1;
		}
	}
	//read entry point
	read_data(dentry.inode_num, 24, buffer, BUFFER_SIZE);
	entry = *((uint32_t*)buffer);

	int new_pid;
	// find a new place to process
    for (i = 0; i <= MAX_PID; i++) {
        if (pid_array[i] == 0) {
        	pid_array[i] = 1;
        	new_pid = i;
        	break;
        }
		// check if too many process are activate
        if (i == MAX_PID) {
			printf("too many programs activate, exit before contiune ");
        	return -1;
        }
    }

    //Set pcb to correct location
    pcb_t* new_pcb = get_cur_pcb_process(new_pid);

    //Store current stack values
	asm volatile("			\n\
				movl %%ebp, %%eax 	\n\
				movl %%esp, %%ebx 	\n\
			"
			:"=a"(new_pcb->parent_kbp_val), "=b"(new_pcb->parent_ksp_val));

	// Set the new page for the process
	repage(_128MB, _8MB + new_pid * _4MB);

	// Load the program into memory
	read_data(dentry.inode_num, 0, (char*)LOAD_ADDR, RANDOM_NUM);

	// set up PCB info
	new_pcb->process_num = new_pid;
	strcpy((int8_t*)(new_pcb->arg_buf), (int8_t*)argument_buf);

	// set up parent info
	if (new_pid == 0) {
		new_pcb->parent_process_num = 0;
	} else {
		new_pcb->parent_process_num = ((pcb_t*)(new_pcb->parent_ksp_val & PCB_MASK))->process_num;
	}

	//Set up FD array
	for (i = 0; i <= MAX_FD; i++) {
		new_pcb->fda[i].jumptable = null_table;
		new_pcb->fda[i].inode = -1;
		new_pcb->fda[i].file_position = 0;
		new_pcb->fda[i].flags = 0;
	}
	// set up stdin & stdout
	new_pcb->fda[0].jumptable = stdin_table;
	new_pcb->fda[1].jumptable = stdout_table;
	new_pcb->fda[0].flags = 1;
	new_pcb->fda[1].flags = 1;

	// set term in pcb
	new_pcb->term = &terms[current_term_id];
	// update term process num to be the current process num
	terms[current_term_id].active_process_num = new_pcb->process_num;
	// content switch
  	tss.ss0 = KERNEL_DS;
  	tss.esp0 = _8MB - _8KB * (new_pid) - 4;
	sti();
	// do the "artificial iret"
    EXEC_TO_USER(USER_DS, v_addr, USER_CS, entry);
	global_status = 0;
	return global_status;
}

/*	system call halt
 *	description: halt the program
 * 	input: status -- the status of the program
 * 	output: global status -- handle exception and change the status of the shell
 * 	side effect: halt the program
 */
int32_t halt(uint8_t status) {
	int i;

	cli();
	global_status = status + 1;
    /* Get current and parent PCB */
    pcb_t* cur_pcb = get_cur_pcb();
    pcb_t* parent_pcb = get_cur_pcb_process(cur_pcb->parent_process_num);
	/* close it in pid */
    pid_array[(uint8_t)cur_pcb->process_num] = 0;
    /* set all flags in PCB to not in use */
 	for (i = 0; i < MAX_FD; i++)
 	{
 		if(cur_pcb->fda[i].flags == 1){
 			close(i);
 		}
		cur_pcb->fda[i].jumptable = null_table;
 		cur_pcb->fda[i].flags = 0;
 	}
	/* if halting the last program, execute shell to prevent page fault */
	if (cur_pcb->process_num == cur_pcb->parent_process_num )
	{
		execute((uint8_t*)"shell");
	}
    /* repage */
    repage(_128MB, _8MB + parent_pcb->process_num * _4MB);
    /** set esp0 in tss */
	tss.esp0 = cur_pcb->parent_ksp_val;
	sti();
    /* Return from iret */
    asm volatile(
				 ""
                 "mov %0, %%eax;"
                 "mov %1, %%esp;"
                 "mov %2, %%ebp;"
                 "leave;"
				 "ret;"
                 :
                 :"r"((uint32_t)status), "r"(cur_pcb->parent_ksp_val), "r"(cur_pcb->parent_kbp_val)
                 :"%eax"
                 );
	return global_status;
}

/*	system call read
 *	description: system call function, read the file
 * 	input: fd -- the fd number in fda
 * 		   buf -- the buffer
 * 		   nBytes  -- # bytes to read
 * 	output: 0 if successful, -1 otherwise
 * 	side effect: see each read function
 */
int32_t read(int32_t fd, void* buf, int32_t  nBytes) {
    pcb_t* cur_pcb = get_cur_pcb();
	// check fd range
	if (fd > MAX_FD || fd < 0) return -1;
    if (buf == NULL) return -1;
    // check if file not in use
    if(cur_pcb->fda[fd].flags == 0) return -1;
    return cur_pcb->fda[fd].jumptable.read(fd, (char*)buf, nBytes);
}

/*	system call write
 *	description: system call function, write the file
 * 	input: fd -- the fd number in fda
 * 		   buf -- the buffer
 * 		   nBytes  -- # bytes to read
 * 	output: 0 if successful, -1 otherwise
 * 	side effect: see each write function
 */
int32_t write(int32_t fd, const void* buf, int32_t nBytes) {
	// check fd range
    if (fd > MAX_FD || fd < 0) {
		return -1;
	}
    pcb_t* cur_pcb = get_cur_pcb();
	// check buf valid
    if (buf == NULL) {
		return -1;
	}
	// check if in use
    if(cur_pcb->fda[fd].flags == 0) {
		return -1;
	}
    return cur_pcb->fda[fd].jumptable.write(fd, (char*)buf, nBytes);
}

/*	system call open
 *	description: system call function, open the file
 * 	input: filename -- the file name we want to open
 * 	output: fd index if successful, -1 otherwise
 * 	side effect: see each open function
 */
int32_t open(const uint8_t* filename) {
	// check empty string
	if (strlen((int8_t*)filename) == 0) {
		return -1;
	}
	uint16_t fd_idx;
	pcb_t *pcb = get_cur_pcb();
	dentry_t file_dir_entry;
	// check file name
	if (read_dentry_by_name(filename, &file_dir_entry) == -1)
	{
		return -1;
	}
	for (fd_idx = MIN_FD; fd_idx <= MAX_FD; fd_idx++) {
		// find a place in fda
		if (pcb->fda[fd_idx].flags == 0) {
			pcb->fda[fd_idx].flags = 1;
			pcb->fda[fd_idx].file_position = 0;
			break;
		}
		// check if too many process activate
		else if (fd_idx == MAX_FD ) {
			return -1;
		}
	}
	// set inode and fops_table_ptr by file type
	if (file_dir_entry.file_type == DIR_FILE_TYPE) {
			if (0 != directory_open(filename))
				return -1;
			pcb->fda[fd_idx].inode = NULL;
			pcb->fda[fd_idx].jumptable = dir_table;
		} else if (file_dir_entry.file_type == REGULAR_FILE_TYPE) {
			if (0 != file_open(filename))
				return -1;
			pcb->fda[fd_idx].inode = file_dir_entry.inode_num;
			pcb->fda[fd_idx].jumptable = file_table;
		}
		else if(file_dir_entry.file_type == RTC_FILE_TYPE) {
			if (0 != rtc_opener(filename))
				return -1;
			pcb->fda[fd_idx].inode = NULL;
			pcb->fda[fd_idx].jumptable = rtc_table;
		}
	return fd_idx;
}

/*	system call close
 *	description: system call function, close the file
 * 	input: fd -- the fd number in fda
 *
 * 	output: 0 if successful, -1 otherwise
 * 	side effect: see each close function
 */
int32_t close(int32_t fd) {
	// check fd range
    if (fd > MAX_FD || fd < MIN_FD) {
			return -1;
		}
    pcb_t* cur_pcb = get_cur_pcb();
	// check if in use
    if(cur_pcb->fda[fd].flags == 0) {
			return -1;
		}
	// check if process cannot be closed
    if (cur_pcb->fda[fd].jumptable.close(fd) != 0) {
			return -1;
		}
	// set flag to not in use
    cur_pcb->fda[fd].flags = 0;
    return 0;
}

/*	system call getargs
 * 	description: get arguments from command
 * 	input: 	buf -- store the argument
 * 			nbytes -- # bytes read
 * 	output: 0 if successful, -1 otherwise
 * 	side effect: none
*/
int32_t getargs (uint8_t* buf, int32_t nbytes) {
	/* Check to see if buf is NULL or not */
	if (buf == NULL)
		return -1;
	/* Get our pcb pointer we are making system call in */
	pcb_t* pcb = get_cur_pcb();
	/* Copy into the arguf */
	strcpy((int8_t*)buf, (int8_t*)pcb->arg_buf);
	return 0;
}

/*	system call vidmap
 * 	description: maps the text-mode video memory into user spae
 * 	input: screen_start -- start address of the screen
 * 	output: 136MB always
 * 	side effect: set up a new page
*/
int32_t vidmap(uint8_t ** screen_start){
	/* Map Virtual Address to Physical Address */
	if (screen_start == NULL || screen_start == (uint8_t**)_128MB)
	{
		return -1;
	}
	set_up_map((uint32_t)_136MB, (uint32_t)VIDEO);
	*screen_start = (uint8_t*)_136MB;
	return _136MB;
}

// for extra credit
int32_t set_handler(int32_t signum, void* handler_address) {
	return -1;
}

// for extra credit
int32_t sigreturn(void) {
	return -1;
}

// helper function
/*	get_cur_pcb_process
 *	description: helper function to get cur PCB process
 * 	input: process -- the process activate
 * 	output: current PCB  process
 * 	side effect: none
 */
pcb_t* get_cur_pcb_process(uint32_t process) {
    return (pcb_t *)(_8MB - (process + 1) * _8KB);
}

/*	fail_func
 *	description: helper function to return -1 used in file op table
 * 	input: none
 * 	output: -1
 * 	side effect: none
 */
int32_t fail_func() {
	return -1;
}

/*	get_cur_pcb_
 *	description: helper function to get cur PCB
 * 	input: none
 * 	output: current PCB
 * 	side effect: none
 */
pcb_t* get_cur_pcb(){
	pcb_t* pcb;
	//Mask the value to the correct 8KB range
	asm volatile("				   \n\
				andl %%esp, %%eax  \n\
				"
				:"=a"(pcb)
				:"a"(PCB_MASK)
				:"cc"
				);
	return pcb;
};
