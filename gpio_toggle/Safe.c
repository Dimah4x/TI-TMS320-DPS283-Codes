/***********************************************
* Title: Micro Proccesors Project
* Filename: Source1.c *
* Date: 09.06.2021 *
* *
* ***safe with changeable password****
*
* Author: Avi Bents	313298341
* 
*
*********************************************/

#include <stdio.h>
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File

#define KB_BUFFER_SIZE 64 //max data from keypad
#define First_Digit_hidden string_array[0]
#define Second_Digit_hidden string_array[1]
#define Third_Digit_hidden string_array[2]
#define Fourth_Digit_hidden string_array[3]

//struct CPUTIMER_VARS CpuTimer0;
struct { //structure for interrupt for 64bit write data (clist)
	char Data[KB_BUFFER_SIZE];
	int Haed;
	int Tail;
} KBCyclicBuffer = { {0}, 0, 0 };

//interrupts
interrupt void cpu_timer0_isr(void); //cpu timer interrupt
interrupt void Xint3456_isr(void); //keypad interrupt

//LCD functions
void LCD_Control(int bit);
void LCD_Enable(int bit);
void LCD_Blink(int bit);
void LCD_WriteData(char data);
void LCD_Backlight(int x);
void LCD_Putc(const char c);
void LCD_Print(const char *str);
void LCD_Clear(void);
void LCD_Init(void);
void LCD_Home();
void LCD_OnOff(char D, char C, char B);
void LCD_GoTo(char y, char x);
void LCD_PrintLine(char line, const char *str);

//keypad functions
inline static void KeboardWriteCode(char data);
inline static char KeboardReadCode();
inline void Beep(int MiliSec);
static char scan2ascii(char sc);
char ReadFromKB(char wait);
char PushButtonRead(int Inx);
void ConfigAndInstallKBInt();

//peripherals
void delay_ms(int ms); //delay in milisec
void Gpio_select(void); // gpio slection
int CharToInt(char c); //convert char to int
char IntToChar(int num); //convert int to char
void correct_beep();
void wrong_beep();

//global variables
int temp = 0; // variable for the delay function
char welcome[] = "enter code      "; //first msg on lcd
static const char EmptyLine[] = "                "; // clear line
static const char no_digits[] = "      ----      ";
static const char Entered_Correct_Code[] = "Safe is open now";
static const char Entered_Wrong_Code_1stline[] = "  !Wrong code!  ";
static const char Entered_Wrong_Code_2ndline[] = "Press # to retry";
static const char change_code_string[] = "change code?    ";
static const char change_code_string_2ndline[] = "YES-1   NO-other";
static const char enter_new_code[] = "enter new code: ";
char string_array[4][17] = {"      *---      ",
							"      **--      ",
							"      ***-      ",
							"      ****      " };

void main(){

	// Step 1. Initialize System Control:
	// PLL, WatchDog, enable Peripheral Clocks
	// This example function is found in the DSP2833x_SysCtrl.c file.
	InitSysCtrl();
	// initialize gpio and keypad interrupts
	Gpio_select();
	// Step 3. Clear all interrupts and initialize PIE vector table:
	// Disable CPU interrupts
	DINT;
	// Initialize PIE control registers to their default state.
	// The default state is all PIE interrupts disabled and flags
	// are cleared.
	// This function is found in the DSP2833x_PieCtrl.c file.
	InitPieCtrl();
	IER = 0x0000;
	IFR = 0x0000;
	//initialize cpu timer
	InitCpuTimers();
	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
	// This function is found in DSP2833x_PieVect.c.
	InitPieVectTable();
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = &cpu_timer0_isr; //get addy from cpu timer function and set the PIE vector
	EDIS;    // This is needed to disable write to EALLOW protected registers
	ConfigCpuTimer(&CpuTimer0, 150, 1000); //interrupts every milisec on a 150mhz clock
	CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	IER |= M_INT1; //interrupt from group 1
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	//User specific code:
	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM
	 //this where the real main starts
	CpuTimer0Regs.TCR.bit.TSS = 1; // STOP TIMER0
	CpuTimer0.InterruptCount = 0; // reset for first count
	// initialize lcd
	LCD_Backlight(1);
	LCD_Init();
	LCD_Home();
	long int entered_code = 0x00000000, new_code, correct_code = 0xE0D0B0E1; //code of 1234 as inputs from keypad (default)
	char First_Digit ,Second_Digit ,Third_Digit ,Fourth_Digit ,retry_pressed ,want_new_code;
	while (1) {
retry:
		First_Digit = 0;
		Second_Digit = 0;
		Third_Digit = 0;
		Fourth_Digit = 0;
		retry_pressed = 0;
		want_new_code = 0;
		entered_code = 0x00000000;
		new_code = 0x00000000;
		LCD_Clear(); //clean
		LCD_GoTo(0, 0); //home
		LCD_Print(welcome); //be nice
		LCD_GoTo(1, 0);
		LCD_Print(no_digits);
		delay_ms(1);
		First_Digit = ReadFromKB(1);
		if (First_Digit != 0) {
			LCD_Print(First_Digit_hidden);
			entered_code |= First_Digit; //bitwise operations between different types should work
			entered_code = entered_code << 8; //copy and shift by 8 to make space for next digit
		}
		LCD_GoTo(1, 0);
		Second_Digit = ReadFromKB(1);
		if (Second_Digit != 0){
			LCD_Print(Second_Digit_hidden);
			entered_code |= Second_Digit; 
			entered_code = entered_code << 8;
		}
		LCD_GoTo(1, 0);
		Third_Digit = ReadFromKB(1);
		if (Third_Digit != 0) {
			LCD_Print(Third_Digit_hidden);
			entered_code |= Third_Digit; 
			entered_code = entered_code << 8;
		}
		LCD_GoTo(1, 0);
		Fourth_Digit = ReadFromKB(1);
		if (Fourth_Digit != 0) {
			LCD_Print(Fourth_Digit_hidden);
			entered_code |= Fourth_Digit; 
		}
		if (entered_code == correct_code) {
			correct_beep();
			LCD_Clear();
			LCD_Home();
			LCD_Print(Entered_Correct_Code);
			delay_ms(1000);
			LCD_Clear();
			LCD_Print(change_code_string); 
			LCD_GoTo(1, 0);
			LCD_Print(change_code_string_2ndline);
			want_new_code = ReadFromKB(1); //check with user if wants to change code
			if(want_new_code == 0xE0){
				LCD_Clear();
				LCD_Print(enter_new_code);
				if (entered_code == correct_code) {
					int i;
					for (i = 0; i < 4; i++) { //input 4 times for new code
						want_new_code = ReadFromKB(1);
						new_code |= want_new_code;
						LCD_GoTo(1, 0);
						LCD_Print(string_array[i]); //print the corresponding digit as hidden
						if(i!=3)
							new_code = new_code << 8; //dont want to shift last one
					}
					correct_code = new_code; //set new code
				}
			}
			goto retry;
		}
		else {
			wrong_beep();
			LCD_Clear();
			LCD_Home();
			LCD_Print(Entered_Wrong_Code_1stline);
			LCD_GoTo(1, 0);
			LCD_Print(Entered_Wrong_Code_2ndline);
			while (retry_pressed != 0xB3) //check if # was pressed
				retry_pressed = ReadFromKB(1);
			goto retry;
		}
	}
	//SPECIAL long int special_pass = 0x00A9190B
}
//
//interrupts
//
interrupt void cpu_timer0_isr(void) {
	CpuTimer0.InterruptCount++;
	// Acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void Xint3456_isr(void) {
	KBCyclicBuffer.Data[KBCyclicBuffer.Haed++ % KB_BUFFER_SIZE] = ReadFromKB(0);
	// Acknowledge this interrupt to receive more interrupts from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
//
//peripherals
//
void delay_ms(int ms) {
	CpuTimer0Regs.TCR.bit.TSS = 0; // START TIMER0
	while (CpuTimer0.InterruptCount<ms) {}
	CpuTimer0Regs.TCR.bit.TSS = 1; // STOP TIMER0
	CpuTimer0.InterruptCount = 0; // reset after stopping the count
}

void Gpio_select(void) {
	EALLOW;
	GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;
	GpioCtrlRegs.GPCMUX1.all = 0x00000000;  // All GPIO
	GpioCtrlRegs.GPCMUX2.all = 0x00000000;  // All GPIO
	GpioCtrlRegs.GPBDIR.all = 0x07FF0F00; //outputs for lcd and 4 keypad bits, if keypad wont work correctly switch to 0x07FF0000
	EDIS;
	ConfigAndInstallKBInt();
}

void correct_beep() {
	Beep(1000);
}

void wrong_beep() {
	Beep(100);
	delay_ms(100);
	Beep(100);
	delay_ms(100);
	Beep(100);
	delay_ms(100);
}
//
//keypad functions
//
inline static void KeboardWriteCode(char data) {
	GpioDataRegs.GPBCLEAR.all = (0x0FL << 8); // Clear 4 data bits GPIO40-GPIO43
	GpioDataRegs.GPBSET.all = ((long)data << 8); // Set the relevant data bits GPIO40-GPIO43
	delay_ms(1);
}

inline static char KeboardReadCode() {
	return((GpioDataRegs.GPBDAT.all >> 12) & 0x0FL); // Read 4 data bits GPIO44-GPIO47
}

inline void Beep(int MiliSec) {
	GpioDataRegs.GPASET.bit.GPIO27 = 1; // Buzzer On
	delay_ms(MiliSec);
	GpioDataRegs.GPACLEAR.bit.GPIO27 = 1; // Buzzer Off
}

static char scan2ascii(char sc) {
	switch (sc) {
	case 0xE0: return('1'); //first row then colomn
	case 0xD0: return('2');
	case 0xB0: return('3');
	case 0x70: return('A');
	case 0xE1: return('4');
	case 0xD1: return('5');
	case 0xB1: return('6');
	case 0x71: return('B');
	case 0xE2: return('7');
	case 0xD2: return('8');
	case 0xB2: return('9');
	case 0x72: return('C');
	case 0xE3: return('*');
	case 0xD3: return('0');
	case 0xB3: return('#');
	case 0x73: return('D');
	}
	return(0);
}

char ReadFromKB(char wait) {
	static char code[] = { 0xE, 0xD, 0xB, 0x7 };
	char data;
	char i;
	KeboardWriteCode(0x0);
	delay_ms(1);
	while (KeboardReadCode() == 0x0F) // Check 4 data bits GPIO44-GPIO47
		if (!wait) return(0);
	Beep(20);
	for (i = 0; i < 4; i++) {
		KeboardWriteCode(code[i]);
		delay_ms(1);
		data = KeboardReadCode();
		if (data != 0x0F)
			break;
	}
	while (KeboardReadCode() != 0x0F); // Wait for button release
	delay_ms(1);
	KeboardWriteCode(0x0);
	return((data << 4) | i); //return code of pressed key from kb
}

char PushButtonRead(int Inx) {
	switch (Inx) {
	case 0:
		return (GpioDataRegs.GPCDAT.bit.GPIO80);
	case 1:
		return (GpioDataRegs.GPCDAT.bit.GPIO81);
	case 2:
		return (GpioDataRegs.GPCDAT.bit.GPIO82);
	case 3:
		return (GpioDataRegs.GPCDAT.bit.GPIO83);
	}
	return(0);
}

void ConfigAndInstallKBInt() {
	EALLOW; 
	// Set input qualification period for GPIO44-GPIO47
	GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 1; // Qual period = SYSCLKOUT/2
	GpioCtrlRegs.GPBQSEL1.bit.GPIO44 = 2; // 6 samples
	GpioCtrlRegs.GPBQSEL1.bit.GPIO45 = 2; // 6 samples
	GpioCtrlRegs.GPBQSEL1.bit.GPIO46 = 2; // 6 samples
	GpioCtrlRegs.GPBQSEL1.bit.GPIO47 = 2; // 6 samples
	GpioIntRegs.GPIOXINT3SEL.all = 12; // Xint3 connected to GPIO44 32+12
	GpioIntRegs.GPIOXINT4SEL.all = 13; // Xint4 connected to GPIO45 32+13
	GpioIntRegs.GPIOXINT5SEL.all = 14; // Xint5 connected to GPIO46 32+14
	GpioIntRegs.GPIOXINT6SEL.all = 15; // Xint6 connected to GPIO47 32+15
	PieVectTable.XINT3 = &Xint3456_isr;
	PieVectTable.XINT4 = &Xint3456_isr;
	PieVectTable.XINT5 = &Xint3456_isr;
	PieVectTable.XINT6 = &Xint3456_isr;
	EDIS; 
	IER |= M_INT12; // interrupt group 12
	// Enable Xint 3,4,5,6 in the PIE: Group 12 interrupt 1-4
	// Enable int1 which is connected to WAKEINT:
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enable the PIE block
	PieCtrlRegs.PIEIER1.bit.INTx5 = 1;
	PieCtrlRegs.PIEIER12.bit.INTx1 = 1; // Enable PIE Group 12 INT1
	PieCtrlRegs.PIEIER12.bit.INTx2 = 1; // Enable PIE Group 12 INT2
	PieCtrlRegs.PIEIER12.bit.INTx3 = 1; // Enable PIE Group 12 INT3
	PieCtrlRegs.PIEIER12.bit.INTx4 = 1; // Enable PIE Group 12 INT4
	// Configure XINT3-6
	XIntruptRegs.XINT3CR.bit.POLARITY = 0; // Falling edge interrupt
	XIntruptRegs.XINT4CR.bit.POLARITY = 0; // Falling edge interrupt
	XIntruptRegs.XINT5CR.bit.POLARITY = 0; // Falling edge interrupt
	XIntruptRegs.XINT6CR.bit.POLARITY = 0; // Falling edge interrupt
	// Enable XINT3-6
	XIntruptRegs.XINT3CR.bit.ENABLE = 1; // Enable Xint3
	XIntruptRegs.XINT4CR.bit.ENABLE = 1; // Enable XINT4
	XIntruptRegs.XINT5CR.bit.ENABLE = 1; // Enable Xint5
	XIntruptRegs.XINT6CR.bit.ENABLE = 1; // Enable XINT6
}
//
//lcd functions
//
void LCD_Control(int bit) {
	if (bit)
		GpioDataRegs.GPBSET.bit.GPIO57 = 1;
	else
		GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1;
}

void LCD_Enable(int bit) {
	if (bit)
		GpioDataRegs.GPBSET.bit.GPIO56 = 1;
	else
		GpioDataRegs.GPBCLEAR.bit.GPIO56 = 1;
}

void LCD_Blink(int bit) {
	if (bit)
		GpioDataRegs.GPBSET.bit.GPIO58 = 1;
	else
		GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1;
}

void LCD_WriteData(char data) {
	GpioDataRegs.GPBCLEAR.all = (0xFFL << 16);	// Clear all data bits GPIO48-GPIO55
	GpioDataRegs.GPBSET.all = ((long)data << 16);	// Set the relevant data bits GPIO48-GPIO55
}

void LCD_Backlight(int x) {
	LCD_Blink(x);
}

void LCD_Putc(const char c) {
	LCD_Control(1);		// Data
	delay_ms(1);
	LCD_Enable(1);
	LCD_WriteData(c);
	delay_ms(1);
	LCD_Enable(0);
	delay_ms(1);
}

void LCD_Print(const char *str) {
	int i;
	for (i = 0; str[i]; i++)
	{
		LCD_Putc(str[i]);
	}
}

void LCD_Clear(void) {
	LCD_Control(0);		// Control
	delay_ms(1);
	LCD_Enable(1);
	LCD_WriteData(0x01);	// Clear code
	delay_ms(1);
	LCD_Enable(0);
	delay_ms(10);
}

void LCD_Init(void) {
	static const char LcdInitStr[] = { 0x38, 0x0E, 0x06, 0x01 };
	int i;
	LCD_Control(0);		// Control
	for (i = 0; i < 4; i++)
	{
		LCD_Enable(1);
		LCD_WriteData(LcdInitStr[i]);
		delay_ms(1);
		LCD_Enable(0);
		delay_ms(5);
	}
}

void LCD_Home() {
	LCD_Control(0);		// Control
	delay_ms(1);
	LCD_Enable(1);
	LCD_WriteData(0x02);	// Home Code
	delay_ms(1);
	LCD_Enable(0);
	delay_ms(1);
}

void LCD_OnOff(char D, char C, char B) {
	LCD_Control(0);		// Control
	delay_ms(1);
	LCD_Enable(1);
	LCD_WriteData(0x08 | (D << 2) | (C << 1) | (B << 0));	// DCB Code
	delay_ms(1);
	LCD_Enable(0);
	delay_ms(1);
}

void LCD_GoTo(char y, char x) {
	LCD_Control(0);		// Control
	delay_ms(1);
	LCD_Enable(1);
	LCD_WriteData((1 << 7) | (0x40 * y + x));	// Set DDRAM Address
	delay_ms(1);
	LCD_Enable(0);
	delay_ms(1);
}

void LCD_PrintLine(char line, const char *str) {
	LCD_GoTo(line, 0);
	LCD_Print(EmptyLine);
	LCD_GoTo(line, 0);
	LCD_Print(str);
}