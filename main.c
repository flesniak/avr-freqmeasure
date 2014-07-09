#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "hd44780.h"

#define MEASURE_COUNT 128

volatile uint16_t currentLength[MEASURE_COUNT];
volatile uint16_t lastCapture;
volatile uint16_t currentIndex;

ISR( TIMER1_CAPT_vect ) { //Timer1 capture register interrupt
  //TCNT1 = 0; //reset Timer1 (perhaps use delta calculation?)
  if( currentIndex < MEASURE_COUNT ) {
    currentLength[currentIndex] = ICR1-lastCapture;
    currentIndex++;
  }
  lastCapture = ICR1;
}

int main() {
  init();
  setMode2(true,false,false); //display on, cursor off, cursorFlash off
  setMode3(true,true,false); //8-bit mode, multi-line display, small font
  clearDisplay();
  setDDRAMAddress(4);
  const char str_freq[] = "Frequenz";
  uchar i;
  for( i = 0; i < 8; i++ ) {
    writeRAM(str_freq[i]);
  }
  setDDRAMAddress(ADR_LINE2+12);
  writeRAM('H');
  writeRAM('z');

  SFIOR = (1 << ACME); //enable AC multiplexer
  ADMUX = 0; //select ADC0 as negative AC input
  ADCSRA = 0; //disable ADC
  TCCR1B = (1 << CS11) | (1 << CS10); //enable Timer1 prescaler clk/64 = 125kHz
  TIMSK = (1 << TICIE1); //enable Timer1 capture interrupt
  ACSR = (1 << ACBG) | (1 << ACIC) | (1 << ACIS1) | (0 << ACIS0); //enable Analog Comparator falling-edge interrupt, bandgap select (1.3V on positive input)
  //ACBG Analog Comparator Bandgap Select internal reference on positive input (1.35V)
  //ACIC Analog Comparator Input Capture Enable capture Timer1 state
  //ACIS1..0 Comparator Interrupt on falling edge
  currentIndex = 0;
  sei();

  while( 1 ) {
    if( currentIndex == MEASURE_COUNT ) {
      //calculate frequency
      uint32_t total = 0;
      for( uchar i = 0; i < MEASURE_COUNT; i++ )
        total += currentLength[i];
      uint32_t temp = 125000*MEASURE_COUNT;
      uint32_t freq = 0;
      while( temp > total ) {
        temp -= total;
        freq++;
      }
      //freq *= 2;

      //output frequency
      setDDRAMAddress(ADR_LINE2+4);
      uchar digit = '0';
      while( freq >= 100000 ) {
        digit++;
        freq -= 100000;
      }
      writeRAM(digit);

      digit = '0';
      while( freq >= 10000 ) {
        digit++;
        freq -= 10000;
      }
      writeRAM(digit);

      digit = '0';
      while( freq >= 1000 ) {
        digit++;
        freq -= 1000;
      }
      writeRAM(digit);

      digit = '0';
      while( freq >= 100 ) {
        digit++;
        freq -= 100;
      }
      writeRAM(digit);

      digit = '0';
      while( freq >= 10 ) {
        digit++;
        freq -= 10;
      }
      writeRAM(digit);

      digit = '0' + (uchar)freq;
      writeRAM(digit);
      
      currentIndex = 0;
    }
    //_delay_ms(500);
    sleep_mode();
  }

  return 0;
}
