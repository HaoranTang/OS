#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
// #include "keyboardhandler.h"
#include "rtc_handler.h"
#include "keyboard.h"
#include "interrupt_wrapper.h"
// #include "syshandler.h"
// #include "handlers.h"
#include "sys_call.h"

/*
 * Exception handler
 *
 * Description: Prints the exception type and halt the program by the loop
 * Inputs: exception -- the exception type
           message -- the message printed on screen
 * Outputs: none
 * Side effects: halt the program
 */
#define EXCEPTION_HANDLER(exception,message)	\
void exception() {				  \
	printf("%s\n", #message);	\
	halt(255);				          \
}


/*
 * undefined handler
 *
 * Description: Prints the message when encountered undefined interrupt
 * Inputs: none
 * Outputs: none
 * Side effects: mask the interrupt flag until the message is printed
 */
void undefined_handler() {
  cli();
	clear();
  printf("undefined interrupt");
  sti();
}



// Exceptions handler
EXCEPTION_HANDLER(DE,"Divide Error");
EXCEPTION_HANDLER(DB,"Debug");
EXCEPTION_HANDLER(NMI,"Non-Maskable Interrupt");
EXCEPTION_HANDLER(BP,"Breakpoint");
EXCEPTION_HANDLER(OF,"Overflow");
EXCEPTION_HANDLER(BR,"BOUND Range Exceeded");
EXCEPTION_HANDLER(UD,"Invalid Opcode");
EXCEPTION_HANDLER(NM,"Device Not Available");
EXCEPTION_HANDLER(DF,"Double Fault");
EXCEPTION_HANDLER(CSO,"Coprocessor Segment Overrun");
EXCEPTION_HANDLER(TS,"Invalid TSS");
EXCEPTION_HANDLER(NP,"Segment Not Present");
EXCEPTION_HANDLER(SS,"Stack-Segment Fault");
EXCEPTION_HANDLER(GP,"General Protection");
EXCEPTION_HANDLER(PF,"Page Fault");
EXCEPTION_HANDLER(MF,"FPU Floating-Poin");
EXCEPTION_HANDLER(AC,"Alignment Check");
EXCEPTION_HANDLER(MC,"Machine Check");
EXCEPTION_HANDLER(XF,"SIMD Floating-Point");
/*
 * idt_init
 *
 * Description: Initializes the IDT table
 * Inputs: none
 * Outputs: none
 * Side effect: Initializes IDT table and coresponding entry
 */
void idt_init() {
  int i;
  for(i = 0; i < NUM_VEC; i++){
    // set present to 1
    idt[i].present = 1;

    // all dpl should be 0 except for the sys call(index 0x80)
    if (i == 0x80) {
      idt[i].dpl = 3;
    } else {
      idt[i].dpl = 0;
    }
    // General interrupt = 01100 (index 32-256), General exception = 01110
    idt[i].reserved0 = 0;
		idt[i].reserved1 = 1;
		idt[i].reserved2 = 1;
    idt[i].reserved3 = 1;
		idt[i].reserved4 = 0;
    // size = 1
    idt[i].size = 1;
    // Selector is kernel
		if (i >= 32) {
      	idt[i].reserved3 = 0;
				SET_IDT_ENTRY(idt[i], undefined_handler);
    }
    idt[i].seg_selector = KERNEL_CS;
  }

  // Set interrupt 0 through interrupt 19
  SET_IDT_ENTRY(idt[0], DE);
  SET_IDT_ENTRY(idt[1], DB);
  SET_IDT_ENTRY(idt[2], NMI);
  SET_IDT_ENTRY(idt[3], BP);
  SET_IDT_ENTRY(idt[4], OF);
  SET_IDT_ENTRY(idt[5], BR);
  SET_IDT_ENTRY(idt[6], UD);
  SET_IDT_ENTRY(idt[7], NM);
  SET_IDT_ENTRY(idt[8], DF);
  SET_IDT_ENTRY(idt[9], CSO);
  SET_IDT_ENTRY(idt[10], TS);
  SET_IDT_ENTRY(idt[11], NP);
  SET_IDT_ENTRY(idt[12], SS);
  SET_IDT_ENTRY(idt[13], GP);
  SET_IDT_ENTRY(idt[14], PF);
  //15 is reserved by intel referenc(ISA manual table 5.11)
  SET_IDT_ENTRY(idt[16], MF);
  SET_IDT_ENTRY(idt[17], AC);
  SET_IDT_ENTRY(idt[18], MC);
  SET_IDT_ENTRY(idt[19], XF);

	SET_IDT_ENTRY(idt[0x21], keyboard_wrapper);
	SET_IDT_ENTRY(idt[0x28], rtc_wrapper);
    SET_IDT_ENTRY(idt[0x80], sys_wrapper);
}
