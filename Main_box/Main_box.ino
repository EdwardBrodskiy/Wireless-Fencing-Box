// --- debbug display ---

#include <Wire.h>

#include <LiquidCrystal_I2C.h>

const int en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
// const int  en = 4, rw = 5, rs = 5, d4 = 0, d5 = 1, d6 = 2, d7 = 3, bl = 7;
const int i2c_addr = 0x27;

LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

int count = 0;
volatile int i_count = 0;

// --- box logic ---

volatile bool new_info = false;

volatile unsigned long hit_time[] = {0, 0};

volatile bool hits[] = {false, false};

unsigned long gap = 0;

// output
unsigned long last_change = 0;

String results[] = {"Red Hit", "Green Hit", "Double"};

// --- transmission ---

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);
                                                                           // Topology
byte addresses[][6] = {"1Node","2Node","3Node"};              // Radio pipe addresses for the 2 nodes to communicate.



void setup() {
  Serial.begin(9600);
  printf_begin();

  // Setup and configure radio

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(addresses[0]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,addresses[1]);      // Open a reading pipe on address 0, pipe 1
  radio.openReadingPipe(2,addresses[2]);
  radio.startListening();                 // Start listening
  radio.powerUp();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  // setup interupt
  pinMode(3, INPUT);
  attachInterrupt(1, recieve, LOW);
  
  // lcd
  lcd.begin(16,2);

  lcd.setCursor(0,0);
  lcd.print("Welcome");

}

void loop() {
  
  if(new_info){
    new_info = false;
    count++;
    if(hits[0] && hits[1]){
      if(hit_time[0] > hit_time[1]){
        gap = hit_time[0] - hit_time[1];
      }else{
        gap = hit_time[1] - hit_time[0];
      }
      
      if(gap < 40){
        output(results[2]);
      }else if(hit_time[0] < hit_time[1]){
        output(results[0]);
      }else{
        output(results[1]);
      }
    }else if(hits[0]){
       output(results[0]);
    }else if(hits[1]){
       output(results[1]);
    }
    last_change = millis();
  }
 
  if((hits[0] or hits[1]) and ((millis() - last_change) > 3000)){
     hits[0] = false;
     hits[1] = false;
     output("Fence");
     count = 0;
     i_count = 0;
  }
}

void recieve(){
  byte pipeNo, gotByte;                          // Declare variables for the pipe and the byte received
  while( radio.available(&pipeNo)){              // Read all available payloads
    radio.read( &gotByte, 1 );                   
                                                 // Since this is a call-response. Respond directly with an ack payload.
                                                 // Ack payloads are much more efficient than switching to transmit mode to respond to a call
    radio.writeAckPayload(pipeNo,&gotByte, 1 );  // This can be commented out to send empty payloads.
    if(!hits[pipeNo - 1]){
      hit_time[pipeNo - 1] = millis();
      hits[pipeNo - 1] = true;
      new_info = true;
    }
    i_count++;
  }
  
}

void output(String m){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(m); 
  lcd.setCursor(0,1);
  lcd.print(String(gap) + " : " + String(count)+ " : " + String(i_count));
}
