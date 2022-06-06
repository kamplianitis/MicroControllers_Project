/*
 * GccApplication1.c
 *
 * Created: 11/3/2021 2:36:47 PM
 * Author : konstantinos Amplianitis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h> // call the interrupt direction library
#include <stdlib.h>

// create the two arrays as i did in assembly
uint8_t seven_seg_status[11] = {0b00000011,0b10011111,0b00100101,0b00001101,0b10011001,0b00100101,0b01000001,0b00011101,0b00000001,0b00001001,0b11111111};
uint8_t	port_C_segment[8];
//store the rotate register as global to be able to get accessed by the extern values
uint8_t rotate_reg = 0b00000001;
// will be useful for keeping in which segment i am each time
uint8_t offset_reg =0b00000000;

// added. Table that will keep the ok cr lf ascii codes to get access to ther with a for loop in the lf_int of assembly
uint8_t okcrlf_table[4] = {0x47,0x4B,0x0D,0x0A};



// usart initialization definitions
#define FOSC 1843200// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


// practically tell the compiler that this is for the 
// interrupt
ISR(TIMER0_OVF_vect,ISR_NAKED)
{
	uint8_t x = offset_reg; // load the offset register
	offset_reg++; 
	
	x = port_C_segment[x];
	x = seven_seg_status[x];
	
	PORTA = x;
	PORTC = rotate_reg;
	
	rotate_reg  = rotate_reg << 1;
	if(rotate_reg == 0) // case we need reset
	{
		rotate_reg = 0b00000001;
		offset_reg = 0b00000000;
	}
	 // timer reset
	 TCNT0 = 96; // update the value in decimal	
	reti();
}

ISR(USART_RXC_vect,ISR_NAKED)
{
	uint8_t inputascii = UDR;
	inputascii = UDR; // put the UDR in the inputs to reset its flags so it does not hang up.
	inputascii = PORTB; // use the portB cause i did the exact change in the stimuli file.
	
	// create the switch function thing based on the input. I cannot use the switch function because of the fact that for the numbers I need a range
	if(inputascii == 0x41) // A
		asm("nop");
	else if(inputascii == 0x54) // T
		asm("nop");
	else if (inputascii == 0x43 || inputascii == 0x4E) // N or C
	{
		for(int i=0; i<8; i++)
			port_C_segment[i] = 0x0A; // set the offset to a condition that will keep the seven segment off.
	}
	else if (inputascii == 0x0A) 
	{
		
		for(int i=0; i<4; i++)
		{
			while (!(UCSRA & (1 << UDRE))) // wait for the buffer to become empty
			{
				asm("nop");
			}

			TCNT2 = okcrlf_table[i];	
		}
	}
	else if(inputascii < 0x30) // non-intresting ascii char
		asm("nop");
	else if(inputascii < 0x3A) // number
	{
		inputascii = inputascii - 0x30; // take the number as bcd
		uint8_t temp ; // will be needed in order to push the values
		for(int i=0; i<8; i++)
		{
			temp = port_C_segment[i]; // save the  value that will be needed for  the  push 
			port_C_segment[i] = inputascii; // place the the  pushed value in the  specific location 
			inputascii = temp; // update the input with the next number that should be pushed
		}
	}	
	reti();
}



//functions for initializing our units
void USART_Init( unsigned int ubrr)
{
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
}


// function for initializing the timer0.
void TIMER0_INIT(void)
{
	TCNT0 = 0x60; // starting value
	TCCR0 = (1<<CS02);
	TIFR = (1<<TOV0);
	TIMSK = (1<<TOIE0);
}




int main(void)
{	
	DDRA=0xff;
	DDRC=0xff;	
	
	// initialisZ the usart
	USART_Init(MYUBRR);
	
	// initialize timer
	TIMER0_INIT();
	
	// enable global interrupts
	sei();
	
    while (1) 
    {
		asm("nop");
    }
}