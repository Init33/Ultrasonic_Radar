/*_____________________________________________________________________________________________________________________

    Project: Ultrasonic Radar       
    Author: Jesse Nolan    
    Year: 2014           

    Description: Scanning ultrasonic sensor used as a radar that scans a 180 degree area and maps detected objects on
                    an LCD screen. Optional alarm system that detects objects closer than a given threshold distance. 
                    Commands given to the 8051 board through UART using putty.
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
void Timer_Init(void);
void LCD_Init(void);
void Interrupts_Init(void);
void External_INT0_ISR(void);
void UART_Init(void);
void PCA_Init(void);
void Voltage_Reference_Init(void);
void DAC_Init(void);
void set_duty(unsigned short int duty);
void delay(unsigned char H,unsigned char L);
void ping(void);
void LCD_display(char string[]);
void LCD_clear(void);
void radar_calc(void);
void LCD_shift(void);
void OLED_init(void);
void OLED_drawline(unsigned char x1H, unsigned char x1L, unsigned char y1H, unsigned char y1L, unsigned char x2H, unsigned char x2L, unsigned char y2H, unsigned char y2L, unsigned char col1, unsigned char col2);
void send_uart(unsigned char byte);
void ping_average(void);

#endif  
