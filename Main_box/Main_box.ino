
// transmission
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);
                                                                           // Topology
byte addresses[][6] = {"1Node","2Node"};              // Radio pipe addresses for the 2 nodes to communicate.


void setup() {
  Serial.begin(9600);
  printf_begin();

  // Setup and configure radio

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,addresses[0]);      // Open a reading pipe on address 0, pipe 1
  radio.startListening();                 // Start listening
  radio.powerUp();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

}

void loop() {
  byte pipeNo, gotByte;                          // Declare variables for the pipe and the byte received
  while( radio.available(&pipeNo)){              // Read all available payloads
    radio.read( &gotByte, 1 );                   
                                                 // Since this is a call-response. Respond directly with an ack payload.
                                                 // Ack payloads are much more efficient than switching to transmit mode to respond to a call
    radio.writeAckPayload(pipeNo,&gotByte, 1 );  // This can be commented out to send empty payloads.
    printf("Sent response %d \n\r", gotByte);  
 }

}
