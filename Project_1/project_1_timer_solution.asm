;
; timer_counter_solution_2.asm
;
; Created: 10/14/2021 4:12:28 PM
; Author : konst
;

;; project 1 solution with timer counter
;; first thing i calculate the initialisation i need to do in the 
;; timer. I have 10MHz clock with 1024 prescaler. the overflow interrupt will have 
;; a frequency of fclk/256 (8 bit counter) = 39.0625 kHz. So the 1/flc' = 25.6ms.
;; The result is that i make 56 cycles in 5.6ms. I have to initialise the timer in 56
;; so that i can overflow the right time. First thing to do in the code is to create the 
;; directives that go in the reset and the overflow interrupt handler. In the reset 
;; tag i initialise the timer value at start, load the prescaler in the proper value, and 
;; enable the timer overflow flag and the timer mask interrupt(to know that i want the specific interrupt).
;; in the end i enable the interrupts so i can take them when they happen. When the interrupt happens i 
;; send a signal to portB to light the LED, i stop the timer and then i jump to an infinite loop to burn the 
;; cycles as long as the programm ended.



.org 0x00 rjmp reset
.org 0x12 rjmp timer_overflow

reset:
	;; enable the portB Data direction register as an output
	sbi DDRB,5
	cbi PORTB, 5 ;; initial value to 0
	

	; load the starting value of the timer 
	ldi r20, 0x3C
	out TCNT0, r20 ;; load the starting value to the timer0 in order to make the proper amount of cycles to have the proper delay

	;; loading the prescaller
	ldi r22, ((1<<CS02)|(1<<CS00)) 
	out TCCR0, r22 ; make the prescaller 1024

	;enable tov0 to send interrupt in case of overflow 
	ldi r20, (1<<TOV0)
	out TIFR, r20 ; last bit of tifr is for tov0
	
	; enable mask interrupt flag so i can take care only about the specific counter's interrupt
	ldi r20, (1<<TOIE0)
	;enable toie
	out TIMSK, r20 ;;last bit of timsk is for toie
	
	;; enable global interrupts so i can get them
	sei



	; the infinite loop
	end_loop:
	rjmp end_loop





timer_overflow:
	;; light the LED through portB
	sbi PORTB, 5
	; stop the timer
	ldi r20, 0
	out TCCR0, r20 
	rjmp end_loop ;; jump to infinite loop to stop the timer