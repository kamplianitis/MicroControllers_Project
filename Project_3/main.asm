;
; AssemblerApplication1.asm
;
; Created: 10/25/2021 9:51:46 PM
; Author : Amplianitis	Konstantinos
; 2017030014
;


.dseg 
	port_C_segment: .byte 8
	seven_seg_status: .byte 11
	
.cseg
	; definitions
	.def rotate_reg = r20
	.def adress_adder = r21
	;directives
	.org $000 jmp start
	.org $012 jmp timer0Ovf
	.org $016 jmp getc 




start: 
	;initialise stack
	ldi r16,LOW(RAMEND)
	out SPL, r16
	ldi r16,HIGH(RAMEND)
	out SPH, r16


	;store the statuses in the sram
	ldi ZL, LOW(seven_seg_status) 
	ldi ZH, HIGH(seven_seg_status)

	ldi r16, 0b00000011
	st  Z+, r16 ; store 0
	
	ldi r16, 0b10011111
	st  Z+, r16 ; store 1
	
	ldi r16, 0b00100101
	st  Z+, r16 ; store 2
	
	ldi r16, 0b00001101
	st  Z+, r16 ; store 3

	ldi r16, 0b10011001
	st  Z+, r16 ; store 4

	ldi r16, 0b00100101
	st  Z+, r16 ; store 5

	ldi r16, 0b01000001
	st  Z+, r16 ; store 6
	
	ldi r16, 0b00011101
	st  Z+, r16 ; store 7

	ldi r16, 0b00000001
	st  Z+, r16 ; store 8

	ldi r16, 0b00001001
	st  Z+, r16 ; store 9
	
	;store the new status
	ldi r16, 0b11111111 ;0xA or everything is closed
	st Z+,r16 


	;initialise the rotating register. This will give us the port C. Not needed in this lab
	ldi rotate_reg, 0b00000001

	;first thing i initialise the portA, portC as outputs
	ldi r18,0xFF 
	; open the data direction registers as outputs
	out DDRA, r18 
	out DDRC, r18 

	; i use only y,z for the subroutines to make sure this will give my register the right value
	ldi XL, LOW(port_C_segment)
	ldi XH, HIGH(port_C_segment)


	;initialisations
	call TimerInit
	call UARTInit
	;enable global interrupts
	sei
	; infinite loop
	Loop: 
		rjmp Loop ; infinite loop




; timer setup routine. There is the initialization of the timer
; to a specific value and with a specific prescaler. Using Timer0
; and prescaler of 256. Starting value of timer 96.
TimerInit:
			;Timer0 with prescaler of 256.
			ldi r19, 0x60 ;timer seted to 96
			out TCNT0,r19

			;loading the prescaler
			ldi r19, (1<<CS02) ; prescaler to 256
			out TCCR0, r19

			;enable tov
			ldi r19, (1<<TOV0)
			out TIFR, r19

			;interrupt mask
			ldi r19, (1<<TOIE0)
			;enable toie
			out TIMSK, r19 ;;last bit of timsk is for toie
			ret 



;initialisation of UART in the microcontroller. 
; the ubrr is calculated by (fosc/16*baud_rate) -1.
;then i enable the reciever and transmitter and the interrupts flag
UARTInit: 
		;load baud prescaler
		ldi r16, 0x00
		out UBRRH, r16
		ldi r16, 0x40
		out UBRRL, r16
		;enable transmiter reciever
		ldi r16, ((1 << RXEN) | (1 << TXEN) | (1<<RXCIE))
		out	UCSRB, r16
		; frame format. not sure if i have to set ucsz0
		ldi r16, ((1<< URSEL)| (0<<USBS)|(3<<UCSZ0))
		out UCSRC, r16
		ret




; timer interrupt handler. Rotates the rot_reg and displays the next situation that 
; will be displayed
timer0Ovf:
			; i load the value that will take the specific portC
			ld adress_adder, X+
			
			; change the values
			; process to load the value of the rotate reg
			ldi ZL, LOW(seven_seg_status) 
			ldi ZH, HIGH(seven_seg_status)
	
			;go to the proper address in the memory
			add ZL, adress_adder

			ld r2, Z

			;outputs
			out PORTA, r2
			out PORTC, rotate_reg
			
			; shifting and preparations for the next 
			; interrupt for the rotations and the 
			; reset if needed
			rol rotate_reg
			cpi rotate_reg, 0x00
			brne timerSet
			rcall Reset_Address


			timerSet:
			;timer reset
			ldi r19, 0x60 ;timer seted to 96
			out TCNT0, r19
			reti



; reseting subroutine. Resets the rotating reg and the address adder to the initial values.
Reset_Address: 
				ldi XL, LOW(port_C_segment) ; no need to load the high part as the YH reg isn't used anywhere
				ld adress_adder, X
				ldi rotate_reg, 0b00000001
				ret





; interrupt reciever. We recieve one char at the time. We have a switch like function 
; to check what is the recieved character on ascii. the check is getting done with branches.
; in the end it returns.
getc:
		in r16, UDR
		in r16, UDR  // need it to be there two times to make sure udr does not hangup
		mov r16, r15 // take the input from rs-232

		; basically a switch
		;a and t
		cpi r16, 0x41
		breq end_processing

		cpi r16, 0x54
		breq end_processing

		;c and n 
		cpi r16, 0x43 
		breq empty_seven_seg

		cpi r16, 0x4E
		breq empty_seven_seg

		;lf 
		cpi r16, 0x0A
		breq lf_int

		; for the numbers i have to cover an array of ascii chars.
		cpi r16,0x30
		brlo end_processing
		; i put this in after cheking the below values on purpose. I don't mention cr cause either way i skip the function
		cpi r16, 0x3A
		brlo data_processing
		
		;end of the interrupt routine
		end_processing:
						reti




; either i need to clear With the option c or wait for a next number with the option n i have to reset the seven segment displays.
; practically at every segment of C we print the 0X0A which means that they are switched off
empty_seven_seg:
				
				ldi r17, 0x0A
				ldi r18, 0x00 ; will help me count in the loop
				ldi r19, 0x08
				; now i create the store loop that will store the off button
				
				; i load the values of the portc Segments
				ldi YL, LOW(port_C_segment)
				ldi YH, HIGH(port_C_segment)
				
				; store at every display reserved for port C
				; to 0X0A in order to make them look switched off
				store_loop_C:
							st Y+, r17
							inc r18
							cp r18, r19
							brne store_loop_C
				rjmp end_processing ;jump to the ret to wait for the next interrupt to come




;data processing is responsible for the numbers.
;the basic thing i need to do is to practically push every 
;stored bit in the next memory slot. 
data_processing:
				
				;I have the r16 which has the data input. I have to always put the input at the last spot(least significant bit)
				
				;first thing i need to do is load both the sram locations that i keep
				
				;portC segments that we will be storing the numbers
				ldi YH, HIGH(port_C_segment)
				ldi YL, LOW(port_C_segment) 

				; next thing to do is to find a way to be able to process the r16 as a number and not as an ascii code.
				;The digits 0 to 9 are ASCII 30 hex to 39 hex, so i have just to subtract the 0x30 from r16 to have the number
				; that i will use as offset in the memory with the Z register.
				subi r16, 0x30 ; subi uses one less cycle so we use that. Now we can use the  r16 as offset.

				ldi r18,0x00
				ldi r19,0x07
				store_loop:
							ld r17, Y
							st Y+, r16
							mov r16, r17
							inc r18
							cp r18,	r19
							brne store_loop			
				rjmp end_processing
				

;practically after the lf has been gotten all i have to do is to transmit to the pc that  
; everything went ok and i can close the interrupt to wait for the next input.
lf_int:
	
	; load the ok<CR><LF> one by one 
	ldi r16, 0x47
	; call the putc to transmit
	rcall putc
	; same procedure for the rest of the transmitions. Too little code to make a loop
	ldi r16,0x4B
	rcall putc
	
	ldi r16,0x0D
	rcall putc
	
	ldi r16,0x0A
	rcall putc
	rjmp end_processing




;transmits a character each time. I use TCNT2 cause 
; i already use the tcnt0 and i UDR won't work with out instruction.

putc:
		in r17,UCSRA
		sbrs r17,UDRE ; practically we check the flag for empty udr. if it's true skip next instruction
		rjmp putc

		out TCNT2,r16 ; transmition of the r16 that we have put the data
		ret