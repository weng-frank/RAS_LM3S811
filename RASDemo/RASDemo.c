#include <stdlib.h>
#include <ctype.h>
#include "inc/hw_types.h"		// tBoolean
#include "driverlib/adc.h"		// ADC functions
#include "driverlib/uart.h"
#include "utils/uartstdio.h"	// input/output over UART
#include "RASLib/init.h"
#include "RASLib/encoder.h"
#include "RASLib/linesensor.h"
#include "RASLib/motor.h"
#include "RASLib/servo.h"
#include "RASLib/timer.h"
#include "RASLib/encoder.h"

#define PRINT_LS(ls)	for(i=0; i<NUM_SENSORS; ++i)\
							UARTputc((ls&(1<<i)) ? '1' : '0')
#define LS(ls, i)		(((ls) & (1<<(i)))>>((i)-1))

// demo logic
void motorDemo(void);
void servoDemo(void);
void lsDemo(void);
void adcDemo(void);
void encDemo(void);

// print the menus for each demo
void mainMenu(void);
void motorMenu(void);
void servoMenu(void);
void lsMenu(void);
void adcMenu(void);
void encMenu(void);
			  
// Main states
enum main_e {
	MAIN_MENU		= 0x1B,	// ESCAPE
	MOTOR_DEMO		= 'M',
	SERVO_DEMO		= 'S',
	LS_DEMO			= 'L',	// linesensor demo
	ADC_DEMO		= 'A',
	ENC_DEMO		= 'E',	// encoder demo
	} main_state = MAIN_MENU;

// Motor states
enum motor_e {
	MOTOR_IDLE		= ' ',
	MOTOR_FORWARD	= 'W',
	MOTOR_BACKWARD	= 'S',
	MOTOR_LEFT		= 'A',
	MOTOR_RIGHT		= 'D',
	MOTOR_INVERT1	= '[',
	MOTOR_INVERT2	= ']',
	MOTOR_VELOCITY	= 'V',	// set base velocity
	} motor_state = MOTOR_IDLE;

// Servo states
enum servo_e {
	SERVO_IDLE		= ' ',
	SERVO_ANGLE		= 'P',	// set angle
	} servo_state = SERVO_IDLE;

// Line sensor states
enum ls_e {
	LS_IDLE			= ' ',				  
	LS_ASYNC		= 'A',	// asynchronous operation
	LS_SYNC			= 'S',	// synchronous operation
	LS_DISCHARGE	= 'D',	// set discharge time	 
	LS_CONT_READ_S	= 'C',	// countinuously read and display line sensor values, synchronously
	LS_CONT_READ_A	= 'V',	// countinuously read and display line sensor values, asynchronously 
	LS_WAIT			= 'W',	// set wait period (in ms) between continuous readings
	LS_PIN_0		= '0',	// Display pin 0
	LS_PIN_1		= '1',	// Display pin 1
	LS_PIN_2		= '2',	// Display pin 2
	LS_PIN_3		= '3',	// Display pin 3
	LS_PIN_4		= '4',	// Display pin 4
	LS_PIN_5		= '5',	// Display pin 5
	LS_PIN_6		= '6',	// Display pin 6
	LS_PIN_7		= '7',	// Display pin 7
	LS_PIN_8		= '8',	// Display all 8 pins
	} ls_state = LS_SYNC;

// ADC states
enum adc_e {
	ADC_IDLE		= ' ',
	ADC_CONT_READ	= 'C',	// read and display values continuously
	ADC_SERVO		= 'S',	// set the servo according to the ADC value
	ADC_WAIT		= 'W',	// set the wait time for continuous read
	ADC_0			= '0',
	ADC_1			= '1',
	ADC_2			= '2',
	ADC_3			= '3',
	} adc_state	= ADC_IDLE;

// encoder states
enum encoder_e {
	ENC_IDLE		= ' ',
	ENC_0			= '0',	// read encoder 0
	ENC_1			= '1',	// read encoder 1
	ENC_2			= '2',	// read both encoders
	ENC_RESET		= 'R',	// reset encoder counts
	ENC_CONTINUOUS	= 'C',	// read continuously
	ENC_WAIT		= 'W',	// set wait period (in ms) between continuous readings
	} encoder_state = ENC_IDLE;

#define BUF_LEN 80
char buffer[BUF_LEN];

int main(void)
{
	LockoutProtection();
	InitializeMCU();

	InitializeUART();
	InitializeLineSensorAsync();
	InitializeMotors(false, false);
	InitializeServos();
	InitializeEncoders(false, false);
	
	//ADC Stuff
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);
	ADCSequenceConfigure(ADC_BASE,0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
	ADCSequenceEnable(ADC_BASE, 0);//*/

	PRINT("\n\n\
Welcome to RASDemo. The idea is to use the source code of this project as\n\
a set of examples of how to program your robot to do some simple things.\n\
The menus will show what keys you can press and what they will do.\n\
");

	// display the main menu
	mainMenu();
	
	while(1)	// never allow main() to exit when working with microcontrollers!
	{
		unsigned char c = UARTgetc();
		if(islower(c))
			c = c + 'A' - 'a';	// convert from upper to lowercase
		switch(c)
		{
			case MOTOR_DEMO:
				main_state = MOTOR_DEMO;
				motorDemo();
				break;
			case SERVO_DEMO:  
				main_state = SERVO_DEMO;
				servoDemo();
				break;
			case ADC_DEMO: 
				main_state = ADC_DEMO;
				adcDemo();
				break;
			case LS_DEMO: 
				main_state = LS_DEMO;
				lsDemo();						   
				break;
			case ENC_DEMO:
				main_state = ENC_DEMO;
				encDemo();
				break;
			default:
				PRINT("That was an invalid choice. Please try again.\n");
				break;					
		}
		mainMenu();
	}
}

void mainMenu(void)
{
	PRINT("\
MAIN MENU:\n\
Key\tAction\n\
M\tMotor Demo\n\
S\tServo Demo\n\
L\tLine Sensor Demo\n\
A\tADC Demo\n\
E\tEncoder Demo\n");
}


void motorDemo(void)								   
{
	static power_t base_velocity = 64;

	motorMenu();

	while(1)
	{
		unsigned char c = UARTgetc();
		if(islower(c))	// convert from upper to lowercase
			c = c + 'A' - 'a';
		
		if(c == MOTOR_VELOCITY)
		{
			PRINT("Enter a new value for base_velocity: ");
			UARTgets(buffer, BUF_LEN); NL;
			base_velocity = atoi(buffer);
			PRINT("Setting "); PRINT_D(base_velocity); NL;
			c = motor_state;
		}	
		 
		switch(c)
		{
		case MAIN_MENU:
			return;
		case MOTOR_IDLE:
			SetMotorPowers(0, 0);
			break;
		case MOTOR_FORWARD:
			SetMotorPowers(base_velocity, base_velocity);
			break;
		case MOTOR_BACKWARD:
			SetMotorPowers(-base_velocity, -base_velocity);
			break;
		case MOTOR_LEFT:
			SetMotorPowers(-base_velocity, base_velocity);
			break;
		case MOTOR_RIGHT:
			SetMotorPowers(base_velocity, -base_velocity);
			break;
		default:
			c = motor_state;
			motorMenu();
			break;
		}
		motor_state = (enum motor_e)c;
	}
}

void motorMenu(void)
{	
	PRINT("\
MOTOR MENU:\n\
ESC\tReturn to Main Menu\n\
 \tIdle\n\
W\tGo forward\n\
S\tGo backward\n\
A\tSpin left\n\
D\tSpin right\n\
V\tSet base velocity\n");
}

void servoDemo(void)
{
	static position_t position = SERVO_NEUTRAL_POSITION;

	servoMenu();

	while(1)
	{
		char c;
		if(UARTIsCharAvail())
		{
			c= UARTgetc();
			if(islower(c))
				c = c + 'A' - 'a'; 
			switch(c)
			{
			case MAIN_MENU:
				return;
			case SERVO_IDLE:
			case SERVO_ANGLE:
				break;
			default:
				c = servo_state;
				servoMenu();
				break;
			}
			servo_state = (enum servo_e)c;
		} 
		switch(servo_state)
		{
			case SERVO_IDLE:
				break;
			case SERVO_ANGLE:
				PRINT("Enter a new value for position: ");
				UARTgets(buffer, BUF_LEN); NL;
				position = atoi(buffer);
				SetServoPosition(SERVO_0, position);
				servo_state = SERVO_IDLE;
				break;
		}
	}
}

void servoMenu(void)
{	
	PRINT("\
SERVO MENU:\n\
ESC\tReturn to Main Menu\n\
 \tIdle\n\
P\tSet the servo power\n");
}

void lsDemo(void)
{
	char action = LS_IDLE;
	linesensor_t ls = 0;
	static unsigned wait = 100;
	static unsigned discharge_time = 1000;
	int i = 0;
	linesensor_t mask = 0;

	lsMenu();

	while(1)
	{
		char c;
		if(UARTIsCharAvail())
		{
			c = UARTgetc();
			if(islower(c))
				c = c + 'A' - 'a'; 
			switch(c)
			{
			case MAIN_MENU:
				return;
			case LS_ASYNC:
			case LS_SYNC:
			case LS_CONT_READ_S:
			case LS_CONT_READ_A:
				break;
			case LS_IDLE:
				if(ls_state==LS_CONT_READ_S)
					c = LS_SYNC;
				else if(ls_state==LS_CONT_READ_A)
					c = LS_ASYNC;
				break;
			case LS_PIN_0:
			case LS_PIN_1:
			case LS_PIN_2:
			case LS_PIN_3:
			case LS_PIN_4:
			case LS_PIN_5:
			case LS_PIN_6:
			case LS_PIN_7:
				i = c - '0';	// convert the ASCII char to a digit
			case LS_DISCHARGE:
			case LS_PIN_8:
			case LS_WAIT:
				action = c;
				c = ls_state;
				break;
			default:
				c = ls_state;
				lsMenu();
				break;
			}
			ls_state = (enum ls_e)c;
		}
		
		switch(action)
		{
		case LS_DISCHARGE:		
			PRINT("Enter a new value for discharge_time: ");
			UARTgets(buffer, BUF_LEN); NL;
			discharge_time = atoi(buffer);
			PRINT("Setting "); PRINT_U(discharge_time); NL;
			SetDischargeTime(discharge_time);
			break;
		case LS_PIN_0:
		case LS_PIN_1:
		case LS_PIN_2:
		case LS_PIN_3:
		case LS_PIN_4:
		case LS_PIN_5:
		case LS_PIN_6:
		case LS_PIN_7:
			mask = 1<<i;
		case LS_PIN_8:
			switch(ls_state)
			{
			case LS_CONT_READ_A:
				ls_state = LS_ASYNC;
			case LS_ASYNC:
				ls = ReadLineSensorAsync();
				break;
			case LS_CONT_READ_S:
				ls_state = LS_SYNC;
			case LS_SYNC:
				ls = ReadLineSensor();
				break;
			}
			if(action == LS_PIN_8)
				PRINT_LS(ls);
			else
				if(ls & mask)
					UARTputc('1');
				else
					UARTputc('0');
			NL;
			break;
		case LS_WAIT:			   
			PRINT("Enter a new value for wait: ");
			UARTgets(buffer, BUF_LEN); NL;
			wait = atoi(buffer);
			PRINT("Setting "); PRINT_U(wait); NL;
			break;
		case LS_IDLE:
			break;
		}
		action = LS_IDLE; 
		switch(ls_state)
		{
		case LS_ASYNC:
		case LS_SYNC:
			break;
		case LS_CONT_READ_S:
		case LS_CONT_READ_A:
			if(ls_state == LS_CONT_READ_S)	
				ls = ReadLineSensor();
			else if(ls_state == LS_CONT_READ_A)
				ls = ReadLineSensorAsync();
			PRINT_LS(ls); NL;
			Wait(wait);
			break;
		}
	}
}

void lsMenu(void)
{
	PRINT("\
LINE SENSOR MENU:\n\
ESC\tReturn to Main Menu\n\
 \tIdle\n\
A\tAsynchronous\n\
S\tSynchronous\n\
D\tSet a new discharge time\n\
R\tRead the current line sensor value\n\
0-7\tRead the pin\n\
C\tContinuous, asynchronous read\n\
V\tContinuous, synchronous read\n\
W\tSet the wait time between iterations in continuous read\n");
}

void adcDemo(void)
{
	unsigned long ulADCValue;
	static unsigned wait = 100;
	adcMenu();

	while(1)
	{
		char c = UARTgetc();
		if(islower(c))
			c = c + 'A' - 'a'; 
		switch(c)
		{
		case MAIN_MENU:
			return;
		case ADC_CONT_READ:
		case ADC_SERVO:
		case ADC_IDLE:
			break;
		case ADC_WAIT:			   
			PRINT("Enter a new value for wait: ");
			UARTgets(buffer, BUF_LEN); NL;
			wait = atoi(buffer);
			PRINT("Setting "); PRINT_U(wait); NL;
			c = adc_state;
			break;
		default:
			adcMenu();
			c = adc_state;
			break;
		}
		adc_state = (enum adc_e)c;
		
		switch(adc_state)
		{
			case ADC_IDLE:
				break;			
			case ADC_SERVO:
			case ADC_CONT_READ:
				ADCProcessorTrigger(ADC_BASE, 0 ); 
				while(!ADCIntStatus(ADC_BASE, 0, false)); 
				ADCSequenceDataGet(ADC_BASE, 0, &ulADCValue);
				if(adc_state == ADC_SERVO)
					SetServoPosition( SERVO_1, (SERVO_MAX_POSITION*ulADCValue)/1023 );
				PRINT_U(ulADCValue); NL;
				Wait(wait);
				break;
		}
	}
}

void adcMenu(void)
{	
	PRINT("\
ADC MENU:\n\
ESC\tReturn to Main Menu\n\
 \tIdle\n\
C\tContinuously read the pot\n\
S\tControl the servo with the pot\n\
W\tSet the wait time in between iterations\n");
}

void encDemo(void)
{
	encoder_count_t enc0, enc1;
	static unsigned wait = 100;

	encMenu();

	while(1)
	{
		char c;
		if(UARTIsCharAvail())
		{
			c= UARTgetc();
			if(islower(c))
				c = c + 'A' - 'a'; 
			switch(c)
			{
			case MAIN_MENU:
				return;
			case ENC_IDLE:
			case ENC_CONTINUOUS:
				encoder_state = (enum encoder_e)c;
				break;
			case ENC_0:
				enc0 = GetEncoderCount(ENCODER_0);
				PRINT_D(enc0); NL;
				break;
			case ENC_1:
				enc1 = GetEncoderCount(ENCODER_1);
				PRINT_D(enc1); NL;
				break;
			case ENC_2:
				GetEncoderCounts(&enc0, &enc1);
				PRINT_D(enc0); TAB; PRINT_D(enc1); NL;
				break;
			case ENC_RESET:
				PresetEncoderCounts(0, 0);
				PRINT("Reset the encoders to (0, 0)\n");
				break;
			case ENC_WAIT:			   
				PRINT("Enter a new value for wait: ");
				UARTgets(buffer, BUF_LEN); NL;
				wait = atoi(buffer);
				PRINT("Setting "); PRINT_U(wait); NL;
				break;
			default:
				encMenu();
				break;
			}
		} 
		switch(encoder_state)
		{
			case ENC_CONTINUOUS:
				GetEncoderCounts(&enc0, &enc1);
				PRINT_D(enc0); TAB; PRINT_D(enc1); NL;
				Wait(wait);
				break;
		}
	}
}

void encMenu(void)
{
	PRINT("\
ENCODER MENU:\n\
ESC\tReturn to Main Menu\n\
 \tIdle\n\
0\tread encoder 0\n\
1\tread encoder 1\n\
2\tread both encoders\n\
R\treset encoder counts\n\
C\tread continuously\n\
W\tset wait period (in ms) between continuous readings\n");
}
