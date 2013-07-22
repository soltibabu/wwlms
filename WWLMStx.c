/*
 *Not tested yet, Transmitter with interrup on INT1
 *Supposed to Transmit 4 times  and sleep, awakwned by low on INT1 pin
 
 
 
 * wwlms_tx.c
 *
 * Created: 7/21/2013 11:59:23 PM
 *  Author: Navaraj
 */ 


#define SCK 5   // SPI clock
#define SDO 4   // SPI Data output (RFM12B side)
#define SDI 3   // SPI Data input (RFM12B side)
#define CS  2   // SPI SS (chip select)
#define NIRQ 2  // (PORTD)

#define HI(x) PORTB |= (1<<(x))
#define LO(x) PORTB &= ~(1<<(x))
#define WAIT_NIRQ_LOW() (PIND&(1<<NIRQ))

#define LED 6
#define LED_OFF() PORTD &= ~(1<<LED)
#define LED_ON() PORTD |= (1<<LED)


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>



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
		if(cmd&0x8000) HI(SDI); else LO(SDI);
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

void rfInit() {
	writeCmd(0x80E7); //EL,EF,868band,12.0pF
	writeCmd(0x8239); //!er,!ebb,ET,ES,EX,!eb,!ew,DC
	writeCmd(0xA640); //frequency select
	writeCmd(0xC647); //4.8kbps
	writeCmd(0x94A0); //VDI,FAST,134kHz,0dBm,-103dBm
	writeCmd(0xC2AC); //AL,!ml,DIG,DQD4
	writeCmd(0xCA81); //FIFO8,SYNC,!ff,DR
	writeCmd(0xCED4); //SYNC=2DD4G
	writeCmd(0xC483); //@PWR,NO RSTRIC,!st,!fi,OE,EN
	writeCmd(0x9850); //!mp,90kHz,MAX OUT
	writeCmd(0xCC17); //OB1COB0, LPX,IddyCDDITCBW0
	writeCmd(0xE000); //NOT USE
	writeCmd(0xC800); //NOT USE
	writeCmd(0xC040); //1.66MHz,2.2V
}

void rfSend(unsigned char data){
	while(WAIT_NIRQ_LOW());
	writeCmd(0xB800 + data);
}

void sendPacket(void)
{
	for(int k=0;k<4;k++)
	{
	LED_ON();
	writeCmd(0x0000);
	rfSend(0xAA); // PREAMBLE
	rfSend(0xAA);
	rfSend(0xAA);
	rfSend(0x2D); // SYNC
	rfSend(0xD4);
	for(int i=0; i<16; i++) {
		rfSend(0x30+i);
	}
	rfSend(0xAA); // DUMMY BYTES
	rfSend(0xAA);
	rfSend(0xAA);
	LED_OFF();
	for(int i=0;i<1000;i++) for(int j=0;j<123;j++);
	}
}

int main() {
	volatile unsigned int i,j;
	EICRA=0x00;  //configure INT1 to trigger on low level
	asm("cli");
	for(i=0;i<1000;i++) for(j=0;j<123;j++);
	portInit();
	rfInit();
	while(1)
	{
		sendPacket();
		sleep_enable();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		EIMSK|=(1<<INT1); //ENABLE INTERRUPT 1
		ADCSRA=0;
		asm("cli");
		MCUCR|=(1<<BODS);
		MCUCR&=~(1<<BODSE);
		asm("sei");		//ensure interrupt enable so we can wake up again
		sleep_cpu(); //go to sleep
		sleep_disable(); //Wake up here
		
	}
}

ISR(INT1_vect)
{
	EIMSK&=~(1<<INT1);
}
