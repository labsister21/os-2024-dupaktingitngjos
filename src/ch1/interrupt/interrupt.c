#include "interrupt.h"
#include "../keyboard/keyboard.h"

// Activate PIC mask for keyboard only
void activate_keyboard_interrupt(void){
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

// I/O port wait, around 1-4 microsecond, for I/O synchronization purpose
void io_wait(void){
    out(0x80, 0);
}

// Send ACK to PIC - @param irq Interrupt request number destination, note: ACKED_IRQ = irq+PIC1_OFFSET
void pic_ack(uint8_t irq){
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

// Shift PIC interrupt number to PIC1_OFFSET and PIC2_OFFSET (master and slave)
void pic_remap(void){
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

/**
 * Main interrupt handler when any interrupt / exception is raised.
 * DO NOT CALL THIS FUNCTION.
 * 
 * This function will be called first if any INT 0x00 - 0x40 is raised, 
 * and will call proper ISR for respective interrupt / exception.
 * 
 * If inter-privilege interrupt raised, SS and ESP is automatically out of main_interrupt_handler()
 * parameter. Can be checked with ((int*) info) + 4 for user $esp, 5 for user $ss
 * 
 * Again, this function is not for normal function call, all parameter will be automatically set when interrupt is called.
 * @param frame Information about CPU during interrupt is raised
 */

void main_interrupt_handler(struct InterruptFrame frame){
    switch (frame.int_number) {
        case (IRQ_KEYBOARD + PIC1_OFFSET):
            keyboard_isr();
            break;       
    }
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}
