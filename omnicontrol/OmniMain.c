/*
 * OmniMain.c
 *
 * Created: 1/28/2015
 * Updated: 6/23/2015
 *  Author: Dan Fisher
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "DCMotor.h"

// 50 RC cycles ~ 1s
#define LASERWAIT 250	// Number of RC cycles for laser to recharge before firing
#define LEDBARWAIT 71	// Number of RC cycles for LED bar to increment by 1 (bar goes from 1 to 8, or 7 intervals

// Start of channel interval
volatile uint16_t ch1_start = 0;
volatile uint16_t ch2_start = 0;
volatile uint16_t ch3_start = 0;
volatile uint16_t ch4_start = 0;
volatile uint16_t ch5_start = 0;
volatile uint16_t ch6_start = 0;

// State of channel (high 1 or low 0)
volatile uint8_t ch1_state = 0;
volatile uint8_t ch2_state = 0;
volatile uint8_t ch3_state = 0;
volatile uint8_t ch4_state = 0;
volatile uint8_t ch5_state = 0;
volatile uint8_t ch6_state = 0;

// Stick positions in us, initialized to neutral stick positions
volatile int16_t ch1 = 1500;
volatile int16_t ch2 = 1500;
volatile int16_t ch3 = 1500;
volatile int16_t ch4 = 1500;
volatile int16_t ch5 = 1500;
volatile int16_t ch6 = 1500;

// Flag is set after each cycle of RC pulses
volatile uint8_t start_flag = 0;

// Function prototypes
void Timer1Init(void);
void shiftByteOut(uint8_t byteOut);

int main(void) {	
	DDRB |= (1 << PORTB1);	// Laser output pin, HIGH = laser on
	DDRD |= (1 << PORTD0) | (1 << PORTD1) | (1 << PORTD2);	// Enable pins for shift register output
	
	/*
	// Knight Rider led variables
	uint16_t ledCount = 1;
	uint8_t ledPosition = 0;
	int8_t ledIncrement = -1;
	uint8_t ledDisplay = 0b00000001;*/
	
	// Laser LED bar variables
	uint16_t ledBarCounter = 1;
	uint8_t ledBarIndicator = 1;
	uint8_t ledBarCycle = 1;
	
	uint8_t laserState = 0;		// 1 = ON, 0 = OFF
	uint16_t laserTimer = 0;	// 
	uint8_t laserPower = 0;		// 0 = can't fire, 1 = can fire

	DCMotorSetup();
	Timer1Init();
	
	// Enable external pin change interrupt 1 (PCINT1) for all 6 RC channels
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11) | (1 << PCINT12) | (1 << PCINT13);
	sei();		// Set the Global Interrupt Enable flag so that interrupts can be processed
	
	// Main loop
	while(1) {		
		// Only update motor speeds after each full cycle of RC pulses
		if(start_flag == 1) {
			// Calculate velocity vector from channels 1 and 2
			// Channel 1 = vx, Channel 2 = vy
			ch1 = ch1 - 1500;	// Center at 0, in range [-500, +500]
			ch2 = ch2 - 1500;
			ch4 = ch4 - 1500;
		
			if(ch1 < -500) {
				ch1 = -500;
			} else if(ch1 > 500){
				ch1 = 500;
			}
			if(ch2 < -500) {
				ch2 = -500;
			} else if(ch2 > 500){
				ch2 = 500;
			}
			if(ch4 < -500) {
				ch4 = -500;
			} else if(ch4 > 500){
				ch4 = 500;
			}
		
			float sqrt3_2 = sqrt(3.0)/2.0;
		
			// Check for a dead stick position
			if((abs(ch1) > 50) || (abs(ch2) > 50) || (abs(ch4) > 50)) {
				// Calculate raw wheel speeds and directions for translation
				float rawSpeed_A = -ch1;
				float rawSpeed_B = 0.5*ch1 + sqrt3_2*ch2;
				float rawSpeed_C = 0.5*ch1 - sqrt3_2*ch2;
			
				// Add in rotation
				if(abs(ch4) > 25) {
					rawSpeed_A = rawSpeed_A - ch4;
					rawSpeed_B = rawSpeed_B - ch4;
					rawSpeed_C = rawSpeed_C - ch4;
				}
			
				// Set motor directions
				if(rawSpeed_A < 0) {
					MotorA_CCW();
				} else MotorA_CW();
		
				if(rawSpeed_B < 0) {
					MotorB_CCW();
				} else MotorB_CW();
		
				if(rawSpeed_C < 0) {
					MotorC_CCW();
				} else MotorC_CW();
			
				rawSpeed_A = fabs(rawSpeed_A);
				rawSpeed_B = fabs(rawSpeed_B);
				rawSpeed_C = fabs(rawSpeed_C);
			
				if(rawSpeed_A > 500) {
					rawSpeed_A = 500;
				}
				if(rawSpeed_B > 500) {
					rawSpeed_B = 500;
				}
				if(rawSpeed_C > 500) {
					rawSpeed_C = 500;
				}
		
				// Set motor speeds
				// Map from [0,500] to [155,255]
				SPEED_A = 155 + (int)(rawSpeed_A*100/500);
				SPEED_B = 155 + (int)(rawSpeed_B*100/500);
				SPEED_C = 155 + (int)(rawSpeed_C*100/500);
			
			} else {
				DCMotorStop();
			}
			
			// Turn laser on (Ch5 > 1500) or off (Ch5 < 1500)
			if((ch5 > 1500) && (laserState == 0) && (laserTimer == 0) && (laserPower == 1)) {
				PORTB |= (1 << PORTB1);
				laserState = 1;
				ledBarCounter = 1;	// Reset LED bar
				ledBarCycle = 1;
				ledBarIndicator = 1;
				laserTimer = LASERWAIT;
				laserPower = 0;
			} else if(ch5 < 1500) {
				PORTB &= ~(1 << PORTB1); // Turn off laser
				laserState = 0;
			}
			
			if(laserTimer > 0) {
				laserTimer--;
			} else {
				PORTB &= ~(1 << PORTB1); // Turn off laser
			}
			
			/*
			// Knight Rider LEDs
			if((ledCount % 5) == 0) {
				shiftByteOut(ledDisplay);
				if((ledPosition == 0) || (ledPosition == 7)) {
					ledIncrement = -ledIncrement;
				}
				ledPosition += ledIncrement;
				ledDisplay = (1 << ledPosition);
				ledCount = 1;
			} else {
				ledCount++;
			}*/
			
			// LED bar count up
			if((ledBarCounter % LEDBARWAIT == 0) && ledBarCycle < 8) {
				ledBarIndicator = (ledBarIndicator << 1) + 1;
				ledBarCounter++;
				ledBarCycle++;
			} else if (ledBarCycle == 8) {
				laserPower = 1;
			} else {
				ledBarCounter++;
			}
			
			// Display LED bar
			shiftByteOut(ledBarIndicator);
			
			// Check for laser hit
			// For Omnibot 1 HIT = 0, for Omnibot 2 HIT = 1 != 0,
			if((PINB & (1 << PORTB2)) != 0) {
				DCMotorStop();				// Turn off motors
				PORTB &= ~(1 << PORTB1);	// Turn off laser
				// Lock up and flash LEDs
				for(int i = 0; i < 10; i++) {
					shiftByteOut(0b11111111);
					_delay_ms(250);
					shiftByteOut(0b00000000);
					_delay_ms(250);
				}
				ledBarCounter = 1;	// Reset LED bar
				ledBarCycle = 1;
				ledBarIndicator = 1;
				laserPower = 0;		// Reset laser power
			}
			
			start_flag = 0;		// Reset flag after speeds have updated
		}
	}
}

void Timer1Init(void) {
	// Setup Timer/Counter1 in normal mode with prescaler of 8
	// At F_CPU = 16MHz, F_TIMER1 = 2MHz, and T_TIMER1 = 0.5 us
	TCCR1B |= (1 << CS11);
}

void shiftByteOut(uint8_t byteOut)
{
	// Clear shift register pins
	// DATA = PD0, LATCH = PD1, CLK = PD2
	PORTD &= ~((1 << PORTD0) | (1 << PORTD1) | (1 << PORTD2));
	
	// Shift out serial data
	for(int i = 0; i < 8; i++)
	{
		// Sets data high if bit i is 1
		if(byteOut & (1 << i))
		PORTD |= (1 << PORTD0);		// data high
		
		PORTD |= (1 << PORTD2);		// clock high
		PORTD &= ~(1 << PORTD2);	// clock low
		
		PORTD &= ~(1 << PORTD0);	// data low
	}
	
	// Display by latching to output pins
	PORTD |= (1 << PORTD1);			// latch high
	PORTD &= ~(1 << PORTD1);		// latch low
	return;
}

ISR (PCINT1_vect) {
	// Read six RC channels on PC0/PCINT8 (Ch1) to PC5/PCINT13 (Ch6)
	// Check to see if it is a rising edge for each channel to start timing
	// Falling edge to stop timing
	uint16_t currentTime = TCNT1 / 2;		// Read Timer/Counter1 in microseconds
	
	if(PINC & (1 << PORTC0)) {		// Channel 1
		if(ch1_state == 0) {
			TCNT1 = 0;			// Reset timer at start of ch1 only
			ch1_start = 0;
			ch1_state = 1;
			start_flag = 1;		// Set flag to update motor speeds & directions
		}
	} else if((PINC & (1 << PORTC0)) == 0) {
		if(ch1_state == 1) {
			ch1 = currentTime - ch1_start;
			ch1_state = 0;
		}
	}
	
	if(PINC & (1 << PORTC1)) {		//Channel 2
		if(ch2_state == 0) {
			ch2_start = currentTime;
			ch2_state = 1;
		}
	} else if((PINC & (1 << PORTC1)) == 0) {
		if(ch2_state == 1) {
			ch2 = currentTime - ch2_start;
			ch2_state = 0;
		}
	}
	
	if(PINC & (1 << PORTC2)) {		//Channel 3
		if(ch3_state == 0) {
			ch3_start = currentTime;
			ch3_state = 1;
		}
	} else if((PINC & (1 << PORTC2)) == 0) {
		if(ch3_state == 1) {
			ch3 = currentTime - ch3_start;
			ch3_state = 0;
		}
	}
	
	if(PINC & (1 << PORTC3)) {		//Channel 4
		if(ch4_state == 0) {
			ch4_start = currentTime;
			ch4_state = 1;
		}
	} else if((PINC & (1 << PORTC3)) == 0) {
		if(ch4_state == 1) {
			ch4 = currentTime - ch4_start;
			ch4_state = 0;
		}
	}
	
	if(PINC & (1 << PORTC4)) {		//Channel 5
		if(ch5_state == 0) {
			ch5_start = currentTime;
			ch5_state = 1;
		}
	} else if((PINC & (1 << PORTC4)) == 0) {
		if(ch5_state == 1) {
			ch5 = currentTime - ch5_start;
			ch5_state = 0;
		}
	}
	
	if(PINC & (1 << PORTC5)) {		//Channel 6
		if(ch6_state == 0) {
			ch6_start = currentTime;
			ch6_state = 1;
		}
	} else if((PINC & (1 << PORTC5)) == 0) {
		if(ch6_state == 1) {
			ch6 = currentTime - ch6_start;
			ch6_state = 0;
		}
	}
}
