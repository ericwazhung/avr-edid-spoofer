#include "projInfo.h"	//Don't include in main.h 'cause that's included in other .c's?
#include "main.h"
//#include _USI_UART_HEADER_
//#include _UART_IN_HEADER_
//#include _SINETABLE_HEADER_
//#include _MULTDIV64_HEADER_
//#include _GOERTZ_HEADER_
//#include <util/delay.h>	//for delay_us in initializing the PLL...
//#include <stdio.h> //for sprintf


#if !defined(__AVR_ARCH__)
 #error "__AVR_ARCH__ not defined!"
#endif

//#include <string.h> //for strcmp

#define USICNT_MASK ((1<<USICNT3)|(1<<USICNT2)|(1<<USICNT1)|(1<<USICNT0))
uint8_t usi_i2c_deviceAddressReceived = FALSE;

// There are three basic states: 
//    Awaiting Start Condition
//    Byte Transmission (address/data)
//    ACK Transmission
// We need different Byte Transmission handlers for Read and Write
// Also another for not-addressed
//    This probably isn't necessary, just disable the output drivers
//    and overflow interrupt...?
#define USI_STATE_AWAITING_START		0
#define USI_STATE_AWAITING_ADDRESS	1  
//#define USI_STATE_AWAITING_BYTE		2 //NOT USED
#define USI_STATE_SENDING_ACK_SW		3	//ACK during a READ request
													// slaveWrite
#define USI_STATE_SENDING_ACK_SR		4	//ACK during a WRITE request
													// !slaveWrite = slaveRead
#define USI_STATE_SENDING_ACK_NOT_ADDRESSED 5 // ACK while another
															 // device is selected
#define USI_STATE_RECEIVING_BYTE		6
#define USI_STATE_SENDING_BYTE		7
#define USI_STATE_AWAITING_BYTE_NOT_ADDRESSED 8

uint8_t usi_i2c_state = USI_STATE_AWAITING_START;


usi_i2c_slaveInit(void)
{
	//Set the pin directions...
	setinPORT(SDA_PIN, SDAPORT);
	setinPORT(SCL_PIN, SDAPORT);
	

	usi_i2c_state = USI_STATE_AWAITING_START;

	//Clear the interrupt flags, etc. before enabling the USI...
	USISR = (1<<USISIF) //Start Condition Interrupt Flag
							  //  (ONLY cleared when written 1)
		   | (1<<USIOIF) //Counter Overflow Interrupt Flag
							  //  (ONLY cleared when written 1)
			| (1<<USIPF)  //Stop Condition Flag (not an interrupt)
			| (0<<USIDC)  //Data Collision Flag (Read-Only, not an interrupt)
         | (USICNT_MASK & 0); //Clear the USI counter

	// Assuming the USI isn't being enabled in the middle of a transaction
	//   AND that the master is already initialized... (hokey?)
   //  A USI counter value of 0 should be initialized...
	//  (there will be 16 CLK transitions before the first ACK is necessary)
	//  But this shouldn't matter, because a start-condition will set that

	//Configure the USI...
	USICR = (1<<USISIE) //Enable the start-condition interrupt
			| (0<<USIOIE) //Wait to Enable the counter-overflow interrupt
							  //  until the start-condition is received...
			| (1<<USIWM1) //Enable two-wire mode
			| (1<<USIWM0) // with SCL tie-up during overflow
			| (1<<USICS1) //Select external clocking
			| (0<<USICS0) //  positive-edge
			| (0<<USICLK) //  4-bit counter counts on both external edges
			| (0<<USITC); // DON'T toggle the clock pin 
							  // (this should always be 0)

}

ISR(USI_START_vect)
{
	//A Start-Detection has occurred
	// The slave (this device) has pulled SCL low (from the manual):
	//  The start detector will hold the SCL line low after the master has
	//  forced a negative edge on this line. This allows the slave to wake
	//  up from sleep or complete other tasks before setting up the USI 
	//  Data Register to receive the address. This is done by clearing the 
	//  start condition flag and resetting the counter. 

	//NOT CERTAIN this is necessary
	// Shouldn't be, since the pin is set as an input...
	//   unless a START is received in the middle of a slave-write???
	// From the diagram showing the connections of the shift-registers
	// it seems we need this to prevent data collision
	USIDR = 0xff;	

	//Set the pin directions...
	// Make sure, just in case a start-condition interrupts a slave-write...
	setinPORT(SDA_PIN, SDAPORT);
	setinPORT(SCL_PIN, SDAPORT);

	//Indicate that the address hasn't yet been received
	// (it should be the first byte transmitted)
	//usi_i2c_deviceAddressReceived = FALSE;
	usi_i2c_state = USI_STATE_AWAITING_ADDRESS;


   //Configure the USI to look for counter-overflow (address received)
   USICR = (1<<USISIE) //Enable the start-condition interrupt
         | (1<<USIOIE) //Enable the counter-overflow interrupt
                       // WE'RE READY!!!
         | (1<<USIWM1) //Enable two-wire mode
         | (1<<USIWM0) // with SCL tie-up during overflow
         | (1<<USICS1) //Select external clocking
         | (0<<USICS0) //  positive-edge
         | (0<<USICLK) //  4-bit counter counts on both external edges
         | (0<<USITC); // DON'T toggle the clock pin 
                       // (this should always be 0)


	//Initialize the counter to 0
	// 16 clock edges will signal 8 bits received (address and r/w)
	// AND clear the start-condition interrupt flag
	// (and indirectly release the SCL line)
	USISR = (1<<USISIF) //Start Condition Interrupt Flag
                      //  (ONLY cleared when written 1)
        | (1<<USIOIF) //Counter Overflow Interrupt Flag
                      //  (ONLY cleared when written 1)
        | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
        | (0<<USIDC)  //Data Collision Flag (Read-Only, not an interrupt)
        | (USICNT_MASK & 0); //Clear the USI counter

}

#define USI_I2C_MYADDRESS 0xA0
#define usi_i2c_isMyAddress(addr) ((addr) == USI_I2C_MYADDRESS)
// In case this device handles multiple addresses... (NYI)
uint8_t usi_i2c_requestedAddress = 0;

ISR(USI_OVF_vect)
{
	//The USI counter has overflowed... this occurs in a couple cases..
	// First: After the address and r/w bit have been received
	// Second: After the ACK bit has been transmitted (by the slave)
	// Third: After the first data-byte has been received
	// Fourth: After the ACK bit has been transmitted for it...
	// and so on...

	//The counter-overflow also pulls SCL Low (from the manual):
	// After eight bits containing slave address and data direction 
	// (read or write) have been transferred, the slave counter overflows 
	// and the SCL line is forced low. 

	// If the slave is not the one the 
	// master has addressed, it releases the SCL line and waits for a new 
	// start condition. (NYI)

	uint8_t udrTemp;
	uint8_t requestedAddress;
	uint8_t slaveWrite;

	switch(usi_i2c_state)
	{
		//We should only get here if another device's address was requested
		// Actually, we shouldn't get here yet... 
	//////NYI...
		case USI_STATE_AWAITING_START:
			// Just make sure we don't cause collisions...
			UDR = 0xff;
			heart_setBlink(0x32);
			break;
		//We should get here when /an/ address is received...
		case USI_STATE_AWAITING_ADDRESS:
			//Check to see if it's ours...
			udrTemp = UDR;
			slaveWrite = udrTemp & 0x01;
			requestedAddress = udrTemp & 0xfe;
			
			//From the manual:
			// When the slave is addressed, it holds the SDA line low during 
			// the acknowledgment cycle before holding the SCL line low again
			if( usi_i2c_isMyAddress(requestedAddress) )
			{
				UDR = 0x00; //Hold SDA low for ACK
								// Should probably be 0x7f, but it should be
								// rewritten in the next state
				setoutPORT(SDA_PIN, SDAPORT);


	         //FTM:
	         // Depending on the R/W bit the master or slave enables its 
				// output. If the bit is set, a master read (slave-write)
	         // operation is in progress (the slave drives the SDA line)
	         // The slave can hold the SCL line low after the acknowledge.
				//(it will, due to counter-overflow)
	         if(slaveWrite)
	            usi_i2c_state = USI_STATE_SENDING_ACK_SW;
	         else
	            usi_i2c_state = USI_STATE_SENDING_ACK_SR;
			}
			else
				usi_i2c_state = USI_STATE_SENDING_ACK_NOT_ADDRESSED;

			//FTM:
			// (The USI Counter Register is set to 14 before releasing SCL). 
		   //Initialize the counter to 14
   		// 2 clock edges will signal 1 ACK bit transmitted
			// AND clear the counter-overflow interrupt flag
   		USISR = (0<<USISIF) // Leave the Start Condition Interrupt Flag
        			| (1<<USIOIF) //Counter Overflow Interrupt Flag
                      		  //  (ONLY cleared when written 1)
        			| (1<<USIPF)  //Stop Condition Flag (not an interrupt)
        			| (0<<USIDC)  //Data Collision Flag (Read-Only)
        			| (USICNT_MASK & 14); //Clear the USI counter

			break;
		//ACK has been transmitted from this device...
		case USI_STATE_SENDING_ACK_SR:
			//Release SDA from the ACK (we're reading...)
			setinPORT(SDA_PIN, SDAPORT);
			//Shouldn't be necessary with above, but can't hurt...
			UDR = 0xff; 
			usi_i2c_state = USI_STATE_RECEIVING_BYTE;

// So what's responsible for releaseing SCL around here???


		   //Initialize the counter to 0, clear the counter-overflow flag
		   // 16 clock edges will signal 8 bits received
		   // AND clear the counter-overflow interrupt flag
		   // (and indirectly release the SCL line)   ?????
		   USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
		         | (1<<USIOIF) //Counter Overflow Interrupt Flag
		                      //  (ONLY cleared when written 1)
		         | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
		         | (0<<USIDC)  //Data Collision Flag 
		         | (USICNT_MASK & 0); //Clear the USI counter

			break;
		//ACK has been transmitted from this device...
		case USI_STATE_SENDING_ACK_SW:
			// We're writing, so leave SDA as an output...
			// This shouldn't be necessary, but shouldn't hurt either.
			UDR = ByteToTransmit!!!;
			setoutPORT(SDA_PIN, SDAPORT);

			usi_i2c_state = USI_STATE_SENDING_BYTE;

         //Initialize the counter to 0, clear the counter-overflow flag
         // 16 clock edges will signal 8 bits transmitted
         // AND clear the counter-overflow interrupt flag
         // (and indirectly release the SCL line)  ??????
         USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
               | (1<<USIOIF) //Counter Overflow Interrupt Flag
                            //  (ONLY cleared when written 1)
               | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
               | (0<<USIDC)  //Data Collision Flag 
               | (USICNT_MASK & 0); //Clear the USI counter
         
         break;
		//ACK has been transmitted from another device... (or NAK if none)
		case USI_STATE_SENDING_ACK_NOT_ADDRESSED:
			//No need to release the SDA pin, it shouldn't have been taken...
			// This shouldn't be necessary since it's input-only... (right?)
			UDR = 0xff;
			usi_i2c_state = USI_STATE_AWAITING_BYTE_NOT_ADDRESSED;

         //Initialize the counter to 0, clear the counter-overflow flag
         // 16 clock edges will signal 8 bits received
         // AND clear the counter-overflow interrupt flag
         // (and indirectly release the SCL line)   ?????
         USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
               | (1<<USIOIF) //Counter Overflow Interrupt Flag
                            //  (ONLY cleared when written 1)
               | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
               | (0<<USIDC)  //Data Collision Flag 
               | (USICNT_MASK & 0); //Clear the USI counter

         break;
		// A byte has been received... it should be processed
		case USI_STATE_RECEIVING_BYTE:
			// Get the byte...
			receivedByte = UDR;

			// Also, we need to send an ACK...
			UDR = 0x00;
			setoutPORT(SDA_PIN, SDAPORT);

			// And reload the counter to interrupt after the ACK
         //Initialize the counter to 14, clear the counter-overflow flag
         // 2 clock edges will signal 1 ACK bit transmitted
         // AND clear the counter-overflow interrupt flag
         // (and indirectly release the SCL line)  ??????
         USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
               | (1<<USIOIF) //Counter Overflow Interrupt Flag
                            //  (ONLY cleared when written 1)
               | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
               | (0<<USIDC)  //Data Collision Flag 
               | (USICNT_MASK & 14); //Clear the USI counter

			usi_i2c_state = USI_STATE_SENDING_ACK_SR;
         break;
		// Byte has been sent...
		case USI_STATE_SENDING_BYTE:
			// Send an ACK..
			UDR = 0x00;
			setoutPORT(SDA_PIN, SDAPORT);

         // And reload the counter to interrupt after the ACK
         //Initialize the counter to 14, clear the counter-overflow flag
         // 2 clock edges will signal 1 ACK bit transmitted
         // AND clear the counter-overflow interrupt flag
         // (and indirectly release the SCL line)  ??????
         USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
               | (1<<USIOIF) //Counter Overflow Interrupt Flag
                            //  (ONLY cleared when written 1)
               | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
               | (0<<USIDC)  //Data Collision Flag 
               | (USICNT_MASK & 14); //Clear the USI counter

         usi_i2c_state = USI_STATE_SENDING_ACK_SW;
			break;
		// A Byte has been transmitted to/from another slave...
		case USI_STATE_AWAITING_BYTE_NOT_ADDRESSED:
			// We don't need to ACK
			// But, we should watch for it to assure no data output collisions
			// Reload the UDR with 0xff just in case
			UDR = 0xff;

         // Reload the counter to interrupt after the ACK
         //Initialize the counter to 14, clear the counter-overflow flag
         // 2 clock edges will signal 1 ACK bit transmitted
         // AND clear the counter-overflow interrupt flag
         // (and indirectly release the SCL line)  ??????
         USISR = (0<<USISIF) //Leave the Start Condition Interrupt Flag
               | (1<<USIOIF) //Counter Overflow Interrupt Flag
                            //  (ONLY cleared when written 1)
               | (1<<USIPF)  //Stop Condition Flag (not an interrupt)
               | (0<<USIDC)  //Data Collision Flag 
               | (USICNT_MASK & 14); //Clear the USI counter

         usi_i2c_state = USI_STATE_SENDING_ACK_NOT_ADDRESSED;
			break;
		default:
			break;
	}
}



}



int main(void)
{

	//Found experimentally: assuming the free-running ADC is always 13
	// cycles per interrupt...
	// The default value was read to be 0x9f
	// This is of course device-specific
//	OSCCAL = 0x9a;

	//*** Initializations ***

	//!!! WDT could cause problems... this probably should be inited earlier and called everywhere...
	//INIT_HEARTBEAT(HEARTBEATPIN, HEARTBEAT, HEARTCONNECTION);
	init_heartBeat();

 	setHeartRate(0);	
	
	//This was only necessary for debugging timer initialization bugs...
	// which have been resolved
//	set_heartBlink(retVal);

	while(1)
	{
		heartUpdate();
		
	}

}


