/*
 * DCMotor.c
 *
 * Created: 1/28/2015 9:53:38 PM
 *  Author: Dan
 */ 

#include <avr/io.h>
#include "DCMotor.h"

void DCMotorSetup(void) {
	// Setup PWM for Timer/Counter0 A & B with prescaler 8 and fast PWM (mode 3)
	// f_PWM = F_CPU/(8*256) = 7812.5 Hz, non-inverting mode
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << CS01);
	
	// Setup PWM for Timer/Counter2 B with prescaler 8 and fast PWM (mode 3)
	// f_PWM = F_CPU/(8*256) = 7812.5 Hz, non-inverting mode
	TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B |= (1 << CS21);
	
	// Enable output for PWM and motor direction pins
	DDRD |= (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);
	DDRB |= (1 << PORTB0);
	
	DCMotorStop();
}

void DCMotorStop(void){
	SPEED_A = 0;
	SPEED_B = 0;
	SPEED_C = 0;
}