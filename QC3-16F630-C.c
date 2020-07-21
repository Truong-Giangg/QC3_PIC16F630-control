#define _XTAL_FREQ 20000000
#include <xc.h>
int dem1 = 1;
unsigned char eepdata;      
unsigned char eepselect;
unsigned char eep1;
int changed = 0;
char dem2 = 0x00;

void volt12(){
    //TRISCbits.TRISC4 = 0x01;    // d+ =0,6V
    //TRISCbits.TRISC3 = 0x01;    // d- =0,6V
    TRISC = 0b00011000;
    PORTC = 0b00000100;
    //-------------------
    EEADR = 0x00;
    EEDATA = 0x60;
    EECON1bits.WREN = 1;    //ENABLE WRITE
    EECON2 = 0x55;          //UNLOCK WRITE
    EECON2 = 0xAA;          //UNLOCK WRITE
    EECON1bits.WR = 1;      //START WRITE
}
void volt9(){
    //TRISCbits.TRISC4 = 0x00;    // 
    //PORTCbits.RC4 = 1;          // d+ =3,3V
    //TRISCbits.TRISC3 = 0x01;    // d- =0,6V
    TRISC = 0b00001000;
    PORTC = 0b00010010;
    //-------------------------
    EEADR = 0x00;
    EEDATA = 0x45;
    EECON1bits.WREN = 1;    //ENABLE WRITE
    EECON2 = 0x55;          //UNLOCK WRITE
    EECON2 = 0xAA;          //UNLOCK WRITE
    EECON1bits.WR = 1;      //START WRITE
}
void volt5(){
    //TRISCbits.TRISC4 = 0x01;    // d+ = 0,6V
    //TRISCbits.TRISC3 = 0x00;
    //PORTCbits.RC3 = 0;          // d- = 0V
    TRISC = 0b00010000;
    PORTC = 0b00000001;
    PORTAbits.RA1 = 0;          // turn off mode led
    //----------------------------
    EEADR = 0x00;
    EEDATA = 0x25;
    EECON1bits.WREN = 1;    //ENABLE WRITE
    EECON2 = 0x55;          //UNLOCK WRITE
    EECON2 = 0xAA;          //UNLOCK WRITE
    EECON1bits.WR = 1;      //START WRITE
    //-----------------------------
    __delay_ms(20);
    EEADR = 0x01;
    EEDATA = 0x00;          //bit to select continuous mode or 5v mode
    EECON1bits.WREN = 1;    //ENABLE WRITE
    EECON2 = 0x55;          //UNLOCK WRITE
    EECON2 = 0xAA;          //UNLOCK WRITE
    EECON1bits.WR = 1;      //START WRITE
}
void inc200mv(){
        TRISCbits.TRISC4 = 0x00;
        PORTCbits.RC4 = 1;      // d+ = 3V3
        __delay_ms(10);
        PORTCbits.RC4 = 0;      // d+ = 0V
        //TRISCbits.TRISC4 = 0x01;// d+ = 0,6V
        __delay_ms(10);
}
void dec200mv(){
        TRISCbits.TRISC3 = 0x00;// d- =0,6V
        PORTCbits.RC3 = 0;
        __delay_ms(10);
        PORTCbits.RC3 = 1;      // d- = 3V3
        __delay_ms(10);
}
void continuous(){
    //TRISCbits.TRISC3 = 0x00;
    //PORTCbits.RC3 = 1;          // d- = 3V3
    //TRISCbits.TRISC4 = 0x01;    // d+ = 0,6V
    TRISC = 0b00010000;
    PORTC = 0b00001000;
    PORTAbits.RA1 = 1;
    
    if(changed == 0){           //the first time controller run
        //select continuous mode use EEPROM---------------
        EEADR = 0x01;
        EEDATA = 0x01;              //bit to select continuous mode
        EECON1bits.WREN = 1;        //ENABLE WRITE
        EECON2 = 0x55;              //UNLOCK WRITE
        EECON2 = 0xAA;              //UNLOCK WRITE
        EECON1bits.WR = 1;          //START WRITE
        //--------------------------------
        __delay_ms(20);
        EEADR = 0x00;
        EEDATA = 0x25;              //we use eepselect because we not want 5volts and continuous mode coincidence 0x25 value
        EECON1bits.WREN = 1;        //ENABLE WRITE
        EECON2 = 0x55;              //UNLOCK WRITE
        EECON2 = 0xAA;              //UNLOCK WRITE
        EECON1bits.WR = 1;          //START WRITE
    }
    if(changed == 1){
        //select continuous mode--------------------------------
        EEADR = 0x01;           //select address to write (0x01)
        EEDATA = 0x01;          //bit to select continuous mode
        EECON1bits.WREN = 1;    //ENABLE WRITE
        EECON2 = 0x55;          //UNLOCK WRITE
        EECON2 = 0xAA;          //UNLOCK WRITE
        EECON1bits.WR = 1;      //START WRITE
        //--------------------------------
        __delay_ms(20);     //must be delay between write and read
        EEADR = 0x00;       //select address to read (0x00)
        EECON1bits.RD = 1;  //enable read
        eep1 = EEDATA;      // store data from EEPROM to eep1
        if(eep1 > 0x25){    //if we increase voltage
            for(int i = 0x01; i<= eep1-0x25 ;i++ ){
                inc200mv();
            }
        }
        if(eep1 < 0x25){    //if we decrease voltage
            for(int i = 0x01; i <= 0x25 - eep1; i++){
                dec200mv();
            }
        }
    }
}
void __interrupt() ISR(void){
    if(INTCONbits.INTF == 1){   //RA2/INT external interrupt flag = 1 when interrupt occurred
        dem1++;
        switch(dem1){
        case 1: volt5(); break;
        case 2: continuous(); break;
        case 3: volt5(); break;
        case 4: volt9(); break;
        case 5: volt12(); dem1 = 0; break;
    }
        INTCONbits.INTF = 0;    // reset external interrupt flag
    }    
}
void main(void) {
    TRISA = 1;
    TRISCbits.TRISC0 = 0x00;
    TRISCbits.TRISC2 = 0x00;
    TRISCbits.TRISC1 = 0x00;
    TRISAbits.TRISA5 = 0x00;
    PORTCbits.RC0 = 0;              // 5v led
    PORTCbits.RC1 = 0;              // 9v led
    PORTCbits.RC2 = 0;              // 12v led 
    PORTAbits.RA1 = 0;              // mode led
    CMCON = 0b00000111;             // disable comparator to execute digital input    
    OPTION_REG = 0b00000000;    // bit 7 = 0 => pull up; bit 6 = 0 => falling edge
    INTCONbits.GIE =1;          // global interrupt enable
    INTCONbits.PEIE = 1;        // peripheral interrupt enable
    INTCONbits.INTE =1;         // RA2/INT external interrupt enable
    
    TRISAbits.TRISA0 = 0x01;        // increase button
    TRISAbits.TRISA4 = 0x01;        // decrease button
    TRISAbits.TRISA2 = 0x01;        // external interrupt A2 button
    
    /*--------------HANDSHAKE----------------*/
    TRISCbits.TRISC3 = 0x00;    // d-
    PORTCbits.RC3 = 0;          // d- = 0V
    __delay_ms(100);            // delay 1s to handshake   
    TRISCbits.TRISC3 = 0x01;    // d- = 0,6V, handshake complete
    __delay_ms(10);
    //--------------------------------------        
    
    /*read data from 0x00 of EEPROM-------the first time run, there no data at all*/
    EEADR = 0x00;           //select address to read (address 0x00)
    EECON1bits.RD = 1;      //enable read
    eepdata = EEDATA;       // store data from EEPROM to eepdata
    /*read data from 0x01 of EEPROM-------the first time run, there no data at all*/
    __delay_ms(20);         //must be delay between write and read
    EEADR = 0x01;           //select address to read (address 0x01)
    EECON1bits.RD = 1;      //enable read
    eepselect = EEDATA;     // store data from EEPROM to eepselect
    
    if(eepdata == 0x60){                            //go to 12 volts from the second time run
        dem1 = 0;
        volt12();
    }
    if(eepdata == 0x25 && eepselect == 0x00 ){      //go to 5 volts from the second time run
        dem1 = 1;
        volt5();
    }
    if(eepdata == 0x45){                            //go to 9 volts from the second time run
        dem1 = 4;
        volt9();
    }
    if(eepdata == 0x25 && eepselect == 0x01){       //go to continuous mode
        dem1 = 2;
        continuous();
    }
    if(eepdata != 0x25 && eepselect == 0x01){       //go to continuous mode then go to changed = 0
        dem1 = 2;
        changed = 1;                //when we impact increase or decrease button, go to changed = 1
        continuous();
    }
    if(eepdata == 0xff ){
        volt5();
    }

    while(1){

        while(PORTAbits.RA1 == 1){      // out loop if mode led = 0, press mode button
        if(PORTAbits.RA0 == 0){         // increase 0,2V if we press button 2
            inc200mv();
            //------------------------------
            dem2 ++;            
            EEADR = 0x00;
            char t = 0x25 + dem2;
            EEDATA = t;          
            EECON1bits.WREN = 1;    //ENABLE WRITE
            EECON2 = 0x55;          //UNLOCK WRITE
            EECON2 = 0xAA;          //UNLOCK WRITE
            EECON1bits.WR = 1;      //START WRITE
        }

        if(PORTAbits.RA4 == 0){         // decrease 0,2V if we press button 3
            dec200mv();
            //-----------------------------------
            dem2 --;            
            EEADR = 0x00;
            char t1 = 0x25 + dem2;
            EEDATA = t1;          
            EECON1bits.WREN = 1;    //ENABLE WRITE
            EECON2 = 0x55;          //UNLOCK WRITE
            EECON2 = 0xAA;          //UNLOCK WRITE
            EECON1bits.WR = 1;      //START WRITE
        }
        __delay_ms(50);
        }
        
    }
    return;
}