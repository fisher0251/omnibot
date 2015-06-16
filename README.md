# omnibot
Control program for a 3-wheeled omnibot platform with embedded ATMEGA328P

by D. Fisher, 06/2015

This is a project created with Atmel Studio 6.2 for the ATMEGA328P programmed via an AVR-ISP MKII. The ominbot platform is taken from <a href="http://makezine.com/projects/make-40/kiwi/">this article at Make Magazine</a>. Motors A, B, and C each have a direction control line and a speed control line. The speed is changed by changing the duty cycle of a PWM on the ENABLE pin of the SN754410. In addition to the motor control, my omnibot has a laser arm that can be turned on or off using an RC channel. 8 LEDs are opperated via a shift register to save microcontroller IO pins. Finally, the bot has a laser (or bright light) detector target. A laser hit to the photoresistor sends a logic LOW (0V) signal to the microcontroller.

<img src="https://electronicfish.files.wordpress.com/2015/06/omnibot-side.jpg" alt="Omnibot - Side" width="500">
