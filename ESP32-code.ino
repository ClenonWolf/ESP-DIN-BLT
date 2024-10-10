#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

//defining pins
const uint8_t I2S_SCK = 5;       // Audio data bit clock 
const uint8_t I2S_WS = 25;       // Audio data left and right clock
const uint8_t I2S_SDOUT = 26;    // ESP32 audio data output to DAC
const uint8_t prevbutton = 13;   //self explainatory
const uint8_t nextbutton = 14;
const uint8_t playbutton = 12;
const uint8_t autobutton = 27;   //Latching button that enables autoplay on reconnect
int8_t IntStat = 0;
int pbuttonstate = 0;            //Play button state for state change detection / edge detection
int lastpbuttonstate = 0;        //last state for edge detection

I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
    Serial.begin(115200);                   //Start serial connection for debugging purposes
    pinMode(prevbutton, INPUT_PULLUP);      //there's probably a more elegant way of setting multiple pins
    pinMode(nextbutton, INPUT_PULLUP);
    pinMode(playbutton, INPUT_PULLUP);
    pinMode(autobutton, INPUT_PULLUP);

    Serial.println("starting I2S-ADC...");
    i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT); //configuring i2s
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
      Serial.println("Failed to initialize I2S!");
      while (1); // do nothing
    }
    a2dp_sink.set_auto_reconnect(true);     //enable auto reconnect. Probably not neccecary as I'm using a reconnect loop
    a2dp_sink.start("Music thingy :3");     //start a2dp_sink and specify device name
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

  //The following code could be put into a while(a2dp_sink.is_connected() == true) loop. But not really needed. 
  //Didn't implement edge detection for prev/next buttens for simplicity reasons
  //Not strictly neccesary as it doesn't cause noticable issues
  if(digitalRead(prevbutton) == 0) {
    a2dp_sink.previous();
  }
  if(digitalRead(nextbutton) == 0) {
    a2dp_sink.next();
  }

  //Play button has edge detection as the same button is used for play/pause
  pbuttonstate = digitalRead(playbutton);
  if(pbuttonstate != lastpbuttonstate){ 
    if(pbuttonstate == HIGH){
      if(a2dp_sink.get_audio_state() == 0) {    //Check if audio is playing, if not button functionality is play. Check takes kinda long for a button press, maybe improve (button needs to be pressed kinda long)
        a2dp_sink.play();
      }
      else{                                     //If audio is playing (Otherwise), button functionality is pause
        a2dp_sink.pause();
      }
    }
  }
  lastpbuttonstate = pbuttonstate;

  //Delay as a sort of debounce. Also causes audio issues if not used which is probably caused by the reconnect loop 
  delay(20);
}
