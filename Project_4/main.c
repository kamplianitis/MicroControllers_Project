/*
 * GccApplication1.c
 *
 * Created: 11/3/2021 2:36:47 PM
 * Author : konstantinos Amplianitis
 */ 

#include <avr/io.h>
#include <avr/interrupt.h> // call the interrupt direction library
#include <stdlib.h>

// the fucntions that are written in assembly code
extern void usart_recieve(void);
extern void timer0Ovf(void);

// practically tell the compiler that this is for the 
ISR(TIMER0_OVF_vect,ISR_NAKED)
{
	timer0Ovf();
	reti();
}

ISR(USART_RXC_vect,ISR_NAKED)
{
	usart_recieve();
	reti();
}


// create the two arrays as i did in assembly 
uint8_t seven_seg_status[11] = {0b00000011,0b10011111,0b00100101,0b00001101,0b10011001,0b00100101,0b01000001,0b00011101,0b00000001,0b00001001,0b11111111};
uint8_t	port_C_segment[8];
//store the rotate register as global to be able to get accessed by the extern values
uint8_t rotate_reg = 0b00000001;
// will be usefull for keeping in wich segment i am each time
uint8_t offset_reg =0b00000000;




// usart initialization definitions
#define FOSC 1843200// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


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
		
    }
}

