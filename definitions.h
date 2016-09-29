/* 
 * File:   definitions.h
 * Author: Owen Holloway
 *
 * Created on 20 September 2016, 3:01 PM
 */

#include <xc.h>

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

//Parameters

//Ramp up scale
#define     RAMP_UP         0.25

//PID coefficients
#define     K_P             40
#define     K_I             40
#define     K_A             40

#define     MOTORA_CALIB    1
#define     MOTORB_CALIB    1

#define     MAX_SPEED       100

/*                      PIC16F1936-I/P
                        --------------
                   MCLR - 1      28 - ICSPDAT
                AN0/RA0 - 2      27 - ICSPCLK
                AN1/RA1 - 3      26 - RB5/AN13/P2B(1)/P3A(1)
                AN2/RA2 - 4      25 - RB4/AN11/P1D
                AN3/RA3 - 5      24 - RB3/AN9/CCP2(1)/P2A(1)
                    RA4 - 6      23 - RB2/AN8/P1B
                AN4/RA5 - 7      22 - RB1/AN10/P1C
                    VSS - 8      21 - RB0/INT
               OSC1/RA7 - 9      20 - VDD
               OSC2/RA6 - 10     19 - VSS
             P2B(1)/RC0 - 11     18 - RC7/RX
            CCP2(1)/RC1 - 12     17 - RC6/TX
               CCP1/RC2 - 13     16 - RC5/SDO
                    RC3 - 14     15 - RC4/SDI/SDA
 * ports used
 * RC0 = output             System LED
 * RB0 = INT                Start Signal
 * RC5 = output             PID LED
 * 
 * RC1 = CCP2/P2A           PWM motor
 * RC2 = CCP1/P1A           PWM motor
 * 
 * RC6 = TX                 RS232  (if implemented)
 * RC7 = RX                 RS232  (if implemented)
 * 
 * RB1                      Left scanner digital input
 * RB2                      Middle scanner digital input
 * RB3                      Right scanner digital input
*/

#define XTAL 20000000   // Oscilator Frequency


// I/O definitions
#define     SYS_Led         LATCbits.LATC0           //System status LED
#define     PID_Led         LATCbits.LATC5           //PID active LED

#define     L_LINE_SCAN_PIN PORTBbits.RB1
#define     M_LINE_SCAN_PIN PORTBbits.RB2
#define     R_LINE_SCAN_PIN PORTBbits.RB3

#endif	/* DEFINITIONS_H */

