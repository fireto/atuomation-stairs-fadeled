#include <FadeLed.h>
#include <Bounce2.h>

//gives the number of elements in an array
#define elements(x) sizeof(x)/sizeof(x[0])

//define input pins
#define pir1_pin 52
#define pir2_pin 53
#define sw_pin 50

//define inputs as bounce objects
Bounce pir1;
Bounce pir2;
Bounce sw;

//define led strips as FadeLed objects
FadeLed leds[] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3};  

byte incomingByte = 0;   // for incoming serial data
unsigned long millisLast; // for timer
bool wasFadingUp = false; // store direction of fading
bool swShort = false; // flag for sw short/long presses
bool manual = false; // flag if manual sw was used

// configure fading 
int fadeTime = 1000; // fade in/out time in ms
int onTime = 5000; // max on duration in ms


void setup() {
  Serial.begin(9600);
  //set the interval (ms) between LED updates
  //it's the same for all FadeLed objects
  //call BEFORE setTime() otherwise the time calculation is wrong
  //default is 50ms (gives 20 updates a second)
  FadeLed::setInterval(10);

  //set to constant fade time for all leds, by setting second parameter to true
  for(byte i = 0; i < elements(leds); i++){
    leds[i].setTime(fadeTime, true);
  }
  //setup the inputs
  pir1.attach(pir1_pin, INPUT_PULLUP);
  pir2.attach(pir2_pin, INPUT_PULLUP);
  sw.attach(sw_pin, INPUT_PULLUP);
}

void allOff(byte fadeFactor) {
  for(byte i = 0; i < elements(leds); i++){
    leds[i].setTime(fadeTime * fadeFactor, true);
    leds[i].off();
  }
}

void allOn(byte fadeFactor) {
  for(byte i = 0; i < elements(leds); i++){
    leds[i].setTime(fadeTime * fadeFactor, true);    
    leds[i].on();
  }
}

void upDown() {
  for(byte i = elements(leds); i > 0; i--){
    leds[i-1].setTime(((elements(leds)-i+1)*fadeTime), true);    
    leds[i-1].on();
  }
}
void downUp() {
  for(byte i = 0; i < elements(leds);i++){
      leds[i].setTime(((i+1)*fadeTime), true);
      leds[i].on();
    }
}

void allToggle(){
  Serial.println("toggle all leds");
  for(byte i = 0; i < elements(leds);i++){
      leds[i].setTime(fadeTime, true); 
      toggle(i);
    }
}

void toggle(byte x) {
  if(leds[x].done()){
//    Serial.print("toggle led:");
//    Serial.println(x);
    if(leds[x].get()) {
      leds[x].off();
    }
    else {
      leds[x].on();
    }
  }
}


void loop() {
  FadeLed::update();
  pir1.update();
  pir2.update();
  sw.update();

  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    switch (incomingByte) {
      case 49:
        millisLast = millis();
        Serial.println("pir1-downUp");
        downUp();
        break;
      case 50:
        millisLast = millis();
        Serial.println("pir2-upDown");
        upDown();
        break;
      case 51:
        millisLast = millis();
        Serial.println("sw");
        if(leds[0].get()) {
          allOff(1);
        }
        else {
          allOn(1);
        }
        break;
      case 52:
        toggle(0);
        break;
      case 53:
        toggle(1);
        break;  
      case 54:
        toggle(2);
        break;                
    }
  }
  
  // pir1 active  
  if( pir1.fell() ) {
    Serial.println("pir1 activated");
    millisLast = millis();
    if (!manual) upDown();
  }

  // pir2 active
  if( pir2.fell() ) {
    Serial.println("pir2 activated");
    millisLast = millis();
    if (!manual) downUp();
  }

  // short press of sw
  if( sw.fell() and sw.duration() < 1000 ) {
      millisLast = millis();
      Serial.println("sw fell short");
      swShort = true;
      manual = true;
      allToggle();
    }
  // sw is pressed for more than a second
  if( !sw.read() and sw.duration() > 1000) {
      swShort = false;
      manual = true;
      // fade all leds on and off 
      for(byte i = 0; i < elements(leds); i++){
        if(leds[i].done()){
          leds[i].setTime(fadeTime * 2, true);
          if(leds[i].get()) leds[i].off();
          else  leds[i].on();
          }
        }
        //Save which way we fade
        wasFadingUp = leds[0].rising();
    }

      //Once the buttons became released, we stop the fading at the current brightness
      if(sw.rose() and !swShort){
        Serial.println("sw fell long");
        for(byte i = 0; i < elements(leds); i++) leds[i].stop();  
      }
    
  if(millis() - millisLast > onTime) {
    allOff(1);
    manual = false;
  }

}
