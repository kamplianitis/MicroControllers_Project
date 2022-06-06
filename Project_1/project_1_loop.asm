;
; AssemblerApplication1.asm
;
; Created: 10/5/2021 8:32:33 PM
; Author : Konstantinos Amplianitis
;

;; Microcontroller atmega16A
;; delay for 20ms for interrupt send with a loop


;; SOLUTION EXPLANATION
;; first thing i find how many cycles-commands i have to write for the loop in order
;; to calculate the times the loop will run. sbiw takes 2 cycles and brne takes 2 cycles
;; every time there's no hit and 1 cycle if there is one. 
;; Apply the formula 
;; Tdelay = (NumberofLoops*Cyclesperloop -1) * (1/fclk)
;; the -1 is cause of the last brne catch.

;; After the calculation i load the number of loops in the registers. The fact that the number
;; is over 255 makes me use 2 registers so i have 16bit. Then i stop any external interrupts so
;; that the only interrupt that will come is by my programm. I set the global interrupt port with 
;; sbi and i do the loop. After the loop completes i send a global interrupt via PortB through the 
;; pin that i have enabled earlier.


; load the time of the loops into the registers. I will use this as a word to make the decrement
ldi r30 , 0x50	
ldi r31 , 0xC3
cli ; stop any external interrupt

sbi DDRB, 5 ; set the data direction register of the interrupts in the B section. Practically set PortB as an output
start: ;; beginning of the loop
	sbiw r30,1 ;; decrease the word by 1 each time it's running
	brne start ;; branch 
	sbi PORTB , 5 ;; send the interrupt

end: ; infinite loop that will keep the program
	rjmp end