#include <linux/head.h>
#include <asm/system.h>
#include <etc/simple_print.h>
void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
//void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
//void page_fault(void);
//void coprocessor_error(void);
void reserved(void);
//void parallel_interrupt(void);
void irq13(void);

void do_double_fault(long esp, long error_code)
{
    simple_print("double_fault");
}

void do_general_protection(long esp, long error_code)
{
	simple_print("general_protection");
}

void do_divide_error(long esp, long error_code)
{
	simple_print("divide_error");
}

void do_int3(long * esp, long error_code,
		long fs,long es,long ds,
		long ebp,long esi,long edi,
		long edx,long ecx,long ebx,long eax)
{
	simple_print("int_3");
}

void do_nmi(long esp, long error_code)
{
	simple_print("nmi");
}

void do_debug(long esp, long error_code)
{
	simple_print("debug");
}

void do_overflow(long esp, long error_code)
{
	simple_print("overflow");
}

void do_bounds(long esp, long error_code)
{
	simple_print("bounds");
}

void do_invalid_op(long esp, long error_code)
{
	simple_print("invalid_op");
}

// void do_device_not_available(long esp, long error_code)
// {
// 	simple_print("device_not_available");
// }

void do_coprocessor_segment_overrun(long esp, long error_code)
{
	simple_print("coprocessor_segment_overrun");
}

void do_invalid_TSS(long esp,long error_code)
{
    simple_print("invalid_tss");	
}

void do_segment_not_present(long esp,long error_code)
{
	simple_print("segment_not_present");
}

void do_stack_segment(long esp,long error_code)
{
	simple_print("stack_segment");
}

 void do_coprocessor_error(long esp, long error_code)
 {
 	simple_print("coprocessor_error");
 }

void do_reserved(long esp, long error_code)
{
	simple_print("reserved");
}

void trap_init(void)
{
    int i;

	set_trap_gate(0,&divide_error);
	set_trap_gate(1,&debug);
	set_trap_gate(2,&nmi);
	set_system_gate(3,&int3);	/* int3-5 can be called from all */
	set_system_gate(4,&overflow);
	set_system_gate(5,&bounds);
	set_trap_gate(6,&invalid_op);
	//set_trap_gate(7,&device_not_available);
	set_trap_gate(8,&double_fault);
	set_trap_gate(9,&coprocessor_segment_overrun);
	set_trap_gate(10,&invalid_TSS);
	set_trap_gate(11,&segment_not_present);
	set_trap_gate(12,&stack_segment);
	set_trap_gate(13,&general_protection);
	//set_trap_gate(14,&page_fault);
	set_trap_gate(15,&reserved);
	//set_trap_gate(16,&coprocessor_error);
	for (i=17;i<48;i++)
		set_trap_gate(i,&reserved);
	set_trap_gate(45,&irq13);
	outb_p(inb_p(0x21)&0xfb,0x21);
	outb(inb_p(0xA1)&0xdf,0xA1);
	//set_trap_gate(39,&parallel_interrupt);
}