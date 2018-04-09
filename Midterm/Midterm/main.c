/*
 * Midterm.c
 *
 * Created: 4/7/2018 5:02:39 PM
 * Author : Tenniel Takenaka-Fuller
 */ 

 /*Library Functions*/
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#define F_CPU 16000000UL


 /*Baud rate for ports*/
 #define FOSC 16000000 // Clock Speed
 #define BAUD 115200 //Baud rate of ESP8266-01
 #define MYUBRR FOSC/8/BAUD -1

/*Function prototypes*/
void send_AT (volatile unsigned char c[]); //sends the arrays to this function

/*Global variables*/
volatile uint8_t ADCvalue; // Volatile if used with interrupts
volatile unsigned char ADCtemp[5];

/*Arrays for AT firmware*/
volatile unsigned char AT[] = "AT\r\n"; //Test AT startup
volatile unsigned char FIRM[] = "AT+GMR\r\n"; //Display firmware version number
volatile unsigned char CWMODE[] = "AT+CWMODE=3\r\n"; //Set to three for wifi mode
volatile unsigned char WEEFEE[] = "AT+CWJAP=\"Autobots\", \"06060606\"\r\n"; //Connect to wifi
volatile unsigned char ENABLE[] = "AT+CIPMUX=0\r\n"; //Single connection
volatile unsigned char CIPSTART[] = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n"; //TCP is the type, Address of strip remote IP, 80 is port
volatile unsigned char CIPSEND[] = "AT+CIPSEND=45\r\n"; //45 is the length of the data to be sent 
volatile unsigned char SEND_DATA[] = "GET /update?key=P6WSZH1BETJPZGQA&field1="; //Write API key
volatile unsigned char PAUSE[] = "\r\n\r\n"; 

 int main( void )
 {
	 
	 //---------------------------------------------------------------
	 // INITIALIZE ADC VALUES
	 //---------------------------------------------------------------

	 ADMUX = 0; // use ADC0
	 ADMUX |= (1 << REFS0); // use AVcc as the reference
	 ADMUX |= (1 << ADLAR); // Right adjust for 8 bit resolution
	 ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescale for 16Mhz
	 ADCSRA |= (1 << ADATE); // Set ADC Auto Trigger Enable
	 ADCSRB = 0; // 0 for free running mode
	 ADCSRA |= (1 << ADEN); // Enable the ADC
	 ADCSRA |= (1 << ADIE); // Enable Interrupts
	 ADCSRA |= (1 << ADSC); // Start the ADC conversion
	 
	 //---------------------------------------------------------------
	 // INITIALIZE UART VALUES
	 //---------------------------------------------------------------
	 
	 /*Set baud rate */
	 UBRR0H = ((MYUBRR) >> 8); //Shift to store the upper 8 bits
	 UBRR0L = MYUBRR; //Store lower bits

   	UCSR0A |= (1 << U2X0); //Double the USART transmission speed
	 UCSR0B |= (1 << TXEN0); // Enable transmitter
	 UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // Set frame: 8data, 1 stop, no parity bit
	 	 
	 //---------------------------------------------------------------
	 // BEGIN USE OF AT COMMANDS
	 //---------------------------------------------------------------
	 
	 _delay_ms(1000); //Test AT
	 send_AT(AT);
	 
	_delay_ms(1000); //Display firmware
	 send_AT(FIRM);
	 
	 _delay_ms(1000); //Set to wifi mode
	 send_AT(CWMODE);
	  
	 _delay_ms(1000); //connect to home wifi
	 send_AT(WEEFEE);
	 
	 _delay_ms(2000); //enable the single connection
	 send_AT(ENABLE);
	 	  

	  sei();	//Enable ADC interrupt vector

	 
	 while(1)
	 {
		//---------------------------------------------------------------
		// AT COMMANDS TO REPEAT FOREVER
		//---------------------------------------------------------------	 
		 
		_delay_ms(1000); //Begin wifi connection
		 send_AT(CIPSTART);

		 _delay_ms(1000); //Length
		 send_AT(CIPSEND);

		 _delay_ms(1000); 
		 send_AT(SEND_DATA); //Uses the API key to send the data
		 
		 send_AT(ADCtemp); //Sends the temperature data
		 
		 send_AT(PAUSE); //A break in the data display
	 }
 }
 
 
 
 
ISR(ADC_vect)
 {
	 volatile unsigned int j;
	 volatile unsigned int lngth;
	char convertTemp[5];
	lngth = 5;
	j = 0;
	 
	ADCvalue = (ADCH << 1); //Shifts the value left to one place
	
	/*convert character to ASCII decimal, hence the 10*/
	itoa(ADCvalue, convertTemp, 10);
	
	/*Store the converted temperature into ADCtemp one bit at a time*/
	while (j<5) 
	{
		ADCtemp[j] = convertTemp[j];
		j = j+1;
	}
 }





void send_AT(volatile unsigned char c[])
 {
	
	volatile unsigned int i=0;
	volatile unsigned int variableLength;
	
	variableLength = 0; //initialize counter
	
	while (c[variableLength] != 0x00) //while not at end of string
	{
		variableLength++; //increment counter to calculate until NULL
	}
	
	while (i<variableLength)
	{
		while(!(UCSR0A & (1 << UDRE0))); //If UDRE0 is 1, buffer is empty & ready to be written to
		UDR0 = c[i];
		i = i+1;
	}

}



 