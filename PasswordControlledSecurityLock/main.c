/*
	Made by Tony Pallone
	
	Programmed on Atmega328P using a 4x3 keypad and a 8x2 LCD display so
	things that are defined for those will be mentioned
*/
#define F_CPU 8000000L    //CPU speed on the Atmega328P
#include <avr/io.h>
#include <util/delay.h>

#define LCD_DPRT PORTB //LCD DATA PORT
#define LCD_DDDR DDRB  //LCD DATA DDR
#define LCD_DPIN PINB  //LCD DATA PIN
#define LCD_CPRT PORTC //LCD COMMANDS PORT
#define LCD_CDDR DDRC  //LCD COMMANDS DDR
#define LCD_CPIN PINC  //LCD COMMANDS PIN
#define LCD_RS  0      //LCD RS   (PC.0)
#define LCD_RW  1      //LCD RW   (PC.1)
#define LCD_EN  2      //LCD EN   (PC.2)
#define KEY_PRT PORTD  //Keyboard PORT
#define KEY_DDR DDRD   //Keyboard DDR
#define KEY_PIN PIND   //Keyboard PIN

int counter = 0;  		 //Used to see how many characters are printed

//****************************************************************
void lcdCommand (unsigned char cmd) {
	LCD_DPRT = cmd;				//send cmd to data port
	LCD_CPRT &= ~(1<<LCD_RS);   //RS = 0 for command
	LCD_CPRT &= ~(1<<LCD_RW);   //RW = 0 for write
	LCD_CPRT |= (1<<LCD_EN);    //EN = 1 for H-to-L pulse
	_delay_us(1);					//wait to make enable wide
	LCD_CPRT &= ~(1<<LCD_EN);   //EN = 0 for H-to_L pulse
	_delay_us(100);				//wait to make enable wide
}

void lcdData(unsigned char data) {
	if (counter == 8)		// if its at 8 go to (1,2) (this is here because its an 8x2 LED)
	{
		lcd_gotoxy(1,2);
	}
	if (counter == 16)
	{
		lcdCommand(0x01);	 // if it gets to 16 reset it to the beginning
		lcd_gotoxy(1,1);
		counter = 0;
	}
	LCD_DPRT = data;				//send data to data port
	LCD_CPRT |= (1<<LCD_RS);		//RS = 1 for data
	LCD_CPRT &= ~(1<<LCD_RW);   //RW = 0 for write
	LCD_CPRT |= (1<<LCD_EN);		//EN = 1 for H-to-L pulse
	_delay_us(1);					//wait to make enable wide
	LCD_CPRT &= ~(1<<LCD_EN);   //EN = 0 for H-to_L pulse
	_delay_us(100);				//wait to make enable wide
	counter ++;
}

void lcd_init() {
	LCD_DDDR = 0xFF;
	LCD_CDDR = 0xFF;
	LCD_CPRT &=~(1<<LCD_EN);   //LCD_EN = 0
	_delay_us(2000);
	//wait for init
	lcdCommand(0x38);   //initialize LCD 2 line, 5x7
	lcdCommand(0x0E);   //display on, cursor on
	lcdCommand(0x01);   //clear LCD
	_delay_us(2000);		 //wait
	lcdCommand(0x06);   //shift cursor right
}

void lcd_print(char * str) {
	unsigned char i = 0;

	while (str[i]!=0)  {
		lcdData(str[i]); i++;
	}
}

// go to specific LCD locations
void lcd_gotoxy(unsigned char x, unsigned char y) {
	unsigned char firstCharAdr[] = {0x80, 0xC0};   // locations of the first character of each line
	if (x == 1, y == 1)
	{
		counter = 0;			// just to make sure when counter is called at this location it goes to 0
	}
	if (x == 1, y == 2)
	{
		counter = 8;	// just to make sure when counter is called at this location it goes to 8
	}
	lcdCommand(firstCharAdr[y-1] + x-1);
	_delay_us(100);
}


int main(void) {
	// initialize LCD
	char colloc, rowloc, printCharacter, password, count; // password and count are used to know when someone enters the password
	char initialcount = 0;
	char userPassword[3] ={'\0','\0','\0'};		 // initially all nulls 
	char keypad[4][3] = {'1','2','3',    // my keypad inputs mapped here
						 '4','5','6',
						 '7','8','9',
						 '*','0','#'};
	lcd_init();		//initialization
	KEY_DDR = 0x0F;
	KEY_PRT = 0x7F;
	
	while (initialcount < 3){
		do
		{
			KEY_PRT &= 0x70;			// make sure no buttons are being pressed yet
			colloc = (KEY_PIN & 0x70);
		}while (colloc != 0x70);  // repeat until someone stops pushing the button

		do
		{
			do
			{
				_delay_ms(20);
				colloc = (KEY_PIN & 0x70);
			} while (colloc == 0x70);  // see if someone pushed a button
			_delay_ms(20);
			colloc = (KEY_PIN & 0x70);
		} while (colloc == 0x70); // this is here to be certain someone pushed a button

		while(1){
			KEY_PRT = 0x7E;    // we ground the first row to see if the thing being pressed is there
			_delay_ms(1);			 // the delay is here because otherwise the hardware needed time to ground it
			colloc = (KEY_PIN & 0x70);
			if (colloc != 0x70){
				rowloc = 0;  // save row location
				break;
			}
			KEY_PRT = 0x7D; 		// we ground the second row to see if the thing being pressed is there
			_delay_ms(10); 			// the delay is here because otherwise the hardware needed time to ground it
			colloc = (KEY_PIN & 0x70);
			if (colloc != 0x70){
				rowloc = 1;				// save row location
				break;
			}
			KEY_PRT = 0x7B;			// we ground the third row to see if the thing being pressed is there
			_delay_ms(10);			// the delay is here because otherwise the hardware needed time to ground it
			colloc = (KEY_PIN & 0x70);
			if (colloc != 0x70){
				rowloc = 2; 			// save row location
				break;
			}
			KEY_PRT = 0x77;
			_delay_ms(10);
			colloc = (KEY_PIN & 0x70);  // If its not any of the past rows we know its the third one
			rowloc = 3;
			break;
		}
		if (colloc == 0x60)
		{
			userPassword[initialcount] = keypad[rowloc][0];
		}
		else if (colloc == 0x50)
		{
			userPassword[initialcount] = keypad[rowloc][1];
		}
		else if (colloc == 0x30)
		{
			userPassword[initialcount] = keypad[rowloc][2];
		}
		
		initialcount++;
		if (initialcount == 3)
		{
			_delay_ms(100);
			count = 0;
			lcd_gotoxy(1,1);
			lcd_print("Pass Entered"); 
			
			_delay_ms(300);
			lcdCommand(0x01);
			lcd_gotoxy(1,1);
			password = 0;    // reset everything
		}

	}
	
	
	while (1){
	 do
	{
		KEY_PRT &= 0x70;			// make sure no buttons are being pressed yet
		colloc = (KEY_PIN & 0x70);
	}while (colloc != 0x70);  // repeat until someone stops pushing the button

	do
	{
		do
		{
			_delay_ms(20);
			colloc = (KEY_PIN & 0x70);
		} while (colloc == 0x70);  // see if someone pushed a button
		_delay_ms(20);
		colloc = (KEY_PIN & 0x70);
	} while (colloc == 0x70); // this is here to be certain someone pushed a button

	while(1){
		KEY_PRT = 0x7E;    // we ground the first row to see if the thing being pressed is there
		_delay_ms(1);			 // the delay is here because otherwise the hardware needed time to ground it
		colloc = (KEY_PIN & 0x70);
		if (colloc != 0x70){
			rowloc = 0;  // save row location
			break;
		}
		KEY_PRT = 0x7D; 		// we ground the second row to see if the thing being pressed is there
		_delay_ms(10); 			// the delay is here because otherwise the hardware needed time to ground it
		colloc = (KEY_PIN & 0x70);
		if (colloc != 0x70){
			rowloc = 1;				// save row location
			break;
		}
		KEY_PRT = 0x7B;			// we ground the third row to see if the thing being pressed is there
		_delay_ms(10);			// the delay is here because otherwise the hardware needed time to ground it
		colloc = (KEY_PIN & 0x70);
		if (colloc != 0x70){
			rowloc = 2; 			// save row location
			break;
		}
		KEY_PRT = 0x77;
		_delay_ms(10);
		colloc = (KEY_PIN & 0x70);  // If its not any of the past rows we know its the third one
		rowloc = 3;
		break;
		}
		if (colloc == 0x60)
		{
			printCharacter = keypad[rowloc][0];
			lcdData(printCharacter);
		}
		else if (colloc == 0x50)
		{
			printCharacter = keypad[rowloc][1];
			lcdData(printCharacter);
		}
		else if (colloc == 0x30)
		{
			printCharacter = keypad[rowloc][2];
			lcdData(printCharacter);
		}/*
		if ((printCharacter == userPassword[0]) && (password == 0))  
		{
			password ++;													// ugly legacy code dunno if I need it
		}
		else if ((printCharacter == userPassword[1]) && (password == 1))
		{
			password ++;
		}
		else if ((printCharacter == userPassword[2]) && (password == 2))
		{
			password ++;
		}*/
		
		if (printCharacter == userPassword[count]){
			password ++;
		}
		else
		{
			password = 0;
		}
		count++;
		if (count == 3)
		{
			_delay_ms(100);
			count = 0;
			if (password == 3)
			{
				lcd_gotoxy(1,1);
				lcd_print("Correct");  // password was entered correctly SUCCESS
			}
			else
			{
				lcd_gotoxy(1,1);
				lcd_print("Incorrect"); // password was entered correctly SAD!
			}
			_delay_ms(300);
			lcdCommand(0x01);
			lcd_gotoxy(1,1);
			password = 0;    // reset everything
		}

	}
	return 0;
}