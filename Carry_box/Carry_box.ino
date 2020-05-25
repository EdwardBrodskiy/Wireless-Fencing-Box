// blade
volatile unsigned long hit_time;
volatile bool hit_confirmed = false;

// transmission
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);
                                                                           // Topology
byte addresses[][6] = {"1Node","2Node"};              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;

void setup() {
  // setup interupt
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(1, hit_detect, FALLING)
  // setup transmission

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads
  radio.openWritingPipe(addresses[0]);    // Open different pipes when writing. Write on pipe 0, address 0
  radio.openReadingPipe(1,addresses[1]);  // Read on pipe 1, as address 1
  radio.startListening();                 // Start listening
  radio.powerUp();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void hit_detect(){
  
}

bool transmit_hit(){
  byte gotByte;                                           // Initialize a variable for the incoming response
    
  radio.stopListening();                                  // First, stop listening so we can talk.      
  printf("Now sending %d as payload. ",1);                // Use a simple byte counter as payload
  unsigned long time = micros();                          // Record the current microsecond count   
                                                          
  if ( radio.write(&counter,1) ){                         // Send the counter variable to the other radio 
      if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
          printf("Got blank response. round-trip delay: %lu microseconds\n\r",micros()-time);     
      }else{      
          while(radio.available() ){                      // If an ack with payload was received
              radio.read( &gotByte, 1 );                  // Read it, and display the response time
              printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,micros()-time);
              counter++;                                  // Increment the counter variable
          }
      }
  
  }else{        printf("Sending failed.\n\r"); }          // If no ack response, sending failed
}
