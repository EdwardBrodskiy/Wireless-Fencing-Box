#define BLADE_BUTTON_OUT 4
#define BLADE_BUTTON_IN 3
#define RF_READ 8
#define RF_WRITE 7


// debug

bool debug = true;

// blade
volatile unsigned long hit_time;
volatile bool hit_detected = false;




// guard
// TX
byte last_bit_on_guard = 0;
unsigned long last_bit_change = micros();
const byte tx_rate = 64;
// RX
const byte rx_rate = 8;
byte buff[1000 / rx_rate / 2] = {0};


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
  
  setup_hit_detection();

  // guard detection
  pinMode(RF_READ, INPUT);
  pinMode(RF_WRITE, OUTPUT);
  
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

void setup_hit_detection(){
  pinMode(BLADE_BUTTON_OUT, OUTPUT);
  digitalWrite(BLADE_BUTTON_OUT, LOW);
  pinMode(BLADE_BUTTON_IN, INPUT_PULLUP);
  attachInterrupt(BLADE_BUTTON_IN - 2, detect_hit, FALLING);
}

void setup_hit_end_detection(){
  pinMode(BLADE_BUTTON_OUT, OUTPUT);
  digitalWrite(BLADE_BUTTON_OUT, LOW);
  pinMode(BLADE_BUTTON_IN, INPUT_PULLUP);
  attachInterrupt(BLADE_BUTTON_IN - 2, detect_hit_end, RISING);
}

bool check_for_guard(){
  digitalWrite(RF_WRITE, 0); // to remove noise
  int b_pointer = 0;
  unsigned long t = micros() - 8;
  unsigned long rx_start = micros();
  while((micros() - rx_start) < 1000){
    if ( (micros() - t) > 8){
      t = micros();
      buff[b_pointer++] = digitalRead(RF_READ);
    }
  }
  // for(int b: buff) Serial.print(b);
  //Serial.println("");
  byte total = 0;
  for(int b:buff) total += b;
  
  return total > 30;
}

void loop() {
  if(hit_detected){
    hit_detected = false;
    
    unsigned long timer = micros();
    
    bool hit_on_guard = !check_for_guard();
    
    setup_hit_end_detection();
    
    if(!hit_on_guard && !digitalRead(BLADE_BUTTON_IN)){
      if(debug) Serial.println("Guard detected: " + String(hit_on_guard) + " Took: " + (micros() - timer));
      
      bool hit_transmitted = false;
      while(!hit_transmitted){
        if(debug) Serial.println("Hit Confirmed!");
        if(transmit_hit(1)){
          hit_transmitted = true;
        }
        hit_transmitted = true; // quick debbug change as transsmission will never be successful (main box is off)
      }
    }
    
  }

  if ( (micros() - last_bit_change) > tx_rate){
    last_bit_change = micros();
    digitalWrite(RF_WRITE, last_bit_on_guard);
    if(last_bit_on_guard == 1){
      last_bit_on_guard = 0;
    }else{
      last_bit_on_guard = 1;
    }
  }
}

void detect_hit(){
  
  detachInterrupt(BLADE_BUTTON_IN - 2);
  pinMode(BLADE_BUTTON_IN, INPUT);
  pinMode(BLADE_BUTTON_OUT, INPUT);
  if(debug) Serial.println("Button Down");
  hit_time = micros();
  hit_detected = true;
  
}

void detect_hit_end(){
  detachInterrupt(BLADE_BUTTON_IN - 2);
  attachInterrupt(BLADE_BUTTON_IN - 2, detect_hit, FALLING);
  if(debug) Serial.println("Button Up");
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
