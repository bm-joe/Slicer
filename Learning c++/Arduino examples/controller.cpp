#include <Servo.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal.h>
#include <string.h>
  //Display Setup
  LiquidCrystal lcd(5, 6, 35, 33, 31, 3);

// Controller Stuff
const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};

unsigned long curMillis;

unsigned long prevReplyToPCmillis = 0;
unsigned long replyToPCinterval = 1000;

float movingStatus = 0.0;
float turningStatus =  0.0;
int magnetStatus = 0;
float heightStatus = 350.0;


//=============

void getDataFromPC() {

    // receive data from PC and save it into inputBuffer
    
  if(Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant
      
    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      strcpy(messageFromPC, inputBuffer);
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
  }
}

//=============

void replyToPC() {

  if (newDataFromPC) {
    newDataFromPC = false;
    Serial.print("<");
    Serial.print(messageFromPC);
    Serial.print(" [Time: ");
    Serial.print(curMillis >> 9); // divide by 512 is approx = half-seconds
    Serial.println("]>");
  }
}

//vector class
class Vector2{
  private:
    float len;
    float angle;
  public:
    float x;
    float y;
  Vector2(float xVal, float yVal) : x(xVal), y(yVal){
    //getting length
    len = sqrt(pow(x, 2.0) + pow(y, 2.0));
  }
  float getLength(){
    len = sqrt(pow(x, 2.0) + pow(y, 2.0));
    return len;
  }
  float getAngle(){
    angle = atan2(y,x);
    angle = angle * (180.0/PI);
    return angle;
  }
};

//distance of top segment - 123.502 mm
// distance of middle segment - 104.291 mm
//distance of foot segment - 148.5 mm

//FINAL VARIABLES (don't change these)
  bool startup;

  //segment size
  #define t 123.502
  #define m 104.291
  #define b 148.5

  //servo information
  #define SERVOMIN  150 // This is the 'minimum' pulse length count (out of 4096)
  #define SERVOMAX  600 // This is the 'maximum' pulse length count (out of 4096)
  #define USMIN  500 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
  #define USMAX  1000 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
  #define SERVO_FREQ 60 // Analog servos run at ~60 Hz updates

  // servoshield object
  Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


  float tickSpeed;
  bool raising = false;
  float startingPos[3] = {-180, 200, 0};
  float kneelingPos[3] = {-180, 200, 0};
  unsigned long time;
  //tick is a variable that goes gradually to tickspeed and then resets. acts as the x axis for the motion equations
  double tick;
  //what phase of the step is happening
  bool phase1 = true;

//CHANGEABLE VARIABLES / SETTINGS:
  //height of the robot. will stop walking automatically if the height is too low. measured in milimeters from (around) the very top servos 
  float height = heightStatus;
  //the speed of the walking and turning. Measured in miliseconds/walk cycle
  float originalTickSpeed = 400.0;
  //dont change the x value of this because lateral doesn't work... only change the y
  Vector2 targetVector(/*Don't change this*/0.0,/*-1.0 = backwards, 1.0 = forwards:*/ movingStatus);
  //-1  = right, 1 = left
  int turn = turningStatus;

//Controller Functions
void processData() {
  String x = messageFromPC;
  if (x == "magnetOn") {
    magnetStatus = 1;}
  else if (x == "magnetOff") {
    magnetStatus = 0;}
  else if (x == "moveForward") {
    movingStatus = 1;
    targetVector.y = 1.0;
    }
  else if (x == "moveReverse") {
    movingStatus = -1;
    targetVector.y = -1.0;
  }
  else if (x == "turnLeft") {
    turn = 1.0;
    targetVector.y = 1.0;
    turningStatus = 1;}
  else if (x == "turnRight") {
    targetVector.y = 1.0;
    turningStatus = -1;
    turn = -1.0;
  }
  else if (x == "stop") {
    movingStatus = 0;
    turningStatus = 0;
    turn = 0.0;
    targetVector.y = 0.0;
    }
  else if (x == "high") {
    if( height < 350.0){
      raising = true;
    }
}
  else if (x == "low") {
    height = 200.0;
    }
  if (raising && height < 350.0){
    height += 1.0;
  }else if(raising){
    raising = false;
  }
  lcd.clear();
  lcd.print(x);
}

// leg classes
class legWithoutShield{
  private:
    //distance
    float d;
    //theta offsets
    float thetaX;
    float thetaZ;
    //servo angles
    float T;
    float M;
    float B;
    //servo objects
    Servo topServo;
    Servo bottomServo;
    Servo middleServo;
    //pins
    int topPIN;
    int bottomPIN;
    int middlePIN;
    //wanted coordinates
    float x;
    float y;
    float z;
  public:
  //constructor
  //int top pin, int middle pin, int bottom pin
  legWithoutShield(int TP, int MP, int BP) : topPIN(TP), middlePIN(MP), bottomPIN(BP) {
    //attaching
  }
  void attach(){
    topServo.attach(topPIN);
    middleServo.attach(middlePIN);
    bottomServo.attach(bottomPIN);
  }
    void setTarget(float X, float Y, float Z){
    x = X;
    y = Y;
    z = Z;
  }
  void goToTarget(){
    d = sqrt(pow(x,2.0) + pow(y,2.0) + pow(z, 2.0));
    d-= t;

    thetaX = atan2(abs(y),x);
    thetaZ = atan2(z,abs(x));

    thetaX = thetaX * 180.0 / PI;
    thetaZ = thetaZ * 180.0 / PI;

    M = acos( (pow(b,2.0) - pow(d,2.0) - pow(m, 2.0) ) / (-2.0*d*m) );
    B = acos( (pow(d,2.0) - pow(b,2.0) - pow(m, 2.0) ) / (-2.0*b*m) );

    B = B * 180.0 / PI;
    M = M * 180.0 / PI;

 

    middleServo.write(M+thetaX-90);
    bottomServo.write(B-90);
    topServo.write(thetaZ +90);
  }
  void goToPolar(float X, float Y, float Z_ANGLE){
    x = X;
    y = Y;
    z = Z_ANGLE;
    d = sqrt(pow(x, 2.0) + pow(y, 2.0));
    d-=t;

    thetaX = atan2(abs(y),x);
    thetaX = thetaX * 180.0 / PI;

        M = acos( (pow(b,2.0) - pow(d,2.0) - pow(m, 2.0) ) / (-2.0*d*m) );
    B = acos( (pow(d,2.0) - pow(b,2.0) - pow(m, 2.0) ) / (-2.0*b*m) );

    B = B * 180.0 / PI;
    M = M * 180.0 / PI;

 

    middleServo.write(M+thetaX-90);
    bottomServo.write(B-90);
    topServo.write(z +90);


  }
};

class legWithShield{
  private:
    //distance
    float d;
    //theta offsets
    float thetaX;
    float thetaZ;
    //servo angles
    float T;
    float M;
    float B;
    //pins
    int topPIN;
    int bottomPIN;
    int middlePIN;
    //wanted coordinates
    float x;
    float y;
    float z;
  public:
  //constructor
  //int top pin, int middle pin, int bottom pin
  legWithShield(int TP, int MP, int BP) : topPIN(TP), middlePIN(MP), bottomPIN(BP) {
  }
  void setTarget(float X, float Y, float Z){
    x = X;
    y = Y;
    z = Z;
  }
  void goToTarget(){
    
    d = sqrt(pow(x,2.0) + pow(y,2.0) + pow(z, 2.0));
    d-= t;

    thetaX = atan2(abs(y),x);
    thetaZ = atan2(z,abs(x));

    thetaX = thetaX * 180.0 / PI;
    thetaZ = thetaZ * 180.0 / PI;

    M = acos( (pow(b,2.0) - pow(d,2.0) - pow(m, 2.0) ) / (-2.0*d*m) );
    B = acos( (pow(d,2.0) - pow(b,2.0) - pow(m, 2.0) ) / (-2.0*b*m) );

    B = B * 180.0 / PI;
    M = M * 180.0 / PI;


    pwm.setPWM(topPIN, 0, map((thetaZ +90), 0, 179, SERVOMIN-1, SERVOMAX-1));
    pwm.setPWM(middlePIN, 0, map((M+thetaX-90), 0, 179, SERVOMIN-1, SERVOMAX-1));
   
   pwm.setPWM(bottomPIN, 0, map((B-90), 0, 179, SERVOMIN-1, SERVOMAX-1));
  }
  void goToPolar(float X, float Y, float Z_ANGLE){
    x = X;
    y = Y;
    z = Z_ANGLE;
    d = sqrt(pow(x, 2.0) + pow(y, 2.0));
    d-=t;

    thetaX = atan2(abs(y),x);
    thetaX = thetaX * 180.0 / PI;

        M = acos( (pow(b,2.0) - pow(d,2.0) - pow(m, 2.0) ) / (-2.0*d*m) );
    B = acos( (pow(d,2.0) - pow(b,2.0) - pow(m, 2.0) ) / (-2.0*b*m) );

    B = B * 180.0 / PI;
    M = M * 180.0 / PI;

 

    pwm.setPWM(topPIN, 0, map((z +90), 0, 179, SERVOMIN-1, SERVOMAX-1));
    pwm.setPWM(middlePIN, 0, map((M+thetaX-90), 0, 179, SERVOMIN-1, SERVOMAX-1));
    pwm.setPWM(bottomPIN, 0, map((B-90), 0, 179, SERVOMIN-1, SERVOMAX-1));


  }

};

  //legs
  /*TRIPOD GROUPS:
  GROUP 1:
  PERPENDICULAR RIGHT = LEG 6
  TOP LEFT = LEG 4
  BOTTOM LEFT = LEG 2

  GROUP 2:
  PERPENDICULAR LEFT = LEG 3
  TOP RIGHT = LEG 5
  BOTTOM LEFT = LEG 1*/
  legWithoutShield leg1(13, 12, 11);
  legWithShield leg5(3, 2, 1); 
  legWithShield leg6(6, 5, 4);
  legWithShield leg4(15, 14, 13); 
  legWithShield leg3(12, 11, 10);
  legWithShield leg2(9, 8, 7);
  legWithShield sLegs[5] = {leg2, leg3, leg4, leg5, leg6};



void setup() {
  Serial.begin(9600);

  //starting lcd
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  lcd.begin(16, 2);
  lcd.print("Hello There");
  
  
  //starting pwm module
  pwm.begin();
  // pwm.setOscillatorFrequency(27500000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  //making new leg
  leg1.attach();

  startup = true;
  delay(10);
  // tell the PC we are ready
  Serial.println("<Arduino is ready>");
}

void loop() {
  time = millis();
  getDataFromPC();
  processData();
  analogWrite(8, magnetStatus*130);
  //making speed lower if turning. high speed turning will rip the freaking thing apart
  if (turn != 0){
    tickSpeed = originalTickSpeed *2.5;
  }else{
    tickSpeed = originalTickSpeed;
  }
  //startup motion
  if(startup){
    for (int i = 0; i < 5; i++){
      sLegs[i].setTarget(startingPos[0], startingPos[1], startingPos[2]);
      sLegs[i].goToTarget();
    }
    leg1.setTarget(startingPos[0], startingPos[1], startingPos[2]);
    leg1.goToTarget();
    if (startingPos[0] < -60){
      startingPos[0] += 0.25;
    }
    if (startingPos[1] < 350){
      startingPos[1] += 1;

    }
    else{
      startup = false; 
      
    }
  }
  else{
  //main movement code 
  if(tick <= tickSpeed && height <= 350.0 && height >=300.0){
    //SECTION 1: GROUP 1 STEPS, GROUP 2 RAISES


    //dont change these functions unless you are adding lateral motion.
    //x is fucked up for any targetvector.x value that isn't 0, which is why lateral motion doesn't work. This also involves z and other axes 
    float z;
    float midZ;
    float y;
    float x;
    float turningZ;
    if (phase1){
      y = ((20.0/pow((tickSpeed/2.0),2.0)) * pow((tick - (tickSpeed/2.0)),2.0)) + (height-20.0);
      x = (-1.0 * (60.0/ tickSpeed) * tick * targetVector.x) - 60.0;
      z = (30.0/tickSpeed) * tick * targetVector.y +25 - (15 * targetVector.y)  ;
      midZ = (40.0/tickSpeed) * tick * targetVector.y - (20.0 * targetVector.y);
      turningZ = (90/tickSpeed) * tick * turn - (45.0 * turn);
    }else{
      y = ((10.0/pow((tickSpeed/2.0),2.0) * -1.0) * pow((tick - (tickSpeed/2.0)),2.0)) + (height+10.0);
      x = ((60.0/ tickSpeed) * tick * targetVector.x) - 60.0 -  (60.0 * targetVector.x);
      z = (30.0/tickSpeed) * -1.0*  tick * targetVector.y +25 + (15 * targetVector.y);
      midZ = (40.0/tickSpeed) * -1.0* tick * targetVector.y + (20.0 * targetVector.y);
      turningZ = (90/tickSpeed) * tick * -1.0 * turn + (45.0 * turn);
    }
    float x1 = ( (60.0/ tickSpeed) * tick * targetVector.x) + 40.0;
    float x2 = sqrt(pow(x1 , 2.0) + pow(z,2.0)) * -1.0;
    //if walking
    if (turn == 0){
      leg6.setTarget(
        //x
        x,
        //y 
        y,
        //z
        midZ);
      leg6.goToTarget();
        leg4.setTarget(
          //x
          x2,
          //y
          y,
          //z
          sqrt(pow( x1 ,2.0) + pow(z,2.0)) * tan((35.0 * (PI/180.0)) - atan2(z, x1)) 
        );
      leg4.goToTarget();
      leg2.setTarget(
        //x
        x2,
        //y
        y,
        //z
        sqrt(pow( x1 ,2.0) + pow(z,2.0)) * tan(35.0* (PI/180.0) - atan2(z, x1)) 
      );
      leg2.goToTarget();
      // if turning
      }else{
        leg6.goToPolar(-60.0, y, turningZ);
        leg4.goToPolar(-60.0, y, turningZ-30);
        leg2.goToPolar(-60.0, y, turningZ+30);
      }
    
    //switching the functions for group 2
    if (phase1 == false){
      y = ((20.0/pow((tickSpeed/2.0),2.0)) * pow((tick - (tickSpeed/2.0)),2.0)) + (height-20.0);
      x = ((-1.0 * (60.0/ tickSpeed) * tick * targetVector.x) - 60.0);
      z = ((30.0/tickSpeed) * tick * targetVector.y) +25 - (15 * targetVector.y);
      midZ = (40.0/tickSpeed) * tick * targetVector.y - (20.0 * targetVector.y);
      turningZ = (90/tickSpeed) * tick * turn - (45.0 * turn);
    }else{ 
      y = ((10.0/pow((tickSpeed/2.0),2.0) * -1.0) * pow((tick - (tickSpeed/2.0)),2.0)) + (height+10.0);
      x = (((60.0/ tickSpeed) * tick * targetVector.x) - 60.0 - (60.0 * targetVector.x));
      z = ((30.0/tickSpeed) * -1.0*  tick * targetVector.y + 25.0 + (15.0 * targetVector.y))  ;
      midZ = (40.0/tickSpeed) * -1.0* tick * targetVector.y + (20.0 * targetVector.y);
      turningZ = (90/tickSpeed) * tick * -1.0 * turn + (45.0 * turn);
    }
    x1 = ( (60.0/ tickSpeed) * tick * targetVector.x) + 40.0;
    x2 = sqrt(pow(x1 , 2.0) + pow(z,2.0)) * -1.0;

    if (turn == 0){
    //group 2
    leg3.setTarget(
      //x
      x,
      //y 
      y,
      //z
      -1.0 * midZ);
    leg3.goToTarget();
      leg5.setTarget(
        //x
        x2,
        //y
        y,
        //z
        -1.0* sqrt(pow( x1 ,2.0) + pow(z,2.0)) * tan((35.0 * (PI/180.0)) - atan2(z, x1)) 
      );
      leg5.goToTarget();
    leg1.setTarget(
      //x
      x2,
      //y
      y,
      //z
      -1.0* sqrt(pow( x1 ,2.0) + pow(z,2.0)) * tan(35.0* (PI/180.0) - atan2(z, x1)) 
    );
    leg1.goToTarget();
    }else{
        leg3.goToPolar(-60.0, y, turningZ);
        leg5.goToPolar(-60.0, y, turningZ+30);
        leg1.goToPolar(-60.0, y, turningZ-30);
    }

  }else if (tick <=tickSpeed && height<= 300.0 && height >=200.0){
    //if height is too low to walk, stop walking and just kneel down.
    for (int i = 0; i < 5; i++){
      sLegs[i].setTarget(kneelingPos[0], height, kneelingPos[2]);
      sLegs[i].goToTarget();
    }
    leg1.setTarget(kneelingPos[0], height, kneelingPos[2]);
    leg1.goToTarget();

  }else if(height <=200.0){
    for (int i = 0; i < 5; i++){
      sLegs[i].setTarget(kneelingPos[0], 200, kneelingPos[2]);
      sLegs[i].goToTarget();
    }
    leg1.setTarget(kneelingPos[0], 200, kneelingPos[2]);
    leg1.goToTarget();
  }
  else if(targetVector.getLength() == 0.0){
      for (int i = 0; i < 5; i++){
      sLegs[i].setTarget(-60, height, 0);
      sLegs[i].goToTarget();
    }
    leg1.setTarget(-60, height, 0);
    leg1.goToTarget();

  }
  else{
    //reseting tick if its over tickspeed, inverting the walking phase
    tick =0.0;
    phase1 = !phase1;
  }
  //end
  tick += millis() - time;
  }
  replyToPC();
}
