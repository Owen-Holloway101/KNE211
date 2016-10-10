#include "functions.h"
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>

int error_sum = 0;

int last_error = 0;

int diff_time = 0;

/*
 * Sets up pic outputs and inputs 
 */

void SetUpPic(void)
{
    OSCCONbits.IRCF = 0b1111;     // set for internal clock use
    ANSELA=0;                     // digital
    ANSELB=0;

    TRISB = 0b00111111;           // set inputs  (RB6-7 reserved for ISP)
    TRISC = 0b10000000;           // set outputs ( RC7 set for RS232 RX input)

    TRISBbits.TRISB1 =1;          // scan input L
    TRISBbits.TRISB2 =1;          // scan input C
    TRISBbits.TRISB3 =1;          // scan input R
    
    TRISCbits.TRISC0 =0;          // led output
    TRISCbits.TRISC5 =0;          // led output
    
    // Registers for Timer2/PWM mode
    PR2=0xFF;                      // Set PWM Frequency
    
    CCP1CONbits.CCP1M=0b1100;      // Turn on Pwm1
    CCP2CONbits.CCP2M=0b1100;      // Turn on Pwm2
    
    CCP1CONbits.P1M = 0b00;        // single output
    CCP2CONbits.P2M = 0b00;
    
    CCPR1L = 0;                    // Duty cycle value
    CCPR2L = 0;                    // Duty cycle value
    T2CONbits.TMR2ON = 1;          // turn on timer 2

    // Registers for Timer1/ timer mode used as System clock
    T1CONbits.TMR1CS = 0;                  // internal clock
    T1CONbits.T1CKPS = 0b00;               // no prescale used

    TMR1H=0xB2; TMR1L=0x10;                // Adjust timer    = 5mS
    T1CONbits.TMR1ON =1;                   // turn on

    // SET UP INTERUPTS
    PIE1bits.TMR1IE =1;
    PIR1bits.TMR1IF=0;  // enable and clear Timer 1

    INTEDG = 0;
    
    INTCONbits.INTE = 1;
    INTCONbits.INTF = 0;                   // enable INT0 - clear flag if set

    INTCONbits.PEIE =1;                    // Enable Peripheral interrupt
    INTCONbits.GIE = 1;                    // Enable Global interrupt
}


//Starting max_speed
int max_speed = 0;

/*
 * PID pwm control 
 */

int * PID() {
    
    int current_error = Scan_Inputs();
    
    error_sum = error_sum + current_error; //get the error sum added
    
    if (current_error != last_error) {
        diff_time = system_time - 1;
        last_error =  current_error;
    }
    
    static int PIDPWM[2];
    
    //Starting ramp up for starting
    if (system_time < (MAX_SPEED/RAMP_UP)) {
        max_speed = (int) ((double) (RAMP_UP * system_time));
    } else { 
        max_speed = MAX_SPEED;
    }
    
    //Coefficients
    double Kp = K_P;
    double Ki = K_I;
    double Ka = K_A;
    
    
    double delta_error_delta_time = current_error/(system_time-diff_time);
    
    double deltaPWM = Kp*abs(current_error)*(max_speed/MAX_SPEED) + Ki*(error_sum/system_time) + Ka*delta_error_delta_time;
    
    int deltaPWM_cast = (int) ((double)deltaPWM*(max_speed/MAX_SPEED));
    
    
    if (current_error == 4) {
        PIDPWM[0] = 0;
        PIDPWM[1] = 0;
    } else if (current_error < 0) {
        SYS_Led = 0;
        PIDPWM[0] = MOTORA_CALIB * max_speed - 20;
        PIDPWM[1] = MOTORB_CALIB * (max_speed - deltaPWM_cast) - 20;
    } else if (current_error > 0) {
        SYS_Led = 1;
        PIDPWM[0] = MOTORA_CALIB * (max_speed - deltaPWM_cast) - 20;
        PIDPWM[1] = MOTORB_CALIB * max_speed - 20;
    } else if (current_error == 0) {
        PIDPWM[0] = MOTORA_CALIB*max_speed;
        PIDPWM[1] = MOTORB_CALIB*max_speed;
    } else {
        return 0;
    }
    
    
    if (PIDPWM[0] > max_speed) {
        PIDPWM[0] = max_speed;
    } else if (PIDPWM[0] < 0) {
        PIDPWM[0] = 0;
    }
    
    if (PIDPWM[1] > max_speed) {
        PIDPWM[1] = max_speed;
    } else if (PIDPWM[1] < 0) {
        PIDPWM[1] = 0;
    }
    
    
    //PIDPWM[0] = MOTORA_CALIB*max_speed;
    //PIDPWM[1] = MOTORB_CALIB*max_speed;
    
    return PIDPWM;
    
}

int Scan_Inputs()               // scan opto inputs
{
   int New_Direct;       // result of last scan
   int Scan_Value = 0;
   
   int L_Line_Scan;  
   int M_Line_Scan;  
   int R_Line_Scan;
   
   int Scan_Result[8];                // conversion table
   Scan_Result[0] = 3;                 // all optos off , no line detected
   Scan_Result[1] = 2;                 // extreme heading to right
   Scan_Result[2] = 0;                 // in middle of line
   Scan_Result[3] = 1;                 // heading to right
   Scan_Result[4] = 2;                 // extreme to left
   Scan_Result[5] = 4;                 // invalid entry
   Scan_Result[6] = 1;                 // heading to left
   Scan_Result[7] = 4;                 // end of run or lost control

    //   left   centre  right
    //     0       0      0  =   0 ->   3      depends on last_Direction
    //     0       0      1  =   1 ->   2      set Direct = -1
    //     0       1      1  =   3 ->   1      set Direct = -1
    //     0       1      0  =   2 ->   0      set Direct =  0    on line
    //     1       1      0  =   6 ->   1      set Direct = +1
    //     1       0      0  =   4 ->   2      set Direct = +1
    //     1       1      1  =   7 ->   4      end of run or lost control
    // invalid scans =  5
    L_Line_Scan = L_LINE_SCAN_PIN;      // scan inputs
    M_Line_Scan = M_LINE_SCAN_PIN;
    R_Line_Scan = R_LINE_SCAN_PIN;
  
    if (!L_Line_Scan) { Scan_Value = Scan_Value + 4;     New_Direct = -1;}
    if (!M_Line_Scan) { Scan_Value = Scan_Value + 2;                     }
    if (!R_Line_Scan) { Scan_Value = Scan_Value + 1;     New_Direct =  1;}
    Scan_Value = Scan_Result[Scan_Value];   // convert scan via table
  
    switch (Scan_Value)
     {
      case 0:        // on line
          Scan_Value = 0;
         break;
     case 1:                                        // just off the line
         Scan_Value = Scan_Value * New_Direct;      // from last scan
         Last_Direct =  New_Direct;                 // save for  next scan
         break;
     case 2:                                        // need large adjustment <-
         Scan_Value = Scan_Value * New_Direct;      // from last scan
         Last_Direct =  New_Direct;                 // save for  next scan
         break;
     case 3:                                        // no line detected
         Scan_Value = Scan_Value * Last_Direct;     // from last scan
         break;
     case 4:                                        // end of run or lost it
         Scan_Value = 4;                            // turn off PWM
         break;
     }
 return Scan_Value;                                // return new scan result
}