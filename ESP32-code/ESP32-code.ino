#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

//pin configuration
const uint8_t I2S_SCK = 5;       // Audio data bit clock 
const uint8_t I2S_WS = 25;       // Audio data left and right clock
const uint8_t I2S_SDOUT = 26;    // ESP32 audio data output to DAC
const uint8_t prevbutton = 13;   //self explainatory
const uint8_t nextbutton = 14;
const uint8_t playbutton = 12;
const uint8_t autobutton = 27;   //Latching button that enables autoplay on reconnect
volatile byte playpause = LOW;   //Volatile byte values for the ISRs
volatile byte prev = LOW;
volatile byte next = LOW;

I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
    Serial.begin(115200);                   //Start serial connection for debugging purposes
    pinMode(prevbutton, INPUT_PULLUP);      //Setting pinModes
    pinMode(nextbutton, INPUT_PULLUP);
    pinMode(playbutton, INPUT_PULLUP);
    pinMode(autobutton, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(prevbutton), ISR0, FALLING); //Starting interrupts for specified pins  
    attachInterrupt(digitalPinToInterrupt(playbutton), ISR1, FALLING);
    attachInterrupt(digitalPinToInterrupt(nextbutton), ISR2, FALLING);
    
    //Configuring i2s
    Serial.println("starting I2S-ADC...");
    i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT); 
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
      Serial.println("Failed to initialize I2S!");
      while (1);
    }
    a2dp_sink.set_auto_reconnect(true);     //enable auto reconnect. Probably not neccecary as I'm using a reconnect loop
    a2dp_sink.start("Music thingy :3");     //start a2dp_sink and specify device name
}

//Defining the ISRs for each button
//I hate the way I did the debounce
void ISR0() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500) 
  {
    prev = HIGH;
  }
  last_interrupt_time = interrupt_time;
  }
void ISR1() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 500) 
  {
    playpause = HIGH;
  }
  last_interrupt_time = interrupt_time;
  }
void ISR2() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 500) 
  {
    next = HIGH;
  }
  last_interrupt_time = interrupt_time;
  }

void loop() {
  //Reconnect if disconnected
  if(a2dp_sink.is_connected() == false){
    while(a2dp_sink.is_connected() == false){
      Serial.println("Attempting to reconnect with last device");
      a2dp_sink.reconnect();
      delay(1000);
      Serial.println("waiting for connection...");
    }
    //Autoplay if autoplay-button is latched on
    if((digitalRead(autobutton) == 0) ) {
      Serial.println("Play on reconnect after delay");
      delay(5000);                                      //Generous delay as buffer, otherwise play command is sent before fully connected. Could probably be lowered down all the way to a second
      a2dp_sink.play();
      Serial.println("Play!");
    }
  }

  //Button actions from the ISRs
  if(prev == HIGH) {
    Serial.println("Previous");
    a2dp_sink.previous();
    prev = LOW;
  }
  if(next == HIGH) {
    Serial.println("Next");
    a2dp_sink.next();
    next = LOW;
  } 
  if(playpause == HIGH){
    if(a2dp_sink.get_audio_state() == 0) {    //Check if audio is playing, if not button functionality is play
      Serial.println("Play");
      a2dp_sink.play();
    }
    else{                                     //If audio is playing (Otherwise), button functionality is pause
      Serial.println("Pause");
      a2dp_sink.pause();
    }
    playpause = LOW;
  }

  //Causes audio issues if not used. Probably CPU usage 
  delay(100);
}