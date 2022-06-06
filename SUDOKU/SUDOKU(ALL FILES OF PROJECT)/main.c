/////////////////////////////////////////////////////////////////
//////////////// A Backtracking program  in C ///////////////////
////////////////   to solve Sudoku problem    ///////////////////
/////////////////////////////////////////////////////////////////
//#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

// UNASSIGNED is used for empty
// cells in sudoku grid
#define UNASSIGNED 0
// N is used for the size of
// Sudoku grid. The size will be NxN
#define N 9

#define SizeOfReadWord 8
	// UART VARIABLES    
   volatile char  readWord[SizeOfReadWord]; // read word from UART
   volatile uint8_t  letter; // next letter of readWord we can write
   
   
   // SUDOKY START VARIABLES
   volatile uint8_t  play; // flag that starts the process of solving the sudoku after getting all the inputs
   volatile uint8_t  progress; // number of solved cells.. it gets updated every time  a value is getting putted in the grid or a cell is updated by solve sudoku function
   
   //GRID VARIABLES 
   volatile uint8_t  sendXpos; // position x of next number we wanna send
   volatile uint8_t  sendYpos; // position y of next number we wanna send
	
  volatile uint8_t  grid[N][N]={};/* {  { 8, 0, 0, 0, 0, 0, 0, 0, 0 }, 
									{ 0, 0, 3, 6, 0, 0, 0, 0, 0 },
									{ 0, 7, 0, 0, 9, 0, 2, 0, 0 },
									{ 0, 5, 0, 0, 0, 7, 0, 0, 0 },
									{ 0, 0, 0, 0, 4, 5, 7, 0, 0 },
									{ 0, 0, 0, 1, 0, 0, 0, 3, 0 },
									{ 0, 0, 1, 0, 0, 0, 0, 6, 8 },
									{ 0, 0, 8, 5, 0, 0, 0, 1, 0 },
									{ 0, 9, 0, 0, 0, 0, 4, 0, 0 } }; //*/  //if we wanna put a sudoku problem manually
	
	
/* 
	FindUnassignedLocation
	
	RETURNS: uint8_t, returns if there is a position in the grid that has a value assigned as zero.
	
	ARGUMNETS
	Arg1: uint8_t *row, the row of the sudoku that is surely assigned by now
	Arg2: uint8_t *col, the col of the sudoku that is surely assigned by now
	
	FUNCTIONALITY:
	The function takes as an argument the last position in the grid that has a value assigned to it.
	Then, through while loops where is the next position that is unassigned in the grid. The function
	does that by updating both the pointers of the row and column in the grid so when it returns it 
	passes the values and by that the position in the sudoku.
*/
uint8_t FindUnassignedLocation( uint8_t* row, uint8_t* col)
{
	(*row)=0;
	(*col)=0;
    while((*row)<N) // checks the remaining rows one by one
	{
        while((*col)<N) // check the values of each row 
		{
            if (grid[*row][*col] == UNASSIGNED)
			{
                return 1; // the row, col are already updated so there is nothing to be done
			}
			(*col)++;
		}
		(*col)=0; // end of the row
		(*row)++;
	}
    return 0; // case that there is no unassigned values in the grid. 
}

/* 
	isSafe

	RETURNS: uint8_t, a binary value that indicates if the number that has been put is allowed to be in that position in the grid.

	ARGUMNETS
	Arg1: uint8_t *row, the row of the sudoku that is surely assigned by now.
	Arg2: uint8_t *col, the col of the sudoku that is surely assigned by now.
	Arg3: uint8_t num, the number that is getting checked in order to be assigned.
	
	
	FUNCTIONALITY:
	The function takes the num to possibly be assigned in checks 3 things.
	
	First thing to check is whether there is the same number in the same 
	row that is going to get putted. In case that happens the safe flag goes 
	down and the process will return zero.
	
	Second thing to check is whether there is the same number in the same 
	column that the value is going to get assigned.In case that happens the safe 
	flag goes down and the process will return zero.
	
	Finally the function is going to check whether the value exists is in the sub-grid 
	(each sub-grid is a 3x3 box). In case that happens the safe flag goes
	down and the process will return zero.

	In case the safe flag is up after all the checks, which means that the conditions 
	have been met, the functions returns true. 
*/
uint8_t isSafe( uint8_t row,uint8_t col, uint8_t num)
{
	//safe flag that is responsible of getting down if 
	//some of the below conditions are not met
	uint8_t safe=1; 
	
	// the below variables are here in order to make 
	// us be able to roam in the grid without alternating
	// the arguments of the functions
	uint8_t tempRow;
	uint8_t tempCol;
	


	// row check 
	tempCol=0;
    while(tempCol<N && safe!=0){
		if (grid[row][tempCol] == num){
			safe=0; // case there is the same number in the same row
		}
		tempCol++;
	}

	// column check
	tempRow=0;
	while(tempRow<N && safe!=0){
		if (grid[tempRow][col] == num){
			safe=0; // case there is the same number in the same column
		}
		tempRow++;
	}
	
	// sub-grid check
	
	//Take the relative position in the sub-grid.
	tempCol=col - col % 3;
	tempRow=row - row % 3;
	
	uint8_t boxRow=0;
	uint8_t boxCol=0;
	// check if there is the same number in the sub-grid
	while(boxRow<3 && safe!=0)
	{
		boxCol=0;
		while(boxCol<3 && safe!=0)
		{
			if (grid[boxRow + tempRow][boxCol + tempCol] == num)
			{
				safe=0;
			}
			boxCol++;
		}
		boxRow++;
	}
    return safe && grid[row][col] == UNASSIGNED;
}


/* 
	SolveSudoku
	
	RETURNS: uint8_t, a binary value that indicates if the sudoku is solved or not.

	FUNCTIONALITY:
	This is a recursive function that calls herself every time the function finds an
	unassigned location in the grid. The Sudoku solution algorithm is backtracking the
	solution of each cell of it. Practically the function finds an unassigned location
	in the grid and starts testing possible numbers there. If a number can be assigned 
	there the function calls herself to repeat the process. if not the function does not 
	assign the number and tries the next one.
*/
uint8_t SolveSudoku()
{
     uint8_t row;
	 uint8_t col;
    // Check If there is no unassigned
    // location, we are done
    if (!FindUnassignedLocation(&row,&col) || play==0)
        return 1; // success!
   //Consider digits 1 to 9
	progress++;
    for (uint8_t num = 1; num <= 9; num++)
    {

        // Check if there is a possible match
        if (isSafe( row, col, num))
        {

            // Make tentative assignment
            grid[row][col] = num;

            // Return, if success!
            if (SolveSudoku()){
                return 1;
			}
            // Failure, unmake & try again
            grid[row][col] = UNASSIGNED;
        }
    }

    // This triggers backtracking
	progress--;
    return 0;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;     SEND A CHAR TO USART     ;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


/*
	putC
	
	RETURNS: void
	
	ARGUMNETS:
	Arg1: uint8_t writeChar, the char that is going to be exported from the I/O
	
	FUNCTIONALITY:
	The function is used for I/O. It waits until UDRE flag is up. (the flag is located
	in the UCSRA). When the flag is up, it writes the writeChar var into the UDR in order
	to be exported.
*/
void putC(uint8_t writeChar){
	while(!((UCSRA & 0x20)==0x20)){}  //; wait until data register empty
	UDR=writeChar;
	return;
}


//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//;  FUNCTION THAT READ A        ;
//;  CHAR FROM UART AND DO       ;
//;  SOMEACTIONS FOR SOME CHARS  ;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/*
	getC
	
	RETURNS: char, the value that came from the I/O as an input
	
	FUNCTIONALITY:
	The function works as following. 
	It reads a char from the UDR and then resets it. In order to 
	be able to process the input from the I/O, the function stores 
	the contents of the UDR into a buffer as long as the char is 
	not the <LF> ascii value. Then returns the char the has been 
	read.
*/
char getC(){
	char  readChar;   //read char from UART
	readChar=UDR;
	if(readChar!=0x0D){
		readWord[letter]=readChar;
		letter++;
	}
	return readChar;
}



////////////////////////
//  ISR FOR TIMER_CMP //
////////////////////////
ISR(TIMER1_COMPA_vect, ISR_NAKED) {
	asm("  in r0,__SREG__ \n\t\
	push r0	 \n\t\
	clr r0 \n\t\
	push r16 \n\t\
	push r17 \n\t\
	push r18 \n\t\
	push r19 \n\t\
	push r20 \n\t\
	push r21 \n\t\
	push r22 \n\t\
	push r23 \n\t\
	push r24 \n\t\
	push r25 \n\t\
	push r26 \n\t\
	push r27 \n\t\
	push r28 \n\t\
	push r29 \n\t\
	push r30 \n\t\
	push r31");
	if(progress>79){PORTB=255;}    // activate LEDs accordingly with progress 
	else if(progress>69){PORTB=127;}
	else if(progress>59){PORTB=63;}	
	else if(progress>49){PORTB=31;}
	else if(progress>39){PORTB=15;}
	else if(progress>29){PORTB=7;}
	else if(progress>19){PORTB=3;}
	else if(progress>9){PORTB=1;}  
	TCNT1H=0;					 // Restart TIMER
	TCNT1L=0;					 // Restart TIMER
	asm(" pop r31\n\t\
	pop r30 \n\t\
	pop r29 \n\t\
	pop r28 \n\t\
	pop r27 \n\t\
	pop r26 \n\t\
	pop r25 \n\t\
	pop r24 \n\t\
	pop r23 \n\t\
	pop r22 \n\t\
	pop r21 \n\t\
	pop r20 \n\t\
	pop r19 \n\t\
	pop r18 \n\t\
	pop r17 \n\t\
	pop r16 \n\t\
	pop r0	 \n\t\
	out __SREG__,r0 \n\t\
	clr r0");
	reti();
}

////////////////////////
//  ISR FOR USART     //
////////////////////////

ISR(USART_RXC_vect, ISR_NAKED){
	asm("  in r0,__SREG__ \n\t\
		   push r0	 \n\t\
		   clr r0 \n\t\
		   push r16 \n\t\
		   push r17 \n\t\
		   push r18 \n\t\
		   push r19 \n\t\
		   push r20 \n\t\
		   push r21 \n\t\
		   push r22 \n\t\
		   push r23 \n\t\
		   push r24 \n\t\
		   push r25 \n\t\
		   push r26 \n\t\
		   push r27 \n\t\
		   push r28 \n\t\
		   push r29 \n\t\
		   push r30 \n\t\
		   push r31");
	char readChar=0;
	while((UCSRA & 0x80) == 0x80){  // while RXC==1 (of UCSRA) read char
		readChar=getC();
	}
		if(readChar==0x0A){ // if LF received send OK;
			if(readWord[0]=='A'){ // if command = AT send ok
				putC('O');        // send letter O
				putC('K');        // send letter K
				putC(0x0D);         // send letter CR
				putC(0x0A);         // send letter LF
			}
			else if(readWord[0]=='C'){  //set grid=0 if readchar==C
				uint8_t  i=0;
				progress=0;
				sendXpos=0;
				sendYpos=0;
				PORTB=0;
				i=0;
				uint8_t  j=0;
				while(i<N){     // set grid array to 0
					while(j<N){
						grid[i][j]=0;
						j++;
					}
					j=0;
					i++;
				}
				putC('O');        // send letter O
				putC('K');        // send letter K
				putC(0x0D);         // send letter CR
				putC(0x0A);         // send letter LF
			}
			else if(readWord[0]=='N'){  // command is N read number of grid
				grid[readWord[1]-48-1][readWord[2]-48-1]=readWord[3]-48; // -48 to convert ascii to int
				progress++;
				putC('O');        // send letter O
				putC('K');        // send letter K
				putC(0x0D);         // send letter CR
				putC(0x0A);         // send letter LF
			}
			else if(readWord[0]=='P'){  //If command is P set flag play =1 to start solving 
				play=1;
				putC('O');        // send letter O
				putC('K');        // send letter K
				putC(0x0D);         // send letter CR
				putC(0x0A);         // send letter LF
			}
			else if(readWord[0]=='S'){  //if command is S send the first number of grid 
				putC('N');				  // send letter N
				putC('1');				  // send position X=0
				putC('1');				  // send position Y=0
				putC(grid[0][0]+48);  // send first number of grid
				putC(0x0D);               // send letter CR
				putC(0x0A);               // send letter LF
				sendXpos=0;
				sendYpos=1;
			}	
			else if(readWord[0]=='T'){  
				if(sendXpos==N){ // if we are DONE
					putC('D');				  // send letter D
					putC(0x0D);               // send letter CR
					putC(0x0A);               // send letter LF
				}
				else{
					putC('N');				  // send letter N
					putC(sendXpos+48+1);		  // send position X=0
					putC(sendYpos+48+1);		  // send position Y=0
					putC(grid[sendXpos][sendYpos]+48);  // send first number of grid
					putC(0x0D);               // send letter CR
					putC(0x0A);               // send letter LF
					sendYpos=sendYpos+1;
					if(sendYpos==N){sendYpos=0;sendXpos=sendXpos+1;}//if we sent all row send next row
												
				}
			}
			else if(readWord[0]=='B'){  
				play =0;     // it will force sudoku to stop (finish)
				putC('O');        // send letter O
				putC('K');        // send letter K
				putC(0x0D);               // send letter CR
				putC(0x0A);               // send letter LF
			}
			else if(readWord[0]=='D'){  
				putC('N');				  // send letter N
				putC(readWord[1]);				  // send position X=0
				putC(readWord[2]);				  // send position Y=0
				putC(grid[readWord[1]-48-1][readWord[2]-48-1]+48);  // send asked number (-48 and +48 to convert int to ascii)
				putC(0x0D);               // send letter CR
				putC(0x0A);               // send letter LF
			}
			letter=0;
			uint8_t i=0; 
			
			while(i<SizeOfReadWord){   //set readWord=0
				readWord[i]=0;
				i++;
			}	
		}
			asm(" pop r31\n\t\
				pop r30 \n\t\
				pop r29 \n\t\
				pop r28 \n\t\
				pop r27 \n\t\
				pop r26 \n\t\
				pop r25 \n\t\
				pop r24 \n\t\
				pop r23 \n\t\
				pop r22 \n\t\
				pop r21 \n\t\
				pop r20 \n\t\
				pop r19 \n\t\
				pop r18 \n\t\
				pop r17 \n\t\
				pop r16 \n\t\
				pop r0	 \n\t\
				out __SREG__,r0 \n\t\
				clr r0");
	reti();
}


//initialize USART
void initUART(){
	UCSRB=(1<<RXEN) | (1<<TXEN) | (1<<RXCIE);  // ; enable RXCIE=Interrupt enable ,RXEN=Receiver Enable, TXEN=Transmitter Enable
	UBRRL=(10000000/16/9600)-1;								  //; set baud rate 9600 with the formula (F_CPU/16/baud) - 1
	UCSRC=(1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
	return;
}

// initializations and infinite loop
int main(void)
{
		
	progress=0; // number of solved cells
	uint8_t i=0;
	while(i<SizeOfReadWord){
		readWord[i]=0;
		i++;
	}
	letter=0; // next letter of readWord we can write
	play=0; // flag that starts ving
	sendXpos=0; // position x of next number we wanna send
	sendYpos=0; // position y of next number we wanna send*/
	
	i=0;
	while(i<N){     // set grid array to 0
		uint8_t  j=0;
		while(j<N){
			grid[i][j]=0;
			j++;
		}
		i++;
	}//*/
	
	//SPL=0x5D;
	//SPH=0x04;// DEFAULT
	DDRB=0xff;  //; set portb as output
	PORTB=0;
	OCR1A=1536;				//; if the same value with counter then interrupt
	TIMSK=(1<<OCIE1A);      //;enable compare interrupt for timer1
	sei();				    // enable global interrupt
	TCCR1B=(1<<CS02);		//; starts the tcnt1l (counter 1B) with prescaler clk/256
	initUART();
	while(1){
		while (play==0){ //;infinite loop
			DDRB=0xff;
		}
			if (SolveSudoku() == 1){
				if(play==1){// if success (user didnt send break from usart)
					putC('D');        // send letter K
					putC(0x0D);       // send letter CR
					putC(0x0A);       // send letter LF
				}
				else{
					putC('E');        // send letter E for ERROR
					putC(0x0D);       // send letter CR
					putC(0x0A);       // send letter LF
				}
		}
		play=0;   // flag for infinite loop
	}
}

