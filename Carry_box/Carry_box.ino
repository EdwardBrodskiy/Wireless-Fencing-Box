#define BLADE_BUTTON_OUT 4
#define BLADE_BUTTON_IN 3
#define RF_READ_INPUT 5


// debug

bool debug = true;

// blade
volatile unsigned long hit_time;
volatile bool hit_detected = false;

bool hit_not_on_guard = false;
bool hit_confirmed = false;

// transmission
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);
                                                                           // Topology
byte addresses[][6] = {"1Node","2Node","3Node"};              

byte side = 1; // 0 | 1 : red | green

void setup() {
  if(debug) Serial.begin(9600);
  if(debug) printf_begin();
  
  // setup hit monitoring
  pinMode(RF_READ_INPUT, INPUT);
  setup_detection();
  
  // setup transmission
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,63);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads
  radio.openWritingPipe(addresses[1 + side]);    // Open different pipes when writing. Write on pipe 0, address 0
  radio.openReadingPipe(1,addresses[0]);  // Read on pipe 1, as address 1
  radio.startListening();                 // Start listening
  radio.powerUp();
  if(debug) radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  
}

void setup_detection(){
  pinMode(BLADE_BUTTON_IN, INPUT_PULLUP);
  attachInterrupt(BLADE_BUTTON_IN - 2, detect_hit, FALLING);
  pinMode(BLADE_BUTTON_OUT, OUTPUT);
  digitalWrite(BLADE_BUTTON_OUT, LOW);
  
}

bool listen_for_guard(){
  detachInterrupt(BLADE_BUTTON_IN - 2);
  pinMode(BLADE_BUTTON_IN, INPUT);
  pinMode(BLADE_BUTTON_OUT, INPUT);
  return digitalRead(RF_READ_INPUT);
}

void loop() {
  if(hit_detected){
    hit_detected = false;
    unsigned long timer = micros();
    hit_not_on_guard = !listen_for_guard();
    setup_detection();
    if(debug) Serial.println("Hit detected: " + String(hit_not_on_guard) + " Took: " + (micros() - timer));
  }
  
  if(hit_not_on_guard && ((micros() - hit_time) > 1000)){
    hit_not_on_guard = false;
    if(!digitalRead(BLADE_BUTTON_IN)){
      hit_confirmed = true;
    }
  }

  if(hit_confirmed){
    if(debug) Serial.println("Hit Confirmed!");
    if(transmit_hit(1)){
      hit_confirmed = false;
    }
    hit_confirmed = false; // quick debbug change !!!!!!!!!!!!!!!!!
  }

}

void detect_hit(){
  if(debug) Serial.println("Interupt");
  hit_time = micros();
  hit_detected = true;
  
}

bool transmit_hit(byte message){
  bool success = false;
  byte gotByte;                                           // Initialize a variable for the incoming response
    
  radio.stopListening();                                  // First, stop listening so we can talk.      
  // printf("Now sending %d as payload. ",message);                // Use a simple byte counter as payload
  unsigned long time = micros();                          // Record the current microsecond count   
  
  if ( radio.write(&message,1) ){                         // Send the counter variable to the other radio 
      if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
          if(debug) printf("Got blank response. round-trip delay: %lu microseconds\n\r",micros()-time);     
      }else{      
          while(radio.available() ){                      // If an ack with payload was received
              radio.read( &gotByte, 1 );                  // Read it, and display the response time
              if(debug) printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,micros()-time);
              success = true;
          }
      }
  
  }else{        
    if(debug) printf("Sending failed.\n\r");  // If no ack response, sending failed
  }          
  
  return success;
}
