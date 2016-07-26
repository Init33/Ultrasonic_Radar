/*_____________________________________________________________________________________________________________________

    Project: Ultrasonic Radar       
    Author: Jesse Nolan    
    Year: 2014           

    Description:
_______________________________________________________________________________________________________________________*/

#ifndef A3_XX_h
#define A3_XX_h

/*-------------------------------
        Global Variables
--------------------------------*/
sbit PB1 = P1 ^ 0		// Pushbutton PB1
sbit PB2 = P1 ^ 1;		// Pushbutton PB2
sbit PB3 = P1 ^ 2; 		// Pushbutton PB3   
sbit PB4 = P1 ^ 3;		// Pushbuttom PB4
sbit PB5 = P1 ^ 4;		// Pushbutton PB5
sbit PB6 = P1 ^ 5;		// Pushbutton PB6
sbit PB7 = P1 ^ 6;		// Pushbutton PB7   
sbit PB8 = P1 ^ 7;		// Pushbuttom PB8

sbit SMODE = P3 ^ 7;	// Pushbutton on F120 development board

sbit LD0 = P2 ^ 0;     	// LD0   
sbit LD1 = P2 ^ 1;  	// LD1
sbit LD2 = P2 ^ 2;  	// LD2
sbit LD3 = P2 ^ 3;  	// LD3  
sbit LD4 = P2 ^ 4;  	// LD4   
sbit LD5 = P2 ^ 5;  	// LD5
sbit LD6 = P2 ^ 6;  	// LD6
sbit LD7 = P2 ^ 7;  	// LD7    

sfr	 LCD = 0xB0;

sbit DB4 	= P3 ^ 0;	// LCD Pins
sbit DB5 	= P3 ^ 1;
sbit DB6 	= P3 ^ 2;
sbit DB7 	= P3 ^ 3;
sbit RS 	= P3 ^ 4;
sbit RW 	= P3 ^ 5;
sbit E  	= P3 ^ 6;
sbit BL 	= P3 ^ 7;

sbit USonicTX = P0 ^ 2;		// TX for ultrasonic
sbit USonicRX = P0 ^ 3;		// RX 
sbit Servo_Ctrl = P0 ^ 4;	// Servo control pin

*---------------------------------------------------
          	Function prototypes
----------------------------------------------------*/
void main(void);
void General_Init(void);
void Timer_Init();
void LCD_Init(void);
void Interrupts_Init();
void External_INT0_ISR(void);

#endif  
