/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask=0xFF; /* IRQs 0-7  */
uint8_t slave_mask=0xFF;  /* IRQs 8-15 */

#define PIC_SIZE    8

/* Initialize the 8259 PIC */
void i8259_init(void) {
  // 1
  outb(ICW1, MASTER_8259_PORT);
  outb(ICW1, SLAVE_8259_PORT);
  // 2
  outb(ICW2_MASTER, MASTER_8259_PORT_D);
  outb(ICW2_SLAVE, SLAVE_8259_PORT_D);
  // 3
  outb(ICW3_MASTER, MASTER_8259_PORT_D);
  outb(ICW3_SLAVE, SLAVE_8259_PORT_D);
  // 4
  outb(ICW4, MASTER_8259_PORT_D);
  outb(ICW4, SLAVE_8259_PORT_D);
  // Enable slave on IRQ 2
  enable_irq(ICW3_SLAVE);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
  /* IRQ 0-7 is master, 8-15 is slave */
  // Check if valid
	if ((irq_num > 15) || (irq_num < 0))return;

  uint8_t mask = 0xFE;  /* setup mask */

  if(irq_num < PIC_SIZE) {     /* handle master */
		//Shift mask
		int b;
		for (b = 0; b < irq_num; b++) {
			mask = (mask << 1) + 1;
		}
		master_mask = mask & master_mask;
		outb(master_mask, MASTER_8259_PORT_D);
		return;
  }
  else {              /* handle slave */
		irq_num = irq_num - 8; //Shift into 0-8
		// Shift mask
		int b;
		for (b = 0; b < irq_num; b++) {
			mask = (mask << 1) + 1;
		}
		slave_mask = mask & slave_mask;
		outb(slave_mask, SLAVE_8259_PORT_D);
		return;
  }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  // Check if valid
	if ((irq_num > 15) || (irq_num < 0))return;

  uint8_t mask = 0x01;  /* setup mask */

  if(irq_num < PIC_SIZE) {     /* handle master */
		//Shift mask
		int b;
		for (b = 0; b < irq_num; b++) {
			mask = (mask << 1);
		}
		master_mask = mask | master_mask;
		outb(master_mask, MASTER_8259_PORT_D);
		return;
  }
  else {              /* handle slave */
		irq_num = irq_num - 8; //Shift into 0-8
		// Shift mask
		int b;
		for (b = 0; b < irq_num; b++) {
			mask = (mask << 1);
		}
		slave_mask = mask | slave_mask;
		outb(slave_mask, SLAVE_8259_PORT_D);
		return;
  }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
  if ((irq_num>=0)&&(irq_num < PIC_SIZE))
    outb(EOI|irq_num, MASTER_8259_PORT);
  else if ((irq_num>=PIC_SIZE)&&(irq_num < PIC_SIZE*2)) {
    outb(EOI|(irq_num-PIC_SIZE), SLAVE_8259_PORT);
    outb(EOI + ICW3_SLAVE, MASTER_8259_PORT);
  }
}
