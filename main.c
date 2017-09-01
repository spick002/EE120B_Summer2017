/* Sean Pickman spick002@ucr.edu
 * Created: 8/30/2017
Custom_Project_Jukebox_Hero
 */ 
#include <avr/io.h>
#include "io.c"
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


unsigned char ToDisplay;
unsigned char cnt=0;
unsigned char beat=0;
unsigned char song_finished=1;
enum LCD_States {LCD_Init, LCD_Init2, LCD_Idle, LCD_Up, LCD_Down, LCD_BRelease, ReleasetoPlay, LCD_Song1, LCD_Song2, LCD_Song3, LCD_SoundA3,LCD_SoundB3,LCD_SoundC4, LCD_SoundD4, LCD_SoundE4, LCD_SoundF4, LCD_SoundG4, LCD_SoundA4, LCD_SoundB4, LCD_SoundC5, LCD_SoundC5s, LCD_SoundD5, LCD_SoundE5, LCD_SoundF5, LCD_SoundF5s, LCD_SoundG5, LCD_SoundA5} LCD_State;

void LCD_Increment(){
	const unsigned char* Welcome_Screen="  **WELCOME**   Select Song <>^";
	const unsigned char* First_Song_Name="Happy Birthday";
	const unsigned char* Second_Song_Name="I Love You";
	const unsigned char* Third_Song_Name="Yankee Doodle";
	const unsigned char* First_Song_Play="Happy Birthday  (Playing)";
	const unsigned char* First_Song_Stopped="Happy Birthday  (Stopped)";
	const unsigned char* First_Song_Fast="Happy Birthday  (Fast)";
	const unsigned char* First_Song_Slow="Happy Birthday  (Slow)";
	const unsigned char* Second_Song_Play="I Love You      (Playing)";
	const unsigned char* Second_Song_Stopped="I Love You      (Stopped)";
	const unsigned char* Second_Song_Fast="I Love You      (Fast)";
	const unsigned char* Second_Song_Slow="I Love You      (Slow)";
	const unsigned char* Third_Song_Play="Yankee Doodle   (Playing)";
	const unsigned char* Third_Song_Stopped="Yankee Doodle   (Stopped)";
	const unsigned char* Third_Song_Fast="Yankee Doodle    (Fast)";
	const unsigned char* Third_Song_Slow="Yankee Doodle    (Slow)";
	switch (LCD_State){//Transitions
		case LCD_Init:
			LCD_State = LCD_Init2;
			break;
		case LCD_Init2:
			LCD_State = LCD_Idle;
			break;
		case LCD_Idle:

			cnt=0;
			if(PINA==0xFE)
				LCD_State=LCD_Down;
			else if(PINA==0xFD)
				LCD_State=LCD_Up;
			else if(PINA==0xFB)
				LCD_State=ReleasetoPlay;
			else
				LCD_State=LCD_Idle;
			break;
		case LCD_Up:
			LCD_State=LCD_BRelease;
			break;
		case LCD_Down:
			LCD_State=LCD_BRelease;
			break;
		default:
			LCD_State=LCD_Init;
			break;
		case LCD_BRelease:
			if(PINA==0xFF){
				if(!song_finished){
					ToDisplay=0;
					TimerSet(150);
					LCD_State=LCD_Init2;
					song_finished=1;}
				else
					LCD_State=LCD_Idle;}
			else
				LCD_State=LCD_BRelease;
			break;
		case ReleasetoPlay:
			if(PINA==0xFF){
				if(ToDisplay==0)
					LCD_State=LCD_Idle;
				else if(ToDisplay==1){
					LCD_DisplayString(1, First_Song_Play);
					LCD_State=LCD_Song1;}
				else if(ToDisplay==2){
					LCD_DisplayString(1, Second_Song_Play);
					LCD_State=LCD_Song2;}
				else if(ToDisplay==3){
					LCD_DisplayString(1, Third_Song_Play);
					LCD_State=LCD_Song3;}}
			else
				LCD_State=ReleasetoPlay;
			break;
		case LCD_SoundA3:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundA3;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundB3:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundB3;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundC4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundC4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundD4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundD4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundE4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundE4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundF4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundF4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundG4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundG4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundA4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundA4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundB4:
			if(beat==2){
				beat=1;
				LCD_State=LCD_SoundB4;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundC5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundC5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
		break;
		case LCD_SoundC5s:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundC5s;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundD5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundD5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
		break;
		case LCD_SoundE5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundE5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundF5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundF5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
			break;
		case LCD_SoundF5s:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundF5s;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
		break;
		case LCD_SoundG5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundG5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
		break;
		case LCD_SoundA5:
			if(beat==1){
				beat=2;
				LCD_State=LCD_SoundA5;
			}
			else if(ToDisplay==1)
				LCD_State=LCD_Song1;
			else if(ToDisplay==2)
				LCD_State=LCD_Song2;
			else if(ToDisplay==3)
				LCD_State=LCD_Song3;
		break;
		case LCD_Song1:						//beat=2, will be quicker beat, beat=1 will be slower beat.
			song_finished=0;
			cnt++;
			if(PINA==0xFB){
				LCD_DisplayString(1, First_Song_Stopped);
				LCD_State=LCD_BRelease;}
			else if(PINA==0xFD){
				LCD_DisplayString(1, First_Song_Fast);
				TimerSet(75);
				LCD_State=ReleasetoPlay;}
			else if(PINA==0xFE){
				LCD_DisplayString(1, First_Song_Slow);
				TimerSet(300);
				LCD_State=ReleasetoPlay;}
			else if(cnt<=2){
				beat=2;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=3){
				beat=1;
				LCD_State=LCD_SoundB4;}
			else if(cnt<=4){
				beat=1;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=5){
				beat=1;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=6){
				beat=1;
				LCD_State=LCD_SoundC5s;}
			//2nd verse
			else if(cnt<=8){
				beat=2;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=9){
				beat=1;
				LCD_State=LCD_SoundB4;}
			else if(cnt<=10){
				beat=1;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=11){
				beat=1;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=12){
				beat=1;
				LCD_State=LCD_SoundD5;}
			//3rd verse
			else if(cnt<=14){
				beat=2;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=15){
				beat=1;
				LCD_State=LCD_SoundA5;}
			else if(cnt<=16){
				beat=1;
				LCD_State=LCD_SoundF5s;}
			else if(cnt<=17){
				beat=1;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=18){
				beat=1;
				LCD_State=LCD_SoundC5s;}
			else if(cnt<=19){
				beat=1;
				LCD_State=LCD_SoundB4;}
			//4th verse
			else if(cnt<=21){
				beat=2;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=22){
				beat=1;
				LCD_State=LCD_SoundF5s;}
			else if(cnt<=23){
				beat=1;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=24){
				beat=1;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=25){
				beat=1;
				LCD_State=LCD_SoundD5;}
			else{
				song_finished=1;
				TimerSet(150);
				LCD_DisplayString(1, Welcome_Screen);
				ToDisplay=0;
				LCD_State=LCD_Idle;}
		break;
		case LCD_Song2:			//beat=2, will be quicker beat, beat=1 will be slower beat.
			song_finished=0;
			TimerSet(250);
			cnt++;
			if(PINA==0xFB){
				LCD_DisplayString(1, Second_Song_Stopped);
				LCD_State=LCD_BRelease;}
			else if(PINA==0xFD){
				LCD_DisplayString(1, Second_Song_Fast);
				TimerSet(125);
				LCD_State=ReleasetoPlay;}
			else if(PINA==0xFE){
				LCD_DisplayString(1, Second_Song_Slow);
				TimerSet(500);
				LCD_State=ReleasetoPlay;}
			else if(cnt<=1){
				beat=2;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=2){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=3){
				beat=1;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=4){
				beat=2;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=5){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=6){
				beat=1;
				LCD_State=LCD_SoundG5;}
			//2nd verse
			else if(cnt<=7){
				beat=1;
				LCD_State=LCD_SoundA5;}
			else if(cnt<=8){
				beat=2;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=9){
				beat=2;
				LCD_State=LCD_SoundF5;}
			else if(cnt<=10){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=11){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=12){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=13){
				beat=1;
				LCD_State=LCD_SoundF5;}
			//3rd verse
			else if(cnt<=14){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=15){
				beat=2;
				LCD_State=LCD_SoundF5;}
			else if(cnt<=16){
				beat=1;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=18){
				beat=1;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=21){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=22){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=23){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=24){
				beat=2;
				LCD_State=LCD_SoundF5;}
			else if(cnt<=25){
				beat=1;
				LCD_State=LCD_SoundG5;}
			//4th verse
			else if(cnt<=26){
				beat=2;
				LCD_State=LCD_SoundG5;}
			else if(cnt<=28){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=29){
				beat=2;
				LCD_State=LCD_SoundF5;}
			else if(cnt<=30){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=31){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=32){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else{
				song_finished=1;
				TimerSet(150);
				LCD_DisplayString(1, Welcome_Screen);
				ToDisplay=0;
				LCD_State=LCD_Idle;}
			break;
			case LCD_Song3:			//beat=2, will be quicker beat, beat=1 will be slower beat.
				song_finished=0;
				TimerSet(150);
				cnt++;
			if(PINA==0xFB){
				LCD_DisplayString(1, Third_Song_Stopped);
				LCD_State=LCD_BRelease;}
			else if(PINA==0xFD){
				LCD_DisplayString(1, Third_Song_Fast);
				TimerSet(75);
				LCD_State=ReleasetoPlay;}
			else if(PINA==0xFE){
				LCD_DisplayString(1, Third_Song_Slow);
				TimerSet(500);
				LCD_State=ReleasetoPlay;}
			else if(cnt<=2){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=3){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=4){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=5){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=6){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=7){
				beat=2;
				LCD_State=LCD_SoundD5;}
			//2nd verse
			else if(cnt<=8){
				beat=2;
				LCD_State=LCD_SoundG4;}
			else if(cnt<=10){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=11){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=12){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=13){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=14){
				beat=2;
				LCD_State=LCD_SoundB4;}
			//3rd verse
			else if(cnt<=15){
				beat=2;
				LCD_State=LCD_SoundG4;}
			else if(cnt<=16){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=17){
				beat=2;
				LCD_State=LCD_SoundD5;}
			else if(cnt<=18){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=19){
				beat=2;
				LCD_State=LCD_SoundF5;}
			else if(cnt<=20){
				beat=2;
				LCD_State=LCD_SoundE5;}
			else if(cnt<=21){
				beat=2;
				LCD_State=LCD_SoundD5;}
			//4th verse
			else if(cnt<=22){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else if(cnt<=23){
				beat=2;
				LCD_State=LCD_SoundB4;}
			else if(cnt<=24){
				beat=2;
				LCD_State=LCD_SoundG4;}
			else if(cnt<=25){
				beat=2;
				LCD_State=LCD_SoundA4;}
			else if(cnt<=26){
				beat=2;
				LCD_State=LCD_SoundB4;}
			else if(cnt<=27){
				beat=2;
				LCD_State=LCD_SoundC5;}
			else{
				song_finished=1;
				TimerSet(150);
				LCD_DisplayString(1, Welcome_Screen);
				ToDisplay=0;
				LCD_State=LCD_Idle;}
			break;
	}
	switch(LCD_State){//State actions
		case LCD_Init:
												PORTB=0x01;
			LCD_DisplayString(1, Welcome_Screen);
			break;
		case LCD_Init2:
												PORTB=0x02;
			LCD_DisplayString(1, Welcome_Screen);
			break;
		case LCD_Idle:
												PORTB=0x03;
			break;
		case LCD_Up:
												PORTB=0x04;
			if(ToDisplay<3){
				ToDisplay+=1;
				LCD_Cursor(1);
				if(ToDisplay==1)
					LCD_DisplayString(1, First_Song_Name);
				if(ToDisplay==2)
					LCD_DisplayString(1, Second_Song_Name);
				if(ToDisplay==3)
					LCD_DisplayString(1, Third_Song_Name);
			}
			break;
		case LCD_Down:
												PORTB=0x05;
			if(ToDisplay>1){
				ToDisplay-=1;
				LCD_Cursor(1);
				if(ToDisplay==1)
					LCD_DisplayString(1, First_Song_Name);
				if(ToDisplay==2)
					LCD_DisplayString(1, Second_Song_Name);
				if(ToDisplay==3)
					LCD_DisplayString(1, Third_Song_Name);
			}
			break;
		case LCD_BRelease:
												PORTB=0x06;
			break;
		case LCD_Song1:
			set_PWM(0);							PORTB=0x07;
			break;
		case LCD_Song2:
			set_PWM(0);							PORTB=0x08;
			break;
		case LCD_Song3:
			set_PWM(0);							PORTB=0x09;
			break;
		case LCD_SoundA3:
											PORTB=0x0A;
			set_PWM(220.00);
			break;
		case LCD_SoundB3:
											PORTB=0x0B;
			set_PWM(246.94);
			break;
		case LCD_SoundC4:
											PORTB=0x0C;
			set_PWM(261.626);
			break;
		case LCD_SoundD4:
											PORTB=0x0D;
			set_PWM(293.66);
			break;
		case LCD_SoundE4:
											PORTB=0x0E;
			set_PWM(329.63);
			break;
		case LCD_SoundF4:
											PORTB=0x0F;
			set_PWM(349.23);
			break;
		case LCD_SoundG4:
											PORTB=0x10;
			set_PWM(392.00);
			break;
		case LCD_SoundA4:
											PORTB=0x11;
			set_PWM(440.00);
			break;
		case LCD_SoundB4:
											PORTB=0x012;
			set_PWM(493.88);
			break;
		case LCD_SoundC5:
											PORTB=0x13;
			set_PWM(523.25);
			break;
		case LCD_SoundC5s:
											PORTB=0x14;
			set_PWM(554.37);
			break;
		case LCD_SoundD5:
										PORTB=0x15;
			set_PWM(587.33);
			break;
		case LCD_SoundE5:
										PORTB=0x16;
			set_PWM(659.26);
			break;
		case LCD_SoundF5:
										PORTB=0x17;
			set_PWM(698.46);
			break;
		case LCD_SoundF5s:
										PORTB=0x18;
			set_PWM(739.98);
			break;
		case LCD_SoundG5:
										PORTB=0x19;
			set_PWM(783.99);
			break;
		case LCD_SoundA5:
										PORTB=0x1A;
			set_PWM(880.00);
			break;
		default:
												PORTB=0xFF;
			break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	PWM_on();
	TimerSet(150);
	TimerOn();
	
	ToDisplay=0;
	// Initializes the LCD display
	LCD_init();
	//LCD_DisplayString(1, );
	
	while(1) {
		LCD_Increment();
		while (!TimerFlag);	// Wait 1 sec
		TimerFlag = 0;
	}
	return 0;
}
