byte b = 0;

unsigned long t = micros();

void setup() {
  pinMode(7, OUTPUT);

}

void loop() {
  if ( (micros() - t) > 64){
    t = micros();
    digitalWrite(7, b);
    if(b == 1){
      b = 0;
    }else{
      b = 1;
    }
  }

}
