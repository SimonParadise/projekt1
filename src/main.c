/* Defines -----------------------------------------------------------*/
// #define SW PD2
// #define Xin PC0
// #define Yin PC1
#define EDT PB4
#define ECLK PB5
// #ifndef F_CPU
// # define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
// #endif

/*
https://docs.arduino.cc/static/6ec5e4c2a6c0e9e46389d4f6dc924073/2f891/Pinout-UNOrev3_latest.png
https://www.microchip.com/en-us/product/ATmega328p
https://components101.com/sites/default/files/component_datasheet/Joystick%20Module.pdf
https://github.com/tomas-fryza/digital-electronics-2
hlava ŠIMONA BUCHTY a  ŠTĚPÁNA VEČEŘI 
ŠIMON BUCHTA MAESTRO ZAPOJENÍ
*/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <gpio.h>           // gpio
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <lcd.h>            // Peter Fleury's LCD library


// #define PC0 14
// #define PC1 15
// #define PD2 2
#define PB4 12
#define PB5 13


static uint8_t axis = 0;      //axis determines if we measure X or Y value of 0 means x, 1 means y
static uint8_t posx = 0;
static uint8_t posy = 0;
int Val;
static uint8_t lastA;
static uint8_t currA;      //encoder CLK 
static uint8_t B;     //encoder DT
static int8_t counter = 0; 

int main(void)
{
    // Initialize I2C (TWI)
    //twi_init();

    // Initialize USART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    ADMUX |= (1<<REFS0);
    ADMUX &= ~(1<<REFS1);

    // Select input channel ADC0 (voltage divider pin)
    ADMUX &= ~((1<<MUX0) | (1<<MUX1) | (1<<MUX2) | (1<<MUX3));
    // Select input channel ADC1
    //ADMUX &= ~ ((1<<MUX1) | (1<<MUX2) | (1<<MUX3)); ADMUX |= (1<<MUX0);
    // Enable ADC module
    ADCSRA |= (1<<ADEN);
    // Enable conversion complete interrupt
    ADCSRA |= (1<<ADIE);
    // Set clock prescaler to 128
    ADCSRA |= ((1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2));

    // Configure 16-bit Timer/Counter1 to test one I2C address
    // Set prescaler to 33 ms and enable interrupt
    TIM1_overflow_33ms();
    TIM1_overflow_interrupt_enable();
    TIM2_overflow_16us(); 
    TIM2_overflow_interrupt_enable();
    // Enables interrupts by setting the global interrupt mask
    lcd_init(LCD_DISP_ON_CURSOR_BLINK);
    lcd_gotoxy(0, 0);                 //starting position is left up

    GPIO_mode_input_nopull(&DDRB, EDT);
    GPIO_mode_input_nopull(&DDRB, ECLK);

    lastA = GPIO_read(&PINB, ECLK);

    sei();

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}


ISR(TIMER1_OVF_vect)
{ 
  static uint8_t no_of_overflows = 0;
  no_of_overflows ++;
  if (no_of_overflows >= 5)
  {
    no_of_overflows =0;
   // ADCSRA |= (1<<ADSC);

    if (axis == 0)
    {
      ADMUX |= (1<<MUX0);
      axis = 1;
    }
    else if (axis == 1)
    {
       ADMUX &= ~(1<<MUX0); 
       axis = 0;
    }
    ADCSRA |= (1<<ADSC);
  }
    //lcd_gotoxy(posx, posy);
    // Start ADC conversion
}

ISR(ADC_vect)
{
 //char string[3];

 Val = ADC;

if (axis == 0)
    {
      if (Val <10){                 //go right
        posx = posx + 1;            
        if (posx == 16){            //there is only 15 positions on the display
          posx = 0;
        }
      }
     else if (Val > 900){            //go left
        if (posx  == 0){           
          posx = 15;
        }
        else {
          posx = posx - 1;
        }
        }
    }

if (axis == 1)
    {
      if (Val <10){             //go down
        if (posy == 0){
          posy = 1;
        }
        else {
          posy = 0;
        }
        
      }
     else if (Val > 900){       //go up
        if (posy == 0){
          posy = 1;
        }
        else {
          posy = 0;
        }
      }
      }
lcd_gotoxy(posx, posy);
} 

ISR(TIMER2_OVF_vect){
  char string[8];
  currA = GPIO_read(&PINB, ECLK);
  B = GPIO_read(&PINB, EDT);

  if (currA != lastA  && currA == 1){
    if (B != currA){
      counter --;
    }
    else {
      counter ++;
    }
  }
  lastA = currA;
  itoa(counter, string, 10);
  uart_puts(string);

  // itoa(A, string, 10);
  // uart_puts(string);
  // itoa(B, string, 10);
  // uart_puts(string);

}
