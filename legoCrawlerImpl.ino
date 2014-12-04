#include <mcp_can.h>
#include <SPI.h>

/*BEGIN Arduino Pins*/
//LEDs
#define BLINK_FRONT_L_PIN 23
#define BLINK_FRONT_R_PIN 25
#define LIGHT_LOW_L_PIN 48
#define LIGHT_LOW_R_PIN 46
#define LIGHT_HIGH_L_PIN 44
#define LIGHT_HIGH_R_PIN 42
#define AMBIENTE_PIN 40
#define ROOF_L_PIN 38
#define ROOF_R_PIN 36
#define BACKDRIVE_PIN 34
#define LIGHT_REAR_L_PIN 32
#define LIGHT_REAR_R_PIN 30
#define BREAK_L_PIN 28
#define BREAK_R_PIN 26
#define BLINK_REAR_L_PIN 24
#define BLINK_REAR_R_PIN 22
//Engine Commands
#define BLUE_ENGINE_F_PIN 47
#define BLUE_ENGINE_B_PIN 49
#define RED_ENGINE_F_PIN 35
#define RED_ENGINE_B_PIN 41
//Photodiode
#define PHOTO_1M_RES_PIN A14
#define PHOTO_1M_RES_PIN A13
/*END Arduino Pins*/

/*BEGIN CAN Messages IDs*/
#define AMBIENT_LIGHT_CAN 0x124
#define BEAM_LIGHT_CAN 0x121
//#define BEAM_LIGHT_CAN 0x101
#define INDICATOR_LIGHT_CAN 0x120
//#define INDICATOR_LIGHT_CAN 0x100
#define POWERTRAIN_BRAKE_CAN 0x230
//#define POWERTRAIN_BRAKE_CAN 0x300
#define POWERTRAIN_CONTROL_CAN 0x221
#define SYSTEM_BRIGHTNESS_CAN 0x421
#define SYSTEM_STATUS_LED_CAN 0x420

#define BACK_LIGHT_CAN 0x102
#define BRAKE_STATUS_CAN 0x300
#define GEARBOX_CAN 0x200
/*END CAN Messages IDs*/

//LED Status Bit Position
#define BLINK_FRONT_L 0
#define BLINK_FRONT_R 5
#define LIGHT_LOW_L 1
#define LIGHT_LOW_R 4
#define LIGHT_HIGH_L 2
#define LIGHT_HIGH_R 3
#define AMBIENTE 13
#define ROOF_L 14
#define ROOF_R 15
#define BACKDRIVE 9
#define LIGHT_REAR_L 7
#define LIGHT_REAR_R 11
#define BREAK_L 8
#define BREAK_R 10
#define BLINK_REAR_L 6
#define BLINK_REAR_R 12

//
#define BLINK_INTERVAL 500
#define CANSEND_INTERVAL_LIGHTSTATUS 500
#define CANSEND_INTERVAL_BRIGHTNESS 1000
#define CANSEND_INTERVAL_ENGINECONTROL 50


MCP_CAN CAN(10); 

long previousBlinkMillis = 0;        // will store last time Blinker LED was updated
long prevCANSendLightMillis = 0;        // will store last time CAN Send was updated
long prevCANSendBrightMillis = 0;        // will store last time CAN Send was updated
long prevCANSendEngineMillis = 0;        // will store last time CAN Send was updated
int blinkIndicator = 0;

unsigned int photoDiode1M = 0;
unsigned int photoDiode2M = 0;

unsigned int ledStatus = 0;
unsigned int previousLedStatus = 0;

unsigned char canMsgEngineControl[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char canMsgPhotodiode[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char canMsgLedStatus[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char len = 0;
unsigned char buf[8];
char str[20];

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  //Input IR Commands
  pinMode(RED_ENGINE_F_PIN, INPUT);
  pinMode(RED_ENGINE_B_PIN, INPUT);
  pinMode(BLUE_ENGINE_F_PIN, INPUT);
  pinMode(BLUE_ENGINE_B_PIN, INPUT);
  pinMode(51, INPUT);
  pinMode(53, INPUT);
  //Output LEDs
  pinMode(BLINK_REAR_R_PIN, OUTPUT);
  pinMode(BLINK_REAR_L_PIN, OUTPUT);
  pinMode(BREAK_R_PIN, OUTPUT);
  pinMode(BREAK_L_PIN, OUTPUT);
  pinMode(LIGHT_REAR_R_PIN, OUTPUT);
  pinMode(LIGHT_REAR_L_PIN, OUTPUT);
  pinMode(BACKDRIVE_PIN, OUTPUT);
  pinMode(ROOF_R_PIN, OUTPUT);
  pinMode(ROOF_L_PIN, OUTPUT);
  pinMode(AMBIENTE_PIN, OUTPUT);
  pinMode(LIGHT_HIGH_R_PIN, OUTPUT);
  pinMode(LIGHT_HIGH_L_PIN, OUTPUT);
  pinMode(LIGHT_LOW_R_PIN, OUTPUT);
  pinMode(LIGHT_LOW_L_PIN, OUTPUT);
  pinMode(BLINK_FRONT_R_PIN, OUTPUT);
  pinMode(BLINK_FRONT_L_PIN, OUTPUT);
  
  initLEDStatus();

  START_INIT:

  if(CAN_OK == CAN.begin(CAN_125KBPS))                   // init can bus : baudrate = 500k
  {
      Serial.println("CAN BUS Shield init ok!");
  }
  else
  {
      Serial.println("CAN BUS Shield init fail");
      Serial.println("Init CAN BUS Shield again");
      delay(100);
      goto START_INIT;
  }

}

void initLEDStatus(){
  ledStatus = 1;
  for(int i=1;i<16;i++){
    writeLedStatus();
    delay(100);
    ledStatus=ledStatus<<1;
  }
  for(int i=1;i<16;i++){
    writeLedStatus();
    delay(100);
    ledStatus=ledStatus>>1;
  }
  ledStatus = 0xFFFF;
  writeLedStatus();
  delay(2000);
  ledStatus = 0;
  writeLedStatus();
}

void writeLedStatus(){
  //check if differs led status from last time
  if(ledStatus != previousLedStatus){
    //write led status to pinouts
    digitalWrite(BLINK_FRONT_L_PIN, (ledStatus&(1<<BLINK_FRONT_L))>>BLINK_FRONT_L);
    digitalWrite(LIGHT_LOW_L_PIN, (ledStatus&(1<<LIGHT_LOW_L))>>LIGHT_LOW_L);
    digitalWrite(LIGHT_HIGH_L_PIN, (ledStatus&(1<<LIGHT_HIGH_L))>>LIGHT_HIGH_L);
    digitalWrite(LIGHT_HIGH_R_PIN, (ledStatus&(1<<LIGHT_HIGH_R))>>LIGHT_HIGH_R);
    digitalWrite(LIGHT_LOW_R_PIN, (ledStatus&(1<<LIGHT_LOW_R))>>LIGHT_LOW_R);
    digitalWrite(BLINK_FRONT_R_PIN, (ledStatus&(1<<BLINK_FRONT_R))>>BLINK_FRONT_R);
    digitalWrite(BLINK_REAR_L_PIN, (ledStatus&(1<<BLINK_REAR_L))>>BLINK_REAR_L);
    digitalWrite(LIGHT_REAR_L_PIN, (ledStatus&(1<<LIGHT_REAR_L))>>LIGHT_REAR_L);
    digitalWrite(BREAK_L_PIN, (ledStatus&(1<<BREAK_L))>>BREAK_L);
    digitalWrite(BACKDRIVE_PIN, (ledStatus&(1<<BACKDRIVE))>>BACKDRIVE);
    digitalWrite(BREAK_R_PIN, (ledStatus&(1<<BREAK_R))>>BREAK_R);
    digitalWrite(LIGHT_REAR_R_PIN, (ledStatus&(1<<LIGHT_REAR_R))>>LIGHT_REAR_R);
    digitalWrite(BLINK_REAR_R_PIN, (ledStatus&(1<<BLINK_REAR_R))>>BLINK_REAR_R);
    digitalWrite(AMBIENTE_PIN, (ledStatus&(1<<AMBIENTE))>>AMBIENTE);
    digitalWrite(ROOF_L_PIN, (ledStatus&(1<<ROOF_L))>>ROOF_L);
    digitalWrite(ROOF_R_PIN, (ledStatus&(1<<ROOF_R))>>ROOF_R);
    //remember writed led status
    previousLedStatus = ledStatus;
  }
}

void toggleBlink(){
  // Serial.println("toggleBlink");
  // Serial.println(blinkIndicator);
  // Serial.println(ledStatus);
  if(blinkIndicator == 1){
    //left blink on?
    if((ledStatus&(1<<BLINK_FRONT_L))>>BLINK_FRONT_L == 1 || (ledStatus&(1<<BLINK_REAR_L))>>BLINK_REAR_L == 1){
      //left off
      ledStatus &= ~(1<<BLINK_FRONT_L);
      ledStatus &= ~(1<<BLINK_REAR_L);
    }else{
      //left on
      ledStatus |= 1<<BLINK_FRONT_L;
      ledStatus |= 1<<BLINK_REAR_L;
    }
    ledStatus &= ~(1<<BLINK_FRONT_R);
    ledStatus &= ~(1<<BLINK_REAR_R);
  }else if(blinkIndicator == 2){
    //right blink on?
    if((ledStatus&(1<<BLINK_FRONT_R))>>BLINK_FRONT_R == 1 || (ledStatus&(1<<BLINK_REAR_R))>>BLINK_REAR_R == 1){
      //right off
      ledStatus &= ~(1<<BLINK_FRONT_R);
      ledStatus &= ~(1<<BLINK_REAR_R);
    }else{
      //right on
      ledStatus |= 1<<BLINK_FRONT_R;
      ledStatus |= 1<<BLINK_REAR_R;
    }
    ledStatus &= ~(1<<BLINK_FRONT_L);
    ledStatus &= ~(1<<BLINK_REAR_L);
  }else if(blinkIndicator == 3 || blinkIndicator == 4){
    //both blink on?
    if((ledStatus&(1<<BLINK_FRONT_R))>>BLINK_FRONT_R == 1 || (ledStatus&(1<<BLINK_REAR_R))>>BLINK_REAR_R == 1
      || (ledStatus&(1<<BLINK_FRONT_L))>>BLINK_FRONT_L == 1 || (ledStatus&(1<<BLINK_REAR_L))>>BLINK_REAR_L == 1){
      //both off
      ledStatus &= ~(1<<BLINK_FRONT_R);
      ledStatus &= ~(1<<BLINK_REAR_R);
      ledStatus &= ~(1<<BLINK_FRONT_L);
      ledStatus &= ~(1<<BLINK_REAR_L);
    }else{
      //both on
      ledStatus |= 1<<BLINK_FRONT_R;
      ledStatus |= 1<<BLINK_REAR_R;
      ledStatus |= 1<<BLINK_FRONT_L;
      ledStatus |= 1<<BLINK_REAR_L;
    }
  }else if(blinkIndicator == 0){
    //both off
    ledStatus &= ~(1<<BLINK_FRONT_R);
    ledStatus &= ~(1<<BLINK_REAR_R);
    ledStatus &= ~(1<<BLINK_FRONT_L);
    ledStatus &= ~(1<<BLINK_REAR_L);    
  }
  writeLedStatus();
}

void blinkLoop(){
  unsigned long currentBlinkMillis = millis();
 
  if(currentBlinkMillis - previousBlinkMillis > BLINK_INTERVAL) {
    // save the last time you blinked the LED 
    previousBlinkMillis = currentBlinkMillis;   

    toggleBlink();
  }
}

void canSendLightLoop(){
  unsigned long currentCANSendMillis = millis();
 
  if(currentCANSendMillis - prevCANSendLightMillis > CANSEND_INTERVAL_LIGHTSTATUS) {
    // save the last time you blinked the LED 
    prevCANSendLightMillis = currentCANSendMillis;   

    sendCANLightData();
  }
}

void canSendBrigthnessLoop(){
  unsigned long currentCANSendMillis = millis();
 
  if(currentCANSendMillis - prevCANSendBrightMillis > CANSEND_INTERVAL_BRIGHTNESS) {
    // save the last time you blinked the LED 
    prevCANSendBrightMillis = currentCANSendMillis;   

    sendCANBrightnessData();
  }
}

void canSendEngineLoop(){
  unsigned long currentCANSendMillis = millis();
 
  if(currentCANSendMillis - prevCANSendEngineMillis > CANSEND_INTERVAL_ENGINECONTROL) {
    // save the last time you blinked the LED 
    prevCANSendEngineMillis = currentCANSendMillis;   

    sendCANEngineData();
  }
}

void sendCANLightData(){
  //send light status
  canMsgLedStatus[0] = ledStatus;
  canMsgLedStatus[1] = ledStatus>>8;
  CAN.sendMsgBuf(SYSTEM_STATUS_LED_CAN, 0,  2, canMsgLedStatus);
}

void sendCANBrightnessData(){
  //read photodiode values and send it through can
  photoDiode2M = analogRead(PHOTO_1M_RES_PIN);
  photoDiode1M = analogRead(PHOTO_1M_RES_PIN);
  
  canMsgPhotodiode[0] = (photoDiode1M*255)/1023; //norm from 10bit to 8bit resolution
  CAN.sendMsgBuf(0x421, 0,  1, canMsgPhotodiode);
}

void sendCANEngineData(){
  //Read Engine Direction Values  
  unsigned char engineBlueDirection = 0;
  unsigned char engineRedDirection = 0;
  //red engine
  if(digitalRead(RED_ENGINE_F_PIN) == HIGH){
    engineRedDirection = 2; //forward
  }else if(digitalRead(RED_ENGINE_B_PIN) == HIGH){
    engineRedDirection = 1; //backward
  }
  // Serial.print("Red: ");
  // Serial.println(engineRedDirection);
  //blue engine
  if(digitalRead(BLUE_ENGINE_F_PIN) == HIGH){
    engineBlueDirection = 2;  //forward
  }else if(digitalRead(BLUE_ENGINE_B_PIN) == HIGH){
    engineBlueDirection = 1;//backward
  }
  // Serial.print("Blue: ");
  // Serial.println(engineBlueDirection);
  //build and send can msg for engine status
  canMsgEngineControl[0] = (engineRedDirection | (engineBlueDirection<<2));
  
  CAN.sendMsgBuf(POWERTRAIN_CONTROL_CAN, 0,  2, canMsgEngineControl);
}

void lockBlink(){
  //both on
  ledStatus |= 1<<BLINK_FRONT_R;
  ledStatus |= 1<<BLINK_REAR_R;
  ledStatus |= 1<<BLINK_FRONT_L;
  ledStatus |= 1<<BLINK_REAR_L;
  writeLedStatus();
  delay(200);
  //both off
  ledStatus &= ~(1<<BLINK_FRONT_R);
  ledStatus &= ~(1<<BLINK_REAR_R);
  ledStatus &= ~(1<<BLINK_FRONT_L);
  ledStatus &= ~(1<<BLINK_REAR_L);
  writeLedStatus();
}

void welcomeBlink(){
  //both on
  ledStatus |= 1<<BLINK_FRONT_R;
  ledStatus |= 1<<BLINK_REAR_R;
  ledStatus |= 1<<BLINK_FRONT_L;
  ledStatus |= 1<<BLINK_REAR_L;
  writeLedStatus();
  delay(200);
  //both off
  ledStatus &= ~(1<<BLINK_FRONT_R);
  ledStatus &= ~(1<<BLINK_REAR_R);
  ledStatus &= ~(1<<BLINK_FRONT_L);
  ledStatus &= ~(1<<BLINK_REAR_L);
  writeLedStatus();
  delay(200);
  //both on
  ledStatus |= 1<<BLINK_FRONT_R;
  ledStatus |= 1<<BLINK_REAR_R;
  ledStatus |= 1<<BLINK_FRONT_L;
  ledStatus |= 1<<BLINK_REAR_L;
  writeLedStatus();
  delay(200);
  //both off
  ledStatus &= ~(1<<BLINK_FRONT_R);
  ledStatus &= ~(1<<BLINK_REAR_R);
  ledStatus &= ~(1<<BLINK_FRONT_L);
  ledStatus &= ~(1<<BLINK_REAR_L);
  writeLedStatus();
}

void farewellLights(){
  ledStatus |= 1<<LIGHT_LOW_L;
  ledStatus |= 1<<LIGHT_LOW_R;
  writeLedStatus();
  delay(2000);
  ledStatus &= ~(1<<LIGHT_LOW_L);
  ledStatus &= ~(1<<LIGHT_LOW_R);
  writeLedStatus();
}


void knightRiderBlink(){
  for(int i=0;i<2;i++){
    ledStatus |= 1<<BLINK_FRONT_R;
    writeLedStatus();
    delay(100);
    ledStatus |= 1<<LIGHT_LOW_R;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<BLINK_FRONT_R);
    ledStatus |= 1<<LIGHT_HIGH_R;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_LOW_R);
    ledStatus |= 1<<LIGHT_HIGH_L;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_HIGH_R);
    ledStatus |= 1<<LIGHT_LOW_L;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_LOW_L);
    ledStatus |= 1<<BLINK_FRONT_L;
    writeLedStatus();
    delay(100);
    ledStatus |= 1<<LIGHT_LOW_L;
    writeLedStatus();
    delay(200);
    ledStatus &= ~(1<<BLINK_FRONT_L);
    ledStatus |= 1<<LIGHT_HIGH_L;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_LOW_L);
    ledStatus |= 1<<LIGHT_HIGH_R;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_HIGH_L);
    ledStatus |= 1<<LIGHT_LOW_R;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_HIGH_R);
    ledStatus |= 1<<BLINK_FRONT_R;
    writeLedStatus();
    delay(100);
    ledStatus &= ~(1<<LIGHT_LOW_R);
    writeLedStatus();
    delay(100);   
  }
}

// the loop routine runs over and over again forever:
void loop() {
  
  //check blink loop
  blinkLoop();

  //can send loops
  canSendLightLoop();
  canSendBrigthnessLoop();
  canSendEngineLoop();

  //check if can data available
  if(CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    switch(CAN.getCanId()){
      //Crawler_frmBeamLight MSG
      case BEAM_LIGHT_CAN:
        if(len >= 1){

          //flash
          if(buf[0]>>2 == 1){
            ledStatus |= 1<<LIGHT_HIGH_L;
            ledStatus |= 1<<LIGHT_HIGH_R;
            ledStatus |= 1<<ROOF_L;
            ledStatus |= 1<<ROOF_R;           
          }else if(buf[0]>>2 == 0){
            ledStatus &= ~(1<<LIGHT_HIGH_L);
            ledStatus &= ~(1<<LIGHT_HIGH_R);
            ledStatus &= ~(1<<ROOF_L);
            ledStatus &= ~(1<<ROOF_R);          
          }

          //normal light off/park/low/high
          if(buf[0] == 0){ //park on
            //all beam lights off
            ledStatus &= ~(1<<LIGHT_LOW_L);
            ledStatus &= ~(1<<LIGHT_LOW_R);
            ledStatus &= ~(1<<LIGHT_REAR_L);
            ledStatus &= ~(1<<LIGHT_REAR_R);
            ledStatus &= ~(1<<LIGHT_HIGH_L);
            ledStatus &= ~(1<<LIGHT_HIGH_R);
          }else if(buf[0] == 1){ //park on
            ledStatus |= 1<<LIGHT_LOW_L;
            ledStatus |= 1<<LIGHT_REAR_L;
          }else if(buf[0] == 2){ //low beam on
            ledStatus |= 1<<LIGHT_LOW_L;
            ledStatus |= 1<<LIGHT_LOW_R;
            ledStatus |= 1<<LIGHT_REAR_L;
            ledStatus |= 1<<LIGHT_REAR_R;
          }else if(buf[0] == 3){ //high beam on
            ledStatus |= 1<<LIGHT_LOW_L;
            ledStatus |= 1<<LIGHT_LOW_R;
            ledStatus |= 1<<LIGHT_REAR_L;
            ledStatus |= 1<<LIGHT_REAR_R;
            ledStatus |= 1<<LIGHT_HIGH_L;
            ledStatus |= 1<<LIGHT_HIGH_R;
          }

          writeLedStatus();
        }
        break;
      case INDICATOR_LIGHT_CAN:
        if(len >= 1){
          blinkIndicator = buf[0];
        }
        break;
      case POWERTRAIN_BRAKE_CAN:
        if(len >= 1){
          /* Break on pressure indicator
          if(buf[0] == 0){ //break off
            ledStatus &= ~(1<<BREAK_R);
            ledStatus &= ~(1<<BREAK_L);
          }else if(buf[0] > 0 && buf[0] < 255){ //break on
            ledStatus |= 1<<BREAK_R;
            ledStatus |= 1<<BREAK_L;
          }else if(buf[0] == 255){ //break on + all blinker
            ledStatus |= 1<<BREAK_R;
            ledStatus |= 1<<BREAK_L;
            //All blink on
            blinkIndicator = 3;
          }
          */
          /* break on signal idicator*/
          if(buf[1]&1 == 1){
            //emergency
            ledStatus |= 1<<BREAK_R;
            ledStatus |= 1<<BREAK_L;
            //All blink on
            blinkIndicator = 4;
          }else{
            //emergency
            ledStatus &= ~(1<<BREAK_R);
            ledStatus &= ~(1<<BREAK_L);
            //All blink off
            if(blinkIndicator == 4){
              blinkIndicator = 0;
            }      
          }
          if((buf[1]>>1)&1 == 1){
            //break on/off
            ledStatus |= 1<<BREAK_R;
            ledStatus |= 1<<BREAK_L;
          }else{
            ledStatus &= ~(1<<BREAK_R);
            ledStatus &= ~(1<<BREAK_L);    
          }
          writeLedStatus();
        }
        break;
      case AMBIENT_LIGHT_CAN:
        if(len >= 1){
          //Ambient light
          if(buf[0]&1 == 1){
            ledStatus |= 1<<AMBIENTE;
          }else{
            ledStatus &= ~(1<<AMBIENTE);
          }
          //farewell
          if((buf[0]>>1)&1 == 1){
            farewellLights();
          }
          //knight rider
          if((buf[0]>>2)&1 == 1){
            knightRiderBlink();
          }
          //welcome
          if((buf[0]>>3)&1 == 1){
            welcomeBlink();
          }
          //head lights
          if((buf[0]>>4)&1 == 1){
            ledStatus |= 1<<ROOF_L;
            ledStatus |= 1<<ROOF_R;
          }else{
            ledStatus &= ~(1<<ROOF_L);
            ledStatus &= ~(1<<ROOF_R);
          }
          //lock blink
          if((buf[0]>>5)&1 == 1){
            lockBlink();
          }
          writeLedStatus();
        }
        break;
      case GEARBOX_CAN:
        if(len >= 1){
          if(buf[0] == 5){
            ledStatus |= 1<<BACKDRIVE;
          }else{
            ledStatus &= ~(1<<BACKDRIVE);
          }
        }
        break;
      default:
        break;
    }
    delay(50); // delay in between reads for stability
  }
}

