

unsigned long t = micros();

int buff[128] = {0};

int b_pointer = 0;

void setup() {
  Serial.begin(9600);
  pinMode(8, INPUT);

}

void loop() {
  if ( (micros() - t) > 8){
    t = micros();
    buff[b_pointer] = digitalRead(8);
    b_pointer++;
  }
  if(b_pointer == 128){
    for(int b: buff) Serial.print(b);
    Serial.println("");
    b_pointer = 0;
  }
}
