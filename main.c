#include <at89c5131.h>
//#include <string.h>	//to use strcat function
#include "lcd.h"		//Header file with LCD interfacing functions
#include "serial.c"	//C file with UART interfacing functions
#include <stdlib.h>     /* atoi */
#include <string.h>

// Variable Initiallisations
code int denominations[4] = {2000,500,200,100};
code unsigned char denom_str[4][6] = {"2000,","500,","200,","100"};
int state[4] = {20,30,10,10};  // initial state of ATM, no. of various denominations to begin with
unsigned char state_str[4][3] = {"20","30","10","10"};
int cash[4] = {0,0,0,0};
unsigned char temp_str[6] = {0,0,0,0,0,'\0'};

//Runtime variable declarations
int denom;
unsigned char ch;  //recieving the user input from the keyboard
unsigned char input_amt[6] = {0,0,0,0,0,'\0'};
int index = 0;
int amt = 0;
int s2i_index = 0;
int clear_index = 0;
bit line1_rst = 0;
unsigned int disp_cmd;
int temp;

//Function Declarations
void denom_available(void);
void line2_clear(void);
void invalid_line2(void);
void clear_ip_amt(void);
void atm_internal(void);
void str_to_amt(void);

//Main function
void main(void)
{

//Call initialization functions
lcd_init();
uart_init();
	
//These strings will be printed in terminal software
transmit_string("************************\r\n");
transmit_string("*****ATM  Simulator*****\r\n");
transmit_string("************************\r\n");
transmit_string("Press d to know ATM State\r\n");
transmit_string("Enter a valid amount to be taken\r\n");
	
	
while(1)
	{

//Receive a character
ch = receive_char();

		if (input_amt[0]==0)	
		{
			if (ch=='d')		//displays the current state of the ATM
			{
				lcd_cmd(0x80);		//clearing line 1 before displaying
				lcd_write_string("                ");
				
				state_to_string(state,state_str);		//converts integer state values to string values
			
				lcd_cmd(0x81);	//Move cursor to 1st line column 2 of LCD
				lcd_write_string("2000:");
				lcd_write_string(*(state_str));
				lcd_write_string(" 500:");
				lcd_write_string(*(state_str+1));
			
				lcd_cmd(0xC1);	//Move cursor to 2nd line column 1 of LCD
				lcd_write_string("200:");
				lcd_write_string(*(state_str+2));
				lcd_write_string("  100:");
				lcd_write_string(*(state_str+3));

				msdelay(3000);
				lcd_cmd(0x80);
				lcd_write_string("                ");
				line2_clear();
			}
			
			else
			{
				index = 0;
				if (ch>48 && ch<58)		//numerical input greater than 0
					{
						input_amt[index] = ch;
						index++;
						//write the value on the second line
						lcd_cmd(0xC0);
						lcd_write_char(ch);
						disp_cmd = 0xC0;
					}
				else	//invalid character entered
					{
						invalid_line2();
					}	
			}			
		}
		
		else
		{
			if (ch == 13) // Enter Key is pressed
			{
//												lcd_cmd(0xC0);
//												lcd_write_string("                ");
//												lcd_cmd(0xC1);	//Move cursor to 2nd line column 1 of LCD								
//												lcd_write_string("Enter Detected");
//												msdelay(2000);
//												line2_clear();
				//ES = 0; //Disabling serial interrupt
				input_amt[index] = '\0';
				lcd_cmd(0xC0);
				lcd_write_string(input_amt);
				msdelay(2000);
				atm_internal();
				//ES = 1; // Re-enabling the serial interrupt
			}
			
			else
			{
				if (ch>='0' && ch<='9')	//valid number entered
				{
					input_amt[index] = ch;
					disp_cmd++;
					index++;
					lcd_cmd(disp_cmd);
					lcd_write_char(input_amt[index-1]);						
				}
				else	//invalid key pressed
				{
					invalid_line2();
				}
			}
		}
		
		denom_available();	//Shows Available denominations on the first line of LCD
		
	msdelay(100);
	}
}

//Available Denomination display function
void denom_available(void)
{
	lcd_cmd(0x80); 
	for(denom=0;denom<4;denom++)
	{
		if (state[denom] != 0) {lcd_write_string(*(denom_str+denom));}
	}	
}

//LCD Clear function
void line2_clear(void)
{
	lcd_cmd(0xC0);
	lcd_write_string("                ");
}

//Invalid Input Line 2 Display
void invalid_line2(void)
{
	lcd_cmd(0xC0);
	lcd_write_string("                ");
	lcd_cmd(0xC1);	//Move cursor to 2nd line column 1 of LCD								
	lcd_write_string("Invalid  Input");
	msdelay(2000);
	line2_clear();
}

//Clearing Input Amount String
void clear_ip_amt(void)
{
	for (clear_index=0;clear_index<5;clear_index++)
	{
		input_amt[clear_index] = 0;
	}
	input_amt[5] = '\0';
}

//Internal Processing by the ATM Machine
void atm_internal(void)
{
	//str_to_amt();	//obtain the integer value of amount entered by the user
	
	amt = atoi(input_amt);
//	int_to_string(amt,temp_str);
	
	
//	amt_temp=to_string(amt);
//	lcd_cmd(0xC0);
//	lcd_write_string(temp_str);
//	msdelay(2000);
	
	clear_ip_amt();
	
	if (amt%100 == 0)
	{
		for(denom=0;denom<4;denom++)
		{
			if (state[denom] != 0)
			{
				cash[denom] = amt/denominations[denom];
				if (state[denom] - cash[denom]>=0) {}	//sufficient denominations available
				else
				{
					line1_rst = 1;		//Line 1 would have to be reset if final cash payment happens
					cash[denom] = state[denom];
				}
				amt = amt - (cash[denom]*denominations[denom]);
			}
			else{}
		}
				
		if (amt == 0)		//Transaction can be made successfully
		{
			lcd_cmd(0xC2);
			lcd_write_string("Collect Cash");
			for(denom=0;denom<4;denom++)	//updating the ATM state, will only update if entered amount can be successfully cashed out
			{
				state[denom] = state[denom] - cash[denom];
			}
			if (line1_rst) {denom_available();} 	//updating the Line 1
		}
		else	//Combination can't be formed for the entered amount with the current ATM State
		{
			lcd_cmd(0xC3);
			lcd_write_string("Try Again");
			msdelay(2000);
			line2_clear();			
		}
		for(denom=0;denom<4;denom++)	//resetting the cash[]
		{
			cash[denom] = 0;
		}
		line1_rst = 0; // resetting the line1 update indicator

	}
	else	//invalid amount entered that can't be taken out, amount value doesn't end at two zeros
	{
		invalid_line2();
	}
}

//String to Integer Amount
//void str_to_amt(void)
//{
//	amt = 0;
//	amt = atoi(input_amt);
////	for(s2i_index=0;s2i_index<index;s2i_index++)
////		{
////			temp = 10*amt;
////			amt = (temp) + (input_amt[s2i_index] - 48);
////		}
//	clear_ip_amt();
//	index = 0;
//}
