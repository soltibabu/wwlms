//Receiver side code. . a random data received is displayed via serial port @19200 baud rate.
//Author: Navaraj Darlami


#define SCK 5   // SPI clock   
#define SDO 4   // SPI Data output (RFM12B side)   
#define SDI 3   // SPI Data input (RFM12B side)   
#define CS  2   // SPI SS (chip select)   
#define NIRQ 2  // (PORTD)   
   
#define HI(x) PORTB |= (1<<(x))   
#define LO(x) PORTB &= ~(1<<(x))   
#define WAIT_NIRQ_LOW() while(PIND&(1<<NIRQ))   
   
#define LED 6   
#define LED_OFF() PORTD &= ~(1<<LED)   
#define LED_ON() PORTD |= (1<<LED)   
   
#define BAUDRATE 51 // 19200 at 16MHz   
   
#include <avr/io.h>   
   
void rsInit(unsigned char baud) {   
  UBRR0L = baud;   
  UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);  // 8N1   
  UCSR0B = (1<<RXEN0) | (1<<TXEN0);    // enable tx and rx   
}   
   
void rsSend(unsigned char data) {   
  while( !(UCSR0A & (1<<UDRE0)));   
  UDR0 = data;   
}   
   
unsigned char rsRecv() {   
  while( !(UCSR0A & (1<<RXC0)));   
  return UDR0;   
}   
   
void portInit() {   
  HI(CS);   
  HI(SDI);   
  LO(SCK);   
  DDRB = (1<<CS) | (1<<SDI) | (1<<SCK);   
  DDRD = (1<<LED);    
}   
   
unsigned int writeCmd(unsigned int cmd) {   
  unsigned char i;   
  unsigned int recv;   
  recv = 0;   
  LO(SCK);   
  LO(CS);   
  for(i=0; i<16; i++) {   
    if(cmd&0x8000) {   
      HI(SDI);   
    }   
    else {   
      LO(SDI);   
    }   
    HI(SCK);   
    recv<<=1;   
    if( PINB&(1<<SDO) ) {   
      recv|=0x0001;   
    }   
    LO(SCK);   
    cmd<<=1;   
  }   
  HI(CS);   
  return recv;   
}   
   
void FIFOReset() {   
  writeCmd(0xCA81);   
  writeCmd(0xCA83);   
}   
/*  
void waitForData() {  
  unsigned int status;  
  while(1) {  
    status = writeCmd(0x0000);  
    if ( (status&0x8000) ) {  
      return;  
    }  
  }  
}  
*/   
void rfInit() {   
  writeCmd(0x80E7); //EL,EF,868band,12.0pF   
  writeCmd(0x8299); //er,!ebb,ET,ES,EX,!eb,!ew,DC   
  writeCmd(0xA640); //freq select   
  writeCmd(0xC647); //4.8kbps   
  writeCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm   
  writeCmd(0xC2AC); //AL,!ml,DIG,DQD4   
  writeCmd(0xCA81); //FIFO8,SYNC,!ff,DR   
  writeCmd(0xCED4); //SYNC=2DD4;   
  writeCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN   
  writeCmd(0x9850); //!mp,90kHz,MAX OUT   
  writeCmd(0xCC17); //!OB1,!OB0, LPX,!ddy,DDIT,BW0   
  writeCmd(0xE000); //NOT USE   
  writeCmd(0xC800); //NOT USE   
  writeCmd(0xC040); //1.66MHz,2.2V   
}   
   
/*  
unsigned char rfRecv() {  
  unsigned int data;  
  writeCmd(0x0000);  
  data = writeCmd(0xB000);  
  return (data&0x00FF);  
}  
*/   
   
unsigned char rfRecv() {   
  unsigned int data;   
  while(1) {   
    data = writeCmd(0x0000);   
    if ( (data&0x8000) ) {   
      data = writeCmd(0xB000);   
      return (data&0x00FF);   
    }   
  }     
}   
   
int main(void) {   
  unsigned char data, i;   
  LED_ON();   
  portInit();   
  rfInit();   
  rsInit(BAUDRATE);   
  FIFOReset();   
  while(1) {     
      for (i=0; i<16; i++) {   
        data = rfRecv();   
        rsSend(data);   
      }   
      FIFOReset();   
  }   
  return 0;   
}  
