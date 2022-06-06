;
; Lab_2_solution.asm
;
; Created: 10/18/2021 8:38:11 PM
; Author : Amplianitis Konstantinos
;



; first thing i do is i reserve the memory that i will need in the dseg. After this i will 
; get the address of the one of the reserved byte arrays and i store the statuses in 7 seg.
; then i do the same procedure for the other array and i store the bcd numbers. I declare 
; r20 as a rotate reg which is responsible for keeping which of the 8 seven segment displays is 
; open and which will be next. I initialise the rotate register and the portA show to 1. Then 
; i set the timer with prescaler 256 and proceed to the infinite loop. In the end i set up the 
; timer interrupt handler to rotate left the rotate reg and load the value of the Z+bcd offset 
; to port A. Every 8 shifts i reset to get them right




.dseg
	; reserve the blocks of memory that we will need to store the addresses and the values of the numbers
	port_C_segment: .byte 9 ;eight for the memory allocation and one for the situation that will need the reset
	seven_seg_status: .byte 10
	
.cseg 
	.def rotate_reg = r20
	.def adress_adder = r21
	.org 0x00 rjmp reset
	.org 0x12 rjmp timer_ovfr

reset:

	; now i initialize the values i want into sram
	; bring the address that is going to store the statuses of the numbers
	ldi ZL, LOW(seven_seg_status) 
	ldi ZH, HIGH(seven_seg_status)

	ldi r16, 0b00000011
	st  Z+, r16 ; store 0	0
	
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
	st  Z, r16 ; store 8

	ldi r16, 0b00000001
	st  Z, r16 ; store 9


	; load the adresses for the encode.

	;reinitialise the r16 as long as its last value is of no use
	;registers that will help me with the bcd depiction
	ldi r16, 0x01
	ldi r17, 0x0A

	; load in Y the address of the port_C_segment to store the bcd encoding
	ldi YL, LOW(port_C_segment)
	ldi YH, HIGH(port_C_segment)

	; create a loop to store the numbers to the port_C_segment of the memory that i reserved
	store_loop: ; bcd loop basicaly
		st Y+ , r16
		inc r16
		cp r16,r17 ; comparison that will help get out of the loop
		brne store_loop
	
	
	; initialisations
	; will be used as the loop reseter.
	ldi r17,0x09
	; now that everything is stored i have to setup the timer and the ports to complete the process
	
	;first thing i initialise the portA, portC as outputs
	ldi r18,0xFF 
	; open the data direction registers as outputs
	out DDRA, r18 
	out DDRC, r18 

	;initialise the rotating register. This will give us the port C 
	ldi rotate_reg, 0b00000001
	; load again to take the first value
	ldi YL, LOW(port_C_segment) ; the reason that i load only the low part is because that 
	ld adress_adder, Y+ 
	out PORTC, rotate_reg ;set the initial status of the program

	; process to load the value of the rotate reg
	ldi ZL, LOW(seven_seg_status) 
	ldi ZH, HIGH(seven_seg_status)

	add ZL, adress_adder
	ld r2, Z
	out PORTA, r2


	; now i initialise the timer
	;I use timer0 with prescaler of 256.
	ldi r19, 0x60 ;timer seted to 96
	out TCNT0, r19

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
	sei

final_loop:
	rjmp final_loop



timer_ovfr:
	rol rotate_reg
	ld adress_adder, Y+ 
	cp adress_adder, r17 ; adrress_adder will become 9 same as the r17 because it's stored on memory 

	breq reset_address

	changes:
	; change the values
	; process to load the value of the rotate reg
	ldi ZL, LOW(seven_seg_status) 
	ldi ZH, HIGH(seven_seg_status)
	
	add ZL, adress_adder

	ld r2, Z
	out PORTA, r2
	out PORTC, rotate_reg
	; reset the timer
	ldi r19, 0x60 ;timer seted to 96
	out TCNT0, r19
	sei
	rjmp final_loop




reset_address: ; reset the shift register and the address that will find the status when the value i want is exceeded
	ldi YL, LOW(port_C_segment) ; no need to load the high part as the YH reg isn't used anywhere
	ld adress_adder, Y
	ldi rotate_reg, 0b00000001
	rjmp changes