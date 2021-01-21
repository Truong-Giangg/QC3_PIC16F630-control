#include <xc.h>     
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA3/MCLR pin function select (RA3/MCLR pin function is MCLR)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
unsigned char eepdata;      
unsigned char eepselect;    //select between normal 5volt and INOV 5volt
unsigned char eep1;
int changed = 0;
int  dem1 = 0;
char dem2 = 0x00;
int mydelay(int t){
    OPTION_REGbits.T0CS = 0;    //set internal clock timer0
    OPTION_REGbits.PSA = 0;     //Prescaler is assigned to the Timer0
    OPTION_REGbits.PS2 = 1;     //Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 0;     //Prescaler Rate Select bits
    OPTION_REGbits.PS0 = 0;     //Prescaler Rate Select bits /32
    TMR0 = t;
    for(int i=1;i<=200;i++){
        while(T0IF == 0){       //Wait for flag to over flow
            
        }
        T0IF = 0;               //Clear flag
        TMR0 = t;
    }
    return 0;
}
int eepromwrite(int add, int dat){
    EEADR = add;            //write address
    EEDATA = dat;           //write data
    EECON1bits.WREN = 1;    //ENABLE WRITE
    EECON2 = 0x55;          //UNLOCK WRITE
    EECON2 = 0xAA;          //UNLOCK WRITE
    EECON1bits.WR = 1;      //START WRITE
    return 0;
}
int eepromread(int add){
    EEADR = add;            //read address
    EECON1bits.RD = 1;      //enable read
    int dat = EEDATA;       // store data from EEPROM to eepromread
    return dat;
}
void volt12(){
    //TRISCbits.TRISC4 = 0x01;    // d+ =0,6V
    //TRISCbits.TRISC3 = 0x01;    // d- =0,6V
    TRISC = 0b00011000;
    PORTC = 0b00000100;
    eepromwrite(0x00,0x60);
}
void volt9(){
    //TRISCbits.TRISC4 = 0x00;    // 
    //PORTCbits.RC4 = 1;          // d+ =3,3V
    //TRISCbits.TRISC3 = 0x01;    // d- =0,6V
    TRISC = 0b00001000;
    PORTC = 0b00010010;
    eepromwrite(0x00,0x45);
}
void volt5(){
    //TRISCbits.TRISC4 = 0x01;    // d+ = 0,6V
    //TRISCbits.TRISC3 = 0x00;
    //PORTCbits.RC3 = 0;          // d- = 0V
    TRISC = 0b00010000;
    PORTC = 0b00000001;
    PORTAbits.RA1 = 0;
    dem2 = 0x00;        //reset dem2 
    eepromwrite(0x00,0x25);
    mydelay(253);   //delay20ms
    eepromwrite(0x01,0x00);
}
void inc200mv(){        //3.3v to 0.6v -> increase 200mv 
    TRISCbits.TRISC4 = 0x00;
    PORTCbits.RC4 = 1;      // d+ = 3V3
    mydelay(253);
    TRISCbits.TRISC4 = 0x01;// d+ = 0,6V
    mydelay(253);
}
void dec200mv(){        //0.6v to 3.3v -> decrease 200mv 
    TRISCbits.TRISC3 = 0x01;// d- =0,6V
    mydelay(253);
    TRISCbits.TRISC3 = 0x00;
    PORTCbits.RC3 = 1;      // d- = 3V3
    mydelay(253);     
}
void INOV(){
    //TRISCbits.TRISC3 = 0x00;
    //PORTCbits.RC3 = 1;          // d- = 3V3
    //TRISCbits.TRISC4 = 0x01;    // d+ = 0,6V
    TRISC = 0b00010000;
    PORTC = 0b00001000;
    PORTAbits.RA1 = 1;
    if(changed == 1){
        //select INOV mode--------------------------------
        eepromwrite(0x01,0x01);
        mydelay(253);   //delay20ms     //delay between write and read
        eep1 = eepromread(0x00);        
        if(eep1 > 0x25){    //if we increase voltage
            for(int i = 0x00; i<= eep1-0x25 ;i++){
                inc200mv();
                mydelay(253);   //delay 20ms
            }
            dem2 = eep1 -0x25;
        }
        if(eep1 < 0x25){    //if we decrease voltage
            for(int i = 0x00; i <= 0x25 - eep1; i++){
                dec200mv();
                mydelay(253);   //delay 20ms
            }
            dem2 = 0x25 - eep1;
        }
        mydelay(178);   //delay 0.5s button delay
        while(PORTAbits.RA2 == 1){      // out loop if RA2 = 0 (press the button)
            if(PORTAbits.RA0 == 0){     // increase 0,2V if we press button 2
                inc200mv();
                dem2 ++;                       
                char t = 0x25 + dem2;               
                eepromwrite(0x00,t);
                mydelay(178);   //delay 0.5s         //delay for button
            }
            if(PORTAbits.RA4 == 0){     // decrease 0,2V if we press button 3
                dec200mv();
                dem2 --;                     
                char t1 = 0x25 + dem2;
                eepromwrite(0x00,t1);
                mydelay(178);   //delay 0.5s         //delay for button
            }           
        }
        volt5();       
    }    
    if(changed == 0){           //the first time controller run
        eepromwrite(0x01,0x01);     //write 0x01 at 0x01
        mydelay(253);   //delay20ms
        eepromwrite(0x00,0x25);
        mydelay(178);   //delay 0.5s button delay
        while(PORTAbits.RA2 == 1){          // out loop if RA2 = 0 (press mode button)
            if(PORTAbits.RA0 == 0){         // increase 0,2V if we press button 2
                inc200mv();
                dem2 ++;                           
                char t = 0x25 + dem2;                
                eepromwrite(0x00,t);
                mydelay(178);   //delay 0.5s         //button delay
            }            
            if(PORTAbits.RA4 == 0){         // decrease 0,2V if we press button 3
                dec200mv();
                dem2 --;                            
                char t1 = 0x25 + dem2;
                eepromwrite(0x00,t1);
                mydelay(178);   //delay 0.5s         //button delay
            }       
        }
        volt5();
    }        
}
void main(void) {    
    TRISC = 0b00011000;     //led 1, 2, 3 output; d+, d- input 
    PORTC = 0b00000000;     //turn off all led
    CMCON = 0b00000111;         // disable comparator to execute digital input A0
    OPTION_REG = 0b00000000;    // bit 7 = 0 => pull up; bit 6 = 0 => falling edge
    TRISA = 0b11111101;         // button as input, RA1 output
    PORTAbits.RA1 = 0;          // mode led off
    /*--------------HANDSHAKE----------------*/
    TRISCbits.TRISC3 = 0x01;
    mydelay(22); //delay 1.5s
    TRISCbits.TRISC3 = 0x00;
    PORTCbits.RC3 = 0;
    /*---------------------------------------*/
    eepdata = eepromread(0x00);         //read eeprom at 0x00 address
    mydelay(253);   //delay20ms         //delay between write and read
    eepselect = eepromread(0x01);       //read eeprom at 0x01 address
    if(eepdata == 0x60){                            //go to 12 volts from the second time run
        dem1 = 3;
    }
    if(eepdata == 0x25 && eepselect == 0x00 ){      //go to 5 volts from the second time run
        dem1 = 0;
    }
    if(eepdata == 0x45){                            //go to 9 volts from the second time run
        dem1 = 2;
    }
    if(eepdata == 0x25 && eepselect == 0x01){       //go to INOV mode then go to changed = 0
        dem1 = 1;
    }
    if(eepdata != 0x25 && eepselect == 0x01){       //go to INOV mode then go to changed = 1
        dem1 = 1;
        changed = 1;            //when we impact increase or decrease button, go to changed = 1
    }
    while(1){
    dem1++;
    switch(dem1){
        case 1: volt5(); break;
        case 2: INOV(); break;
        //5volt
        case 3: volt9(); break;
        case 4: volt12(); dem1 = 0; break;
    }
    mydelay(178);   //delay 0.5s
    while(PORTAbits.RA2 == 1){      //out loop if RA2 = 0(press mode button)
                            
    }    
    }
    return;
}