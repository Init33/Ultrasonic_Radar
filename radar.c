#include <c8051f120.h>     // SFR declarations
#include "radar.h"
#include <stdio.h>

//delay variables
unsigned char LCD_delayH = 0xF5;
unsigned char LCD_delayL = 0x11;
unsigned char ping_delayH = 0xFF;
unsigned char ping_delayL = 0xFB;
unsigned char maskH = 0xFF;
unsigned char maskL = 0xF3;
//used to tell the servo when to sweep the other direction
unsigned int sweep_direc =1;
//used for the 1 second delay with the alarm
unsigned int beep_count = 0;
//variables used in the ping function
int RXcounts;
int RXfinal;
unsigned int RXavg = 0;
unsigned int counts_reached = 0;
unsigned int counts;
unsigned int count;
//stores the uart RX input byte
unsigned char UART_RX;
//variables used for drawing to the OLED
unsigned char x1H, x1L, x2H, x2L, y1H, y1L, y2H, y2L;
unsigned int y;
unsigned char draw_ready;
unsigned char servo_ready;
//used to keep track of the servo duty cycle
unsigned int duty_cycle = 64225;
unsigned int duty_ref;
//used for buffering of the LCD output chars
unsigned char LCD_HB;
unsigned char LCD_LB;
//used for angle and distance calculations
int angle;
unsigned int distance;
unsigned char distance2;
//used to convert the distance and angle to strings
unsigned char angle_string[5] = {0,0,0,0,0};
unsigned char dist_string[5] = {0,0,0,0,0};
//used to keep track of when the OLED is enabled
unsigned char OLED_enable = 0;
unsigned int ping_inc;
int i,j,k,z,p;
//used for uart communication
int send_instructions = 0;
//keeps track of when the servo is on
unsigned int servo_stopped = 0;

code char message[166] =
{
	"\nHello welcome to radar \nOptions:\ns: stop sweeping\ng: start sweeping\no: stop alarm\np: start alarm\nr: start radar display\n"
};
code char message2[21] =
{
	"Servo is now OFFLINE\n"
};
code char message3[20] =
{
	"Servo is now ONLINE\n"
};
code char message4[21] =
{
	"Alarm is now OFFLINE\n"
};
code char message5[20] =
{
	"Alarm is now ONLINE\n"
};
code char message6[93] =
{
	"Radar screen ACTIVATED\nDisregard terminal until radar is stopped\nPress push button 1 to stop\n"
};

//function prototypes
void UART_Init();
void PCA_Init();
void Voltage_Reference_Init();
void DAC_Init();
void set_duty(unsigned short int duty);
void delay(unsigned char H,unsigned char L);
void ping();
void LCD_display(char string[]);
void LCD_clear(void);
void radar_calc(void);
void LCD_shift(void);
void OLED_init();
void OLED_drawline(unsigned char x1H, unsigned char x1L, unsigned char y1H, unsigned char y1L, unsigned char x2H, unsigned char x2L, unsigned char y2H, unsigned char y2L, unsigned char col1, unsigned char col2);
void send_uart(unsigned char byte);
void ping_average();

void main(void)
{
	//initialisation
	General_Init();
	Interrupts_Init();
	Timer_Init();
	OLED_init();
	UART_Init();
	LCD_Init();
	PCA_Init();
	Voltage_Reference_Init();
	DAC_Init();
	
	while(1)
	{
		//takes the average of multiple pings
        ping_average();
        
		// displays the angle and distance every 20 cycles of the main while loop
		if(z == 20)
		{
			radar_calc();	//calculates the angle and distance and converts the numbers to strings
			LCD_clear();	//clears the LCD
			LCD_display(angle_string); //displays the angle
			LCD_display(" degrees");	//suffix of angle
			LCD_shift();				//changes LCD lines
			LCD_display(dist_string);	//displays the distance
			LCD_display(" mm   ");		//distance suffix
			z = 0;		//reinit z
		}
		z++;	//inc z each main loop
		
		// if s is sent over serial, stop the servo
		if (UART_RX == 's')
		{
			SFRPAGE = UART0_PAGE;
			//print the servo stopped message
			for(k=0;k<21;k++)
			{
				send_instructions = 1;
				SBUF0 = message2[k];
				while(send_instructions);
			}
			SFRPAGE = PCA0_PAGE;
			CR = 0;	//turns off the PCA (servo)
			UART_RX = 0;	//reset the RX variable
			servo_stopped = 1; //servo stopped variable is high
		}
		// if g is pressed, start the servo
		if (UART_RX == 'g')
		{
			SFRPAGE = UART0_PAGE;
			for(k=0;k<20;k++)
			{
				send_instructions = 1;
				SBUF0 = message3[k];
				while(send_instructions);
			}
			SFRPAGE = PCA0_PAGE;
			CR = 1;	//PCA on
			UART_RX = 0;	//reset uart RX
			servo_stopped = 0;
		}
		//if o is received, turn off the alarm
		if (UART_RX == 'o')
		{
			SFRPAGE = UART0_PAGE;
			for(k=0;k<21;k++)
			{
				send_instructions = 1;
				SBUF0 = message4[k];
				while(send_instructions);
			}
			SFRPAGE = DAC0_PAGE;
			DAC0CN = 0x18;	//DAC speaker is off
			UART_RX = 0;
		}
		// if p is pressed, turn the alarm on
		if (UART_RX == 'p')
		{
			SFRPAGE = UART0_PAGE;
			for(k=0;k<20;k++)
			{
				send_instructions = 1;
				SBUF0 = message5[k];
				while(send_instructions);
			}
			SFRPAGE = DAC0_PAGE;
			DAC0CN = 0x98;		//DAC on
			UART_RX = 0;
		}
        // if r is pressed, radar scan is activated
		if (UART_RX == 'r')
		{
			SFRPAGE = UART0_PAGE;
			for(k=0;k<93;k++)
			{
				send_instructions = 1;
				SBUF0 = message6[k];
				while(send_instructions);
			}
			OLED_enable = 1;
			UART_RX = 0;
		}
        
        /*--------------------------------------------------
                Turns on the OLED screen for scanning
         ---------------------------------------------------*/
		if(OLED_enable)
		{
			SFRPAGE = UART0_PAGE;
			SCON0 = SCON0 & 0xEF; //disables receive enable
			//reset the module
			LD0 = 0;
			LD0 = 1;
			//waits around 5 seconds for startup
			for(p=0;p<70;p++)
			{
				delay(0x00,0x00);
			}
			SCON0 = SCON0 | 0x10; //enables receive
            
			//clear screen
			send_uart(0xFF);
			send_uart(0xCD);
            
			delay(0xFF,0xC0);
            
            // scans while OLED is enabled
			while(OLED_enable)
			{
				RXavg = 0;       
				ping();
				RXavg = RXavg + RXcounts;
				ping();
				RXavg = RXavg + RXcounts;
				RXfinal = RXavg/2;
                
				y = 2*((64225-duty_ref)/30);  //calculates the angle for y coordinate position on OLED
                // calculates the y high and low bytes
				y1L = y & 0x00FF;
				y = y>>8;
				y1H = y & 0x00FF;
				y2L = y1L;
				y2H = y1H;
                
                distance2 = (RXfinal*10)/72;  //calculates distance, but as an unsigned char
				x1L = distance2;
                // if for some reason, the distance overflows the screen boundary, set x1L as max screen width
				if (distance2>0xF0)
				{
					x1L = 0xF0;
				}
                
				while(servo_ready==0);	//waits for the servo to shift to the next scan position
				servo_ready = 0;		//reset initialiser
                
				SFRPAGE = PCA0_PAGE;
				CR = 0;     //turns off the servo while drawing lines
				OLED_drawline(0x00,0x00,y1H,y1L,0x00,x1L,y2H,y2L,0x00,0x00); //draws line from bottom to top
				delay(0xCC,0x00);
				OLED_drawline(0x00,x1L,y1H,y1L,0x00,0xF0,y2H,y2L,0xFF,0xFF); //draws line over the top of the first line proportional to the distance
				draw_ready = 1; //tells the servo interrupt that the line has successfully been drawn
				SFRPAGE = PCA0_PAGE;
				CR = 1; //servo is turned back on
				
                //if the first pushbutton is pressed, turn off the OLED
				if (~PB1)
				{
					LD0 = 0;
					OLED_enable = 0;
                    //resend the menu options
					for(k=0;k<166;k++)
					{
						send_instructions = 1;	// initialise variable
						SBUF0 = message[k];
						while(send_instructions); //wait until the variable is cleared
					}
				}
			}
		}
        
		
		/*--------------------------------------------------
                If the object is within 100 mm
         ---------------------------------------------------*/
		if (RXfinal < 716)
		{
			SFRPAGE = PCA0_PAGE;
			CR = 0;	//stop sweeping servo
			SFRPAGE = TMR2_PAGE;
			TR2 = 1;
            
			while(RXfinal < 716)
			{
                ping_average();
        
                //displays the angle and position every 20 loops
				if(z == 20)
				{
					radar_calc();	//calculates the angle and distance and converts the numbers to strings
					LCD_clear();	//clears the LCD
					LCD_display(angle_string); //displays the angle
					LCD_display(" degrees");	//suffix of angle
					LCD_shift();				//changes LCD lines
					LCD_display(dist_string);	//displays the distance
					LCD_display(" mm   ");		//distance suffix
					z = 0;
				}
				z++;
			}
            // if the object is further than 100 mm and the servo is operation, start the servo again and turn alarm off
			if((RXfinal > 716) && (servo_stopped==0))
			{
				SFRPAGE = PCA0_PAGE;
				CR = 1;
				SFRPAGE = TMR2_PAGE;
				TR2 = 0;
			}
            // if the object is further than 100 mm and the servo is OFF, turn the alarm off
			if((RXfinal > 716) && (servo_stopped==1))
			{
				SFRPAGE = TMR2_PAGE;
				TR2 = 0;
			}
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------
        Initialisation Functions
 --------------------------------------------------------------------------------------------------------------------*/
void General_Init()
{
	WDTCN = 0xde;
	WDTCN = 0xad;
  	SFRPAGE = CONFIG_PAGE;
	P0MDOUT = 0x15;		// NOTE: Pushpull required for Servo control OTHERWISE TOO WEAK TO DRIVE PROPERLY SINCE ONLY 3.3V!!!!
	P1MDOUT = 0x00;		// Ensure not pushpull outputs....this could damage microcontroller...
	P2MDOUT = 0xff;		// Need to make pushpull outputs to drive LEDs properly
    XBR0      = 0x04;
    XBR2      = 0x40;
	Servo_Ctrl = 0;	// Initialise servo control pin to 0
  	OSCICN    = 0x83;		// Need a faster clock....24.5MHz selected
}
void Interrupts_Init()
{
    IE        = 0xB0;   //all int, timer 2, uart0
    EIE1      = 0x08;
    IP        = 0x10; //uart0 high priority
}
void Timer_Init()
{
	SFRPAGE = TIMER01_PAGE;
	TMOD      = 0x11;		//timer 0 and 1 in 16 bit mode
    CKCON     = 0x02;       //SYSCLK/48
	
	SFRPAGE   = TMR2_PAGE;
    RCAP2L    = 0xAA;
    RCAP2H    = 0xF0;
    
	SFRPAGE   = TMR3_PAGE;
    TMR3CF    = 0x08;   //SYSCLK
	RCAP3L 	  = 0xF6;
	RCAP3H 	  = 0xFF;
    
	// timer 4 used for UART0
	SFRPAGE   = TMR4_PAGE;
    TMR4CF    = 0x08;   //SYSCLK
    RCAP4L    = 0x60;
    RCAP4H    = 0xFF;
	TR4 	  = 1;      //timer 4 on
}

void OLED_init()
{
	LD0 = 0;    //OLED reset pin on
}

void UART_Init()
{
    SFRPAGE   = UART0_PAGE;
    SCON0     = 0x50;		//mode 1, receive enabled
    SSTA0     = 0x0F;		//timer 4 overflow generates TX and RX clocks
	//sending instructions to putty
	for(k=0;k<166;k++)
	{
		send_instructions = 1;	// initialise variable
		SBUF0 = message[k];
		while(send_instructions); //wait until the variable is cleared
	}
}
void PCA_Init()
{
    SFRPAGE   = PCA0_PAGE;
    PCA0CN    = 0x40;   //timer enabled
    PCA0MD    = 0x01;   //SYSCLK/4
    PCA0CPM0  = 0x49;   //enables the capture compare interrupts
    PCA0L     = 0x7F;
    PCA0H     = 0x60;
    PCA0CPL0  = 0xE1;
    PCA0CPH0  = 0xFA;
}
void Voltage_Reference_Init()
{
	SFRPAGE   = ADC0_PAGE;
	REF0CN    = 0x03; // Turn on internal bandgap reference and output buffer to get 2.4V reference (pg 107)
}
void DAC_Init()
{
	SFRPAGE = DAC0_PAGE;
	DAC0CN 	= 0x80;     //DAC on
	DAC0L = 0x00;
	DAC0H = 0x00;
}
/*---------------------------------------------------------------------
            functions
 --------------------------------------------------------------------*/
void ping_average()
{
	RXavg = 0;          //reinit
    ping_inc = 0;   //reinit
    //takes 8 pings and averages them
	while(ping_inc<8)
    {
        ping();
 		RXavg = RXavg + RXcounts;
        ping_inc++;
    }
    RXfinal = RXavg/8;
}


//basic delay tool using timer 0
void delay(unsigned char H,unsigned char L)
{
	unsigned char SFRPAGE_SAVE2 = SFRPAGE;
	SFRPAGE = TIMER01_PAGE;
	TH0 = H;
	TL0 = L;
	TR0 = 1;
	while(~TF0);    //waits until overflow has occured
	TF0 = 0;
	TR0 = 0;
	SFRPAGE = SFRPAGE_SAVE2;
}
//function that sends a TX pulse, waits for an RX pulse and records the time elapsed
void ping()
{
	//ping signal
	for(i=0;i<4;i++)
	{
        USonicTX = 1;
        delay(ping_delayH,ping_delayL);
        USonicTX = 0;
        delay(ping_delayH,ping_delayL);
	}
	//mask time
	delay(maskH,maskL);
    
	count = 0;          //reinit
	counts_reached = 0; //reinit
    
	SFRPAGE = TMR3_PAGE;
	TR3 = 1;    //timer 3 on
    
	while (count<1429) 	//counts 1429 times which is about enough time for a 20cm return trip ping
	{
		if(TF3)		//if the timer 3 overflow is set
		{
			TF3 = 0;	//reset the overflow flag
			count++;	//increment the count
            
			// detects when the TX ping was received
			if (USonicRX && ~counts_reached)
			{
				RXcounts = count;		//the number of counts to recieve TX signal is TXcounts
				counts_reached=1;		//records that the TX signal has been recieved
			}
		}
	}
	
	TR3 = 0;    //timer 3 off
	
	// if the TX signal was not recieved, return the max distance of 20cm (595 counts)
	if(counts_reached == 0)
	{
		RXcounts = count;
	}
}

// Timer 2 interrupt that controls proximity beep_count
void Timer2_ISR (void) interrupt 5
{
  	unsigned char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
	SFRPAGE = DAC0_PAGE;
	if(beep_count<525s)  //increment that counts 1 second for on off alarm sequence
	{
        //creates a square wave on the DAC every timer 2 interrupt
		if(DAC0L == 0x00)
		{
			DAC0L = 0xFF;
			DAC0H = 0xFF;
		}
		else if(DAC0L == 0xFF)
		{
			DAC0L = 0x00;
			DAC0H = 0x00;
		}
	}
	else    //if the increment is between 500 and 1000 then no sound for 1 second
	{
		DAC0L = 0x00;
		DAC0H = 0x00;
	}
    //if 2 seconds have elapsed, reinit the counter
	if (beep_count>1050)
	{
		beep_count = 0;
	}
	beep_count++; //increment the counter
    
	SFRPAGE = TMR2_PAGE;
	TF2 = 0;
  	SFRPAGE = SFRPAGE_SAVE; 							      // Restore SFR page
}

// interrupt routine for UART communication
void UART0 (void) interrupt 4
{
	unsigned char SFRPAGE_SAVE1 = SFRPAGE;        // Save Current SFR page
	UART_RX = SBUF0;	//reads the UART sent value
	send_instructions = 0;
	TI0 = 0;	//resets transmit interrupt
	RI0 = 0;	// resets receive interrupt
	SFRPAGE = SFRPAGE_SAVE1;
}

// program counter array that controls servo
void PCA (void) interrupt 9
{
	unsigned char SFRPAGE_SAVE2 = SFRPAGE;
	SFRPAGE = PCA0_PAGE;
    //if the first counter has overflowed, turn the servo low (~high)
	if(CCF0==1)
	{
		Servo_Ctrl=1;
	}
    // if the second counter has overflowed, calculate the new capture values from the changing duty cycle
	if(CF==1)
	{
		Servo_Ctrl=0;
        //if the servo is sweeping positively
		if(sweep_direc==1)
		{
            //calculates the current duty cycle from the capture values
			duty_cycle = PCA0CPH0;
			duty_cycle = duty_cycle<<8;
			duty_cycle = duty_cycle | PCA0CPL0;
            //if the OLED is active, only allow the servo to move if the draw function has completed
			if((draw_ready==1)&&(OLED_enable==1))
			{
				duty_cycle = duty_cycle + 30;
			}
			if(OLED_enable==0)
			{
				duty_cycle = duty_cycle + 30;
			}
            //duty_ref tracks the last calculated duty cycle for display purposes
			duty_ref = duty_cycle;
            //if the servo has gone full 90 degrees change the direction
			if(duty_cycle>64225)
			{
				sweep_direc = 0;
			}
            //assign new capture values
			PCA0CPL0 = duty_cycle & 0x00FF;
			duty_cycle = duty_cycle>>8;
			PCA0CPH0 = duty_cycle & 0x00FF;
		}
        //in the other direction, duty cycle is decremented
		if(sweep_direc==0)
		{
			duty_cycle = PCA0CPH0;
			duty_cycle = duty_cycle<<8;
			duty_cycle = duty_cycle | PCA0CPL0;
			if((draw_ready==1)&&(OLED_enable==1))
			{
				duty_cycle = duty_cycle - 30;
			}
			if(OLED_enable==0)
			{
				duty_cycle = duty_cycle - 30;
			}
            
			duty_ref = duty_cycle;
            
			if(duty_cycle<60565)
			{
				sweep_direc = 1;
			}
			PCA0CPL0 = duty_cycle & 0x00FF;
			duty_cycle = duty_cycle>>8;
			PCA0CPH0 = duty_cycle & 0x00FF;
		}
		servo_ready = 1;
		CCF0 = 0;
		CF = 0;
	}
	SFRPAGE = SFRPAGE_SAVE2;
}
//calculates the angle and distance and converts them to strings or display use
void radar_calc(void)
{
	angle = (((duty_ref-60555)*10)/204)-90;
	distance = (RXfinal*10)/72;
	sprintf(angle_string,"%d",angle);
	sprintf(dist_string,"%d",distance);
}
//basic function that sends byte to the uart and waits until it is sent
void send_uart(unsigned char byte)
{
	send_instructions = 1;
	SBUF0 = byte;
	while(send_instructions);
}
// LCD initialisation
void LCD_Init()
{
	P3 = 0x03;
	E = 1;			//enable high
	delay(LCD_delayH,LCD_delayL);
	E = 0;			//enable low
	E = 1;			//enable high
	delay(LCD_delayH,LCD_delayL);
	E = 0;			//enable low
	E = 1;			//enable high
	delay(LCD_delayH,LCD_delayL);
	E = 0;			//enable low
	E = 1;			//enable high
	delay(LCD_delayH,LCD_delayL);
	E = 0;			//enable low
    
	// 4-bit mode
	P3 = 0x02;
	E = 1;			//enable high
	delay(LCD_delayH,LCD_delayL);
	E = 0;			//enable low
	// function set
	P3 = 0x02;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x08;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	//Display on
	P3 = 0x00;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x0E;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	//Clear Display
	P3 = 0x00;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x01;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	//Entry mode set
	P3 = 0x00;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x06;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	delay(LCD_delayH,LCD_delayL);
	E = 0;
    
}
//shifts the cursor on the LCD to the next line
void LCD_shift(void)
{
    //Shift display
	P3 = 0x0C;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x00;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
}
// clears the LCD screen
void LCD_clear(void)
{
    //Clear Display
	P3 = 0x00;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
	P3 = 0x01;
	E = 1;
	delay(LCD_delayH,LCD_delayL);
	E = 0;
}
// displays a string of characters sent to the LCD
void LCD_display(char string[])
{
	char LCD_char;
	j = 0;
	while(string[j]!='\0')  //waits until the null terminator is found
	{
        //calculates the LCD high byte
		LCD_char = string[j] & 0xF0;
		LCD_char = LCD_char>>4;
		LCD_char = LCD_char | 0x10;
		LCD_HB = LCD_char;
		//calculates the LCD low byte
		LCD_char = string[j] & 0x0F;
		LCD_char = LCD_char | 0x10;
		LCD_LB = LCD_char;
        //sends the high and low bytes to the LCD
        P3 = LCD_HB;
        E = 1;
        delay(LCD_delayH,LCD_delayL);
        E = 0;
        P3 = LCD_LB;
        E = 1;
        delay(LCD_delayH,LCD_delayL);
        E = 0;
        j++;
	}
	return;
}
//draws a line on the OLED display using x1,y1,x2,y2,colour input bytes
void OLED_drawline(unsigned char x1H, unsigned char x1L, unsigned char y1H, unsigned char y1L, unsigned char x2H, unsigned char x2L, unsigned char y2H, unsigned char y2L, unsigned char col1, unsigned char col2)
{
    SFRPAGE = PCA0_PAGE;
    CR = 0;	//disable servo
    
    SFRPAGE = UART0_PAGE;
    // draw line command
    send_uart(0xFF);    //sends the byte via uart0
    send_uart(0xC8);
    // x1	
    send_uart(x1H);
    send_uart(x1L);
    // y1
    send_uart(y1H);
    send_uart(y1L);
    // x2
    send_uart(x2H);
    send_uart(x2L);
    // y2
    send_uart(y2H);
    send_uart(y2L);
    // colour
    send_uart(col1);
    send_uart(col2);
    
    SFRPAGE = PCA0_PAGE;
    CR = 1;	//enable servo
}
