/*
 * File:   main.c
 * Author: Owen Holloway
 * University of Tasmania 2016
 *
 * Created on 20 September 2016, 2:55 PM
 */


#include <xc.h>
#include "definitions.h"
#include "functions.h"

char motorASpeed;
char motorBSpeed;

int Waiting_2_Start = 1;


void main(void) {
    SetUpPic();
    
    while (1) {    
        int *PWM;
        PWM = PID();
        if (Waiting_2_Start) {
            //SYS_Led = 1;
            PID_Led = 0;
            CCPR2L = 0;                    // Duty cycle value
            CCPR1L = 0;
        } else {
            //SYS_Led = 1;
            PID_Led = 1;
            //CCPR2L = 0;    //MOTORA
            //CCPR1L = 0;    //MOTORB
            CCPR2L = PWM[0];    //MOTORA
            CCPR1L = PWM[1];    //MOTORB
        }
        
        if (system_time >= 4000) {
            Waiting_2_Start = 1;
        }
        
    }
    return;
}

/*
 * Sets up the System time functions 
 */

static void interrupt ISR(void)
{
if (PIR1bits.TMR1IF)            // Timer 1
  {
   TMR1H=0xB2; 
   TMR1L=0x10;                  // Adjust timer  = 5mS
   system_time++;
   PIR1bits.TMR1IF = 0;         // clear flag
  }

if (INTCONbits.INTF)            // start signal received ( PortB.0)
  {
    system_time = 0;
    Waiting_2_Start = 0;      // clear start run flag
    INTCONbits.INTF=0;          // clear flag
  }
}