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
byte addresses[][6] = {"1Node","2Node","3Node"};              

byte side = 0; // 0 | 1 : red | green

void setup() {
  Serial.begin(9600);
  printf_begin();
  Serial.println("begin");
  // setup interupt
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(1, detect_hit, FALLING);
  // setup transmission
  Serial.println("st");
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads
  radio.openWritingPipe(addresses[1 + side]);    // Open different pipes when writing. Write on pipe 0, address 0
  radio.openReadingPipe(1,addresses[0]);  // Read on pipe 1, as address 1
  radio.startListening();                 // Start listening
  radio.powerUp();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  Serial.println("est");
}

void loop() {
  //delay(1000);
  //Serial.println(!digitalRead(3));
  //Serial.println(millis() - hit_time);
  if(hit_confirmed && (micros() - hit_time) > 1000 && !digitalRead(3)){
    if(transmit_hit(1)){
      hit_confirmed = false;
    }
  }

}

void detect_hit(){
  hit_time = micros();
  hit_confirmed = true;
}

bool transmit_hit(byte message){
  bool success = false;
  byte gotByte;                                           // Initialize a variable for the incoming response
    
  radio.stopListening();                                  // First, stop listening so we can talk.      
  printf("Now sending %d as payload. ",message);                // Use a simple byte counter as payload
  unsigned long time = micros();                          // Record the current microsecond count   
  
  if ( radio.write(&message,1) ){                         // Send the counter variable to the other radio 
      if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
          printf("Got blank response. round-trip delay: %lu microseconds\n\r",micros()-time);     
      }else{      
          while(radio.available() ){                      // If an ack with payload was received
              radio.read( &gotByte, 1 );                  // Read it, and display the response time
              printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,micros()-time);
              success = true;
          }
      }
  
  }else{        printf("Sending failed.\n\r"); }          // If no ack response, sending failed
  
  return success;
}
