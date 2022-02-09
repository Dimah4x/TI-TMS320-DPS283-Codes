#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
/*
// Prototype statements for functions found within this file.
interrupt void cpu_timer0_isr(void);
void new_Delay(int sec);
void Buzzer_test();
void Gpio_select(void);
static long temp=0;
static long toggle = 0xA;


void main(void)
{
   InitSysCtrl();
   Gpio_select();	  
   DINT;
   InitPieCtrl();
   IER = 0x0000;
   IFR = 0x0000;
   InitPieVectTable();
   EALLOW;  // This is needed to write to EALLOW protected registers
   PieVectTable.TINT0 = &cpu_timer0_isr;
   EDIS;    // This is needed to disable write to EALLOW protected registers
   InitCpuTimers();   // For this example, only initialize the Cpu Timers
   ConfigCpuTimer(&CpuTimer0, 150, 1000000);
   CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
   IER |= M_INT1;
   IER |= M_INT13;
   IER |= M_INT14;
   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
   EINT;   // Enable Global interrupt INTM
   ERTM;   // Enable Global realtime interrupt DBGM
   while(1)
	Buzzer_test();
}

interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;
   temp++;
   // Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void new_Delay(int sec){
	temp = 0;
	while(temp!=0)
		temp = 0;
	while(temp<sec);

}

void Buzzer_test(){
	GpioDataRegs.GPASET.bit.GPIO27 = 1;
	new_Delay(1);
	GpioDataRegs.GPACLEAR.bit.GPIO27 = 1;
	new_Delay(1);
}

void Gpio_select(void)
{
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;
	GpioCtrlRegs.GPCMUX1.all = 0x00000000;  // All GPIO
	GpioCtrlRegs.GPCMUX2.all = 0x00000000;  // All GPIO
    GpioCtrlRegs.GPCDIR.all = 0x0000000F;   // All outputs 4 Leds
    EDIS;
}
*/
