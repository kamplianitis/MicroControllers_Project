#define __SFR_OFFSET 0          
#include <avr/io.h>            
#define _SFR_ASM_COMPAT 1

.section .data
.global timer0Ovf
.global usart_recieve


.section .text

; timer interrupt handler. Rotates the rot_reg and displays the next situation that 
; will be displayed
timer0Ovf:
			; i load the value that will take the specific portC
			;ld adress_adder, X+
			ldi ZL, lo8(offset_reg) 
			ldi ZH, hi8(offset_reg)
			ld r22, Z // load the offset of the ports .. This will show in witch display i was before the interrupt
			
			// save the  value to a reg to save it for next 
			mov r23,r22
			// increase for the next use
			inc r22
			st Z, r22

			// load the porc displays positions
			ldi YL, lo8(port_C_segment) 
			ldi YH, hi8(port_C_segment)
			
			// find wich position  
			add YL,r23

			ld  r21,Y
			
			; change the values
			; process to load the value of the rotate reg
			ldi ZL, lo8(seven_seg_status) 
			ldi ZH, hi8(seven_seg_status)
	
			;go to the proper address in the memory
			add ZL, r21

			ld r2, Z


			ldi XL,lo8(rotate_reg)
			ldi XH,hi8(rotate_reg)
			ld  r20,X
			;outputs
			out PORTA, r2
			out PORTC, r20
			
			; shifting and preparations for the next 
			; interrupt for the rotations and the 
			; reset if needed
			
			rol r20
			//
			cpi r20, 0x00
			brne timerSet
			rcall Reset_Address
			

			timerSet:
			;timer reset
			st X,r20
			ldi r19, 0x60 ;timer seted to 96
			out TCNT0, r19
			ret



; reseting subroutine. Resets the rotating reg and the address adder to the initial values.
Reset_Address: 
				ldi XH, hi8(port_C_segment)
				ldi XL, lo8(port_C_segment) ; no need to load the high part as the YH reg isn't used anywhere
				
				ld r21, X // offset
				// now that i dont need the X register i  load the rotate_reg
				ldi XH, hi8(rotate_reg)
				ldi XL, lo8(rotate_reg) 				
				ld r20,X
				ldi r20, 0b00000001
				// change the  offset of the screen choser
				ldi ZH, hi8(offset_reg)
				ldi ZL, lo8(offset_reg) 	
				// use r23 cause i don't need it's value eitherway it's garbage
				ldi r23, 0
				st	X,r23		
				ret





; interrupt reciever. We recieve one char at the time. We have a switch like function 
; to check what is the recieved character on ascii. the check is getting done with branches.
; in the end it returns.
usart_recieve:
		in r16, UDR
		in r16, UDR
		mov r16, r15

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
						ret




; either i need to clear With the option c or wait for a next number with the option n i have to reset the seven segment displays.
; practically at every segment of C we print the 0X0A which means that they are switched off
empty_seven_seg:
				
				ldi r17, 0x0A
				ldi r18, 0x00 ; will help me count in the loop
				ldi r19, 0x08
				; now i create the store loop that will store the off button
				
				; i load the values of the portc Segments
				ldi YL, lo8(port_C_segment)
				ldi YH, hi8(port_C_segment)
				
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
				ldi YH, hi8(port_C_segment)
				ldi YL, lo8(port_C_segment) 

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