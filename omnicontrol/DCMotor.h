/*
 * DCMotor.h
 *
 * Created: 1/28/2015 9:54:27 PM
 *  Author: Dan
 */ 

// 8-bit speed from 0 to 255
#define SPEED_A OCR2B
#define SPEED_B OCR0B
#define SPEED_C OCR0A

// MotorA = PD4, MotorB = PB0, MotorC = PD7
#define MotorA_CW()		(PORTD |= (1 << PORTD4))
#define MotorA_CCW()	(PORTD &= ~(1 << PORTD4))
#define MotorB_CW()		(PORTB |= (1 << PORTB0))
#define MotorB_CCW()	(PORTB &= ~(1 << PORTB0))
#define MotorC_CW()		(PORTD |= (1 << PORTD7))
#define MotorC_CCW()	(PORTD &= ~(1 << PORTD7))

// Setup PWM on Timer/Counter0 and Timer/Counter2
// Setup output control pins for Motors A, B, and C
void DCMotorSetup(void);

// Stops all 3 motors by setting their speeds to 0
void DCMotorStop(void);