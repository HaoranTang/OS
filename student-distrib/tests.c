#include "tests.h"
#include "x86_desc.h"
#include "idt.h"
#include "lib.h"
#include "rtc_handler.h"
#include "keyboard.h"
#include "file_sys.h"
#include "terminal.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
  printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/* divide test
 * Description: test to trigger divide by zero exception
 * Inputs: None
 * Outputs: None
 * Side Effects: trigger exception
 * Coverage: divide by zero
 * Files: idt.c/h
 */

void divide_test(){
	TEST_HEADER;
	int result;
	int five = 5;
	int zero = 0;
	result = five / zero;
}

/*
 * rtc_test
 * Description: check if rtc is initialized
 * NPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS: fill the screen with char that blinks
 * COVERAGE: RTC
 * Files: rtc_handler.c/h
 */
// void rtc_test(){
// 	rtc_init();
// }

/*
 *  opcode_test
 *	Description: check if opcode exception is invalid
 *	INPUTS: None
 *  OUTPUTS: None
 *	SIDE EFFECTS: trigger exception
 *	COVERAGE: invalid opcode
 *	FILES: idt.c/h
 */
int opcode_test() {
  TEST_HEADER;
  asm volatile("ud2");
  return FAIL;
}

/* null test
 * Description: dereference null and trigger page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: trigger page fault
 * Coverage: dereference null
 * Files: idt.c/h
 */
int null_test(){
	TEST_HEADER;
	int result = FAIL;
	int* test = 0;
	*test = 9;
	return result;
}

/*
 * keyboard_test
 * Description: Init the keyboard so the OS show input character on screen
 * Inputs: None
 * Outputs: None
 * Side effects: init the keyboard
 * Coverages: Keyboard initialization
 * Files: keyboard.c
 */
void keyboard_test(){
	keyboard_init();
}
/*
 * page_fault_test_video
 * Description: Trigger exception when trying to access outside of video memory page
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS:  Cause exception
 * COVERAGE: Paging
 * FILES: paging.c
 */
void page_fault_test_viedo(){
  TEST_HEADER;
  unsigned int addr = 0xB7FFF;
  (*(int*)addr) = 0x1;
}

/*
 * page_fault_test_video
 * Description: Trigger exception when trying to access outside of kernel memory page
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS:  Cause exception
 * COVERAGE: Paging
 * FILES: paging.c
 */
void page_fault_test_kernel(){
  TEST_HEADER;
  unsigned int addr = 0xC0001;
  (*(int*)addr) = 0x1;
}

/*
 * page_value_test_video
 * Description: print the first value of viedo mem onto the screen
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS:  print the first value of viedo mem
 * COVERAGE: Paging
 * FILES: paging.c
 */
int paging_value_test_viedo(){
	TEST_HEADER;
	int result = FAIL;
	/* 0xB8000 is the start of video mem */
	int* i = (int*)0xB8000;
	int j = *i;
	//clear();
	printf("The first value in video memory is %x\n", j);
	result = PASS;
	return result;
}

/*
 * page_value_test_kernel
 * Description: print the first value of kernel space onto the screen
 * INPUTS: None
 * OUTPUTS: None
 * SIDE EFFECTS:  print the first value of kernel space
 * COVERAGE: Paging
 * FILES: paging.c
 */
int paging_value_test_kernel(){
	TEST_HEADER;
	int result = FAIL;
	/* 1048576*4 is the first address in kernel */
	int* i = (int*)(1048576 * 4);
	int j = *i;
	printf("The first value in kernel space is %x\n", j);
	result = PASS;
	return result;
}


/* Checkpoint 2 tests */

/*  read_file_test
 *  Description: test whether read a file given a name
 *  input: none
 *  output: PASS / FAIL
 *  side effect: none
*/
int read_file_test() {
	TEST_HEADER;
    dentry_t dentry;
    if(read_dentry_by_name((uint8_t*)"frame0.txt", &dentry) == 0) return PASS;
	return FAIL;
}

/*  read_file_test
 *  Description: test whether read a file given an index
 *  input: none
 *  output: PASS / FAIL
 *  side effect: none
*/
int read_idx_test() {
	TEST_HEADER;
	dentry_t dentry;
	if(read_dentry_by_index(0, &dentry) == -1) return FAIL;
	if(dentry.file_type != 1) return FAIL;
	if(dentry.inode_num != 0) return FAIL;
	return PASS;
}

/*  list_dir
 *  Description: list all files
 *  input: none
 *  output: PASS / FAIL
 *  side effect: print all file names
*/
int list_dir() {
	TEST_HEADER;
	char buf[MAX_FILE_NAME_LEN + 1];
	int32_t EOL;	// end of line
	int i;
	for(i = 0; i < MAX_FILE_NAME_LEN; i++) {
		EOL = read_dir(i, buf, MAX_FILE_NAME_LEN);
		if(EOL == 0) break;
		if(EOL > MAX_FILE_NAME_LEN) return FAIL;
		if(EOL < 0) return FAIL;
		buf[EOL] = '\0';
		printf(buf);
		printf("\n");
	}
	return PASS;
}

/*  print_file_content
 *  Description: print file content in small file
 *  input: none
 *  output: none
 *  side effect: print frame0.txt
*/
void print_file_content() {
	dentry_t* den;
	read_dentry_by_name((uint8_t*)"frame0.txt", den);
	char buf[6000];
	read_data(den->inode_num, 0, buf, 187);
	printf(buf);
}

/*  print_file_content
 *  Description: print file content in exe
 *  input: none
 *  output: none
 *  side effect: print grep
*/
void print_file_content2() {
	dentry_t* den;
	read_dentry_by_name((uint8_t*)"grep", den); // change file name to print different file content
	char buf[60000];
	read_data(den->inode_num, 0, buf, 6149);
	int i;
	//printf(buf);
	for (i = 0; i < 6149; i++) {
		if (buf[i] != NULL)
			putc(buf[i]);
	}
}

/* rtc_open_test
 * function for testing if rtc is open successfully
 * INPUTS: None
 * OUTPUTS: PASS if success, FAIL if fail
 * SIDE EFFECTS:  None
 * COVERAGE: rtc_opener
 * FILES: rtc_handler.c and rtc_handler.h
 */
// int rtc_open_test() {
// 	rtc_init();
// 	int ret = rtc_opener();
// 	int time1, time2;
// 	for (time1 = 1; time1 <= 40000; time1++) {
// 		for (time2 = 1; time2 <= 40000; time2++) {

// 		}
// 	}
// 	rtc_stop_interrupt();
// 	clear();
// 	if (ret != 0) return FAIL;
// 	return PASS;
// }

/* rtc_close_test
 * function for testing if rtc is close successfully
 * INPUTS: None
 * OUTPUTS: PASS if success, FAIL if fail
 * SIDE EFFECTS:  None
 * COVERAGE: rtc_closer
 * FILES: rtc_handler.c and rtc_handler.h
 */
// int rtc_close_test() {
// 	rtc_init();
// 	int ret = rtc_closer();
// 	rtc_stop_interrupt();
// 	clear();
// 	if (ret != 0) return FAIL;
// 	return PASS;
// }

/* rtc_write_test
 * function for testing if rtc is writing successfully
 * INPUTS: None
 * OUTPUTS: PASS if success, FAIL if fail
 * SIDE EFFECTS:  None
 * COVERAGE: rtc_write
 * FILES: rtc_handler.c and rtc_handler.h
 */
// int rtc_write_test() {
// 	rtc_init();
// 	int32_t* temp_buf;
// 	int32_t freq = 2;
// 	temp_buf = &freq;
// 	int i;
// 	for (i = 0; i < 7; i++) {
// 		rtc_write(0, temp_buf, 4);
// 		*temp_buf *= 2;
// 		int time1, time2;
// 		for (time1 = 1; time1 <= 40000; time1++) {
// 			for (time2 = 1; time2 <= 40000; time2++) {

// 			}
// 		}
// 	}
// 	clear();
// 	if (rtc_write(0, NULL, 4) != -1) return FAIL;
// 	if (rtc_write(0, temp_buf, 5) != -1) return FAIL;
// 	*temp_buf = 2048;
// 	if (rtc_write(0, temp_buf, 4) != -1) return FAIL;
// 	rtc_stop_interrupt();
// 	clear();

// 	return PASS;
// }

/* rtc_read_test
 * function for testing if rtc is reading successfully
 * INPUTS: None
 * OUTPUTS: PASS if success, FAIL if fail
 * SIDE EFFECTS:  None
 * COVERAGE: rtc_read
 * FILES: rtc_handler.c and rtc_handler.h
 */
// int rtc_read_test() {
// 	rtc_init();
// 	rtc_read(0, NULL, 4);
// 	rtc_stop_interrupt();
// 	clear();

// 	return PASS;
// }
/* Open/close terminal test
 *
 * Activate opening/closing terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage:
 * Files: terminal.c/h
 */
// int terminal_open_close_test(){
// 	int32_t * fd;
// 	if(terminal_open(fd) == 0 && terminal_close(fd) == 0){
// 		return PASS;
// 	}
// 	return FAIL;
// }

/* Write (null) test
 *
 * Activate Write to terminal
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: prints what is written
 * Coverage:
 * Files: terminal.c/h
 */
// int terminal_write_test(){
//
// 	char buf[5] = "\0hhh";
//
// 	if (4 == terminal_write(buf, 4))
// 		return PASS;
//
// 	return FAIL;
// }

/* Read test
 *
 * Keyboard reading
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Allows keyboard reading
 * Coverage:
 * Files: terminal.c/h
 */
// int terminal_read_test(){
//
// 	char buf[128];
//
// 	while (1) {
// 		terminal_read(buf, 128);
// 	}
// }

/* Write overflow test
 *
 * Try to write 199 chars
 * Inputs: None
 * Outputs: None
 * Side Effects: prints 127 chars
 * Coverage:
 * Files: terminal.c/h
 */
// int terminal_overflow_test(){
//
// 	char buf[200] = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
//
// 	if (127 == terminal_write(buf, 200))
// 		return PASS;
//
// 	return FAIL;
// }



/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	/* CP 1: */
	//TEST_OUTPUT("paging_value_test_viedo", paging_value_test_viedo());
	//TEST_OUTPUT("paging_value_test_kernel", paging_value_test_kernel());
	//page_fault_test_viedo();
	//page_fault_test_kernel();
	//divide_test();
	//opcode_test();
	//null_test();
  	//rtc_test();
  	//keyboard_test();

	/* CP 2: */
	//TEST_OUTPUT("read_file_test", read_file_test());
	//TEST_OUTPUT("read_idx_test", read_idx_test());
	//TEST_OUTPUT("list_dir", list_dir());
	//print_file_content();
	//print_file_content2();

// 	//TEST_OUTPUT("rtc_open_test", rtc_open_test());
// 	// The nested loop below is to have a delay show the full rtc test
// 	int time1, time2;
// 	for (time1 = 1; time1 <= 40000; time1++) {
// 		for (time2 = 1; time2 <= 40000; time2++) {

// 		}
// 	}
// 	//TEST_OUTPUT("rtc_close_test", rtc_close_test());
// 	// The nested loop below is to have a delay show the full rtc test
// 	for (time1 = 1; time1 <= 40000; time1++) {
// 		for (time2 = 1; time2 <= 40000; time2++) {

// 		}
// 	}
// 	//TEST_OUTPUT("rtc_write_test", rtc_write_test());
// 	// The nested loop below is to have a delay show the full rtc test
// 	for (time1 = 1; time1 <= 40000; time1++) {
// 		for (time2 = 1; time2 <= 40000; time2++) {

// 		}
// 	}
// //	TEST_OUTPUT("rtc_read_test", rtc_read_test());
// 	// The nested loop below is to have a delay show the full rtc test
// 	for (time1 = 1; time1 <= 40000; time1++) {
// 		for (time2 = 1; time2 <= 40000; time2++) {

// 		}
// 	}

	// TEST_OUTPUT("terminal open/close test",terminal_open_close_test());
	// TEST_OUTPUT("terminal_write_test", terminal_write_test());
	// TEST_OUTPUT("terminal_read_test", terminal_read_test());
	// TEST_OUTPUT("terminal_overflow_test", terminal_overflow_test());

}
