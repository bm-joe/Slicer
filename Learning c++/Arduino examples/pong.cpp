#include <math.h>
#include <random>

class player{
  public:
    int xPos;
    int yPos;
    int size;
    player(int x, int y, int s) : xPos(x), yPos(y), size(s) {}
    void up(){
      if (yPos+size < 4){
        yPos += 1;
      }
    }    
    void down(){
      if (yPos > 0){
        yPos -= 1;
      }
    }
};

class ball{
  private:
    const float startingX;
    const float startingY;
    const float startingRise;
    const float startingRun;
  public:
    float xPos;
    float yPos;
    float rise;
    float run;
    int roundedX;
    int roundedY;
    ball(float x, float y, float ri, float ru) : startingX(x), startingY(y), startingRise(ri), startingRun(ru), xPos(x), yPos(y), rise(ri), run(ru) {}
    void updateMovement(player p1, player p2){
      if((xPos+run) > 12.0){
        if(yPos+rise > (p2.yPos + p2.size) || yPos+rise < p2.yPos ){
          reset();
        }
        else{
          run = (abs(run)*-1.0);
        }

      }
      else if((xPos+run) < 1.0){
        if(yPos+rise > (p1.yPos + p1.size) || yPos+rise < p1.yPos ){
          reset();
        }
        else{
          run = (abs(run));
        }
        
      }
      if((yPos+rise) > 3.0){
        rise = (abs(rise)*-1.0);
      }
      else if((yPos+rise) < 0.0){
        rise = (abs(rise));
      }
      xPos += run;
      yPos += rise;
      roundedX = round(xPos);
      roundedY = round(yPos);
    }
    void reset(){
      int flip = random(0,2);
      xPos = startingX;
      yPos = startingY;
      if (flip<1){
        rise = startingRise;
        run = startingRun;
      }
      else{
        rise = startingRise*-1.0;
        run = startingRun*-1.0;
      }
      
      delay(1000);
    }

};


const int row[14] = {22,24,26,28,30,32,34,36,38,40,42,44,46,48};
const int col[4] = {4,5,6,7};

const int encoders[2][5] = {
  {13,12,11,10,9},
  {A4,A3,A2,A1,A0}
};

int lastEncoder[2][3] = {
  {0,0,0},
  {0,0,0}
};

int currentEncoder[2][3] = {
  {0,0,0},
  {0,0,0}
};
void prettyRefresh(int c){
  for (int i = 0; i < (sizeof(col)/sizeof(col[0])); i++){
    for (int x = 0; x < (sizeof(row)/sizeof(row[0])); x++){
      digitalWrite(row[x],1);
      digitalWrite(col[i],0);
      delay(c);
      digitalWrite(row[x],0);
      digitalWrite(col[i],1);
    }
  }
}
//GAME OBJECTS
ball ping(7.0,1.0,0.167,0.47);
player players[2] =  {player(0,1,2), player(13,1,2)};
int ready[2] = {0,0};
float animationCooldown = 0.0;
float refresh = 0.0;
int gameStarted = 0;
void setup() {
  // PIN SETUPS
  //display
  for (int i = 0; i < (sizeof(row)/sizeof(row[0])); i++){
    pinMode(row[i],OUTPUT);
    digitalWrite(row[i],0);
  }
  for (int i = 0; i < (sizeof(col)/sizeof(col[0])); i++){
    pinMode(col[i],OUTPUT);
    digitalWrite(col[i],1);
  }

  //encoders
  for (int i = 0; i < 2; i++){
    //GND
    pinMode(encoders[i][0], OUTPUT);
    digitalWrite(encoders[i][0], 0);
    //+
    pinMode(encoders[i][1], OUTPUT);
    digitalWrite(encoders[i][1], 1);
    //SW
    pinMode(encoders[i][2], INPUT);
    digitalWrite(encoders[i][2], 1);
    //DT
    pinMode(encoders[i][3], INPUT);
    //CLK
    pinMode(encoders[i][4], INPUT);
  }
  //pong text

  prettyRefresh(10);
  //squares
  for (int i = 0 ; i < 4; i++){
    digitalWrite(row[i],1);
    digitalWrite(row[i+10],1);
    digitalWrite(col[i],0);
  }
}
float lastTime = 0.0;
float currentTime = 0.0;
void loop() {
  currentTime = millis();
  animationCooldown += currentTime - lastTime;
  refresh += currentTime - lastTime;
  //current encoders
  for (int i = 0; i < 2; i ++ ){
    currentEncoder[i][0] = digitalRead(encoders[i][2]);
    currentEncoder[i][1] = digitalRead(encoders[i][3]);
    currentEncoder[i][2] = digitalRead(encoders[i][4]);
    //click
    if ( currentEncoder[i][0] == 0 && lastEncoder[i][0] == 1){
      ready[i] = 1;
      if (i == 0){
        digitalWrite(row[0],0);
        digitalWrite(row[1],0);
        digitalWrite(row[2],0);
        digitalWrite(row[3],0);
      }
      else{
        digitalWrite(row[10],0);
        digitalWrite(row[11],0);
        digitalWrite(row[12],0);
        digitalWrite(row[13],0);
      }
    }
    //moving thingies
    if (currentEncoder[i][1] == currentEncoder[i][2]) {
      //left 1
      if (lastEncoder[i][1] != currentEncoder[i][1] && lastEncoder[i][2] == currentEncoder[i][2]) {
        
        digitalWrite(col[players[i].yPos],1);
        digitalWrite(col[players[i].yPos+1],1);
        players[i].up();
      }

      //right one
      else if (lastEncoder[i][1] == currentEncoder[i][1] && currentEncoder[i][2] != lastEncoder[i][2]) {
        digitalWrite(col[players[i].yPos],1);
        digitalWrite(col[players[i].yPos+1],1);
        players[i].down();
      }
    }    
    //starting menu animation

  }
  //starting menu game
  if (ready[0] == 0 || ready[1] == 0){
    if (animationCooldown < 5) {
      digitalWrite(col[1],1);
      digitalWrite(col[2],1);
      if (ready[0] == 0){
        digitalWrite(row[1],1);
        digitalWrite(row[2],1);
      }      
      if (ready[1] == 0){
        digitalWrite(row[11],1);
        digitalWrite(row[12],1);
      }
    }
    else if (animationCooldown < 10){
      digitalWrite(col[1],0);
      digitalWrite(col[2],0);
      if (ready[0] == 0){
        digitalWrite(row[1],0);
        digitalWrite(row[2],0);
      }      
      if (ready[1] == 0){
        digitalWrite(row[11],0);
        digitalWrite(row[12],0);
      }
    }
    else{
      animationCooldown = 0;
    }
  }
  //game started
  else if (ready[0] == 1 && ready[1] == 1 && gameStarted == 0){
    gameStarted = 1;
    prettyRefresh(20);
  }
  //game loop
  else if (gameStarted){
    
    if (animationCooldown > 75){
      //turning off previous positions of ball
      digitalWrite(row[ping.roundedX],0);
      digitalWrite(col[ping.roundedY],1);
      //updating the ball's movement 
      ping.updateMovement(players[0],players[1]);
      animationCooldown =0;
    }
    //displaying
    if (refresh<4){
      //disable ball
      digitalWrite(row[ping.roundedX],0);
      digitalWrite(col[ping.roundedY],1);
      //write plyer 1
      digitalWrite(row[players[0].xPos],1);
      digitalWrite(col[players[0].yPos],0);
      digitalWrite(col[players[0].yPos + 1],0);
      
    }
    else if (refresh<10){
      //disable plqay 1
      digitalWrite(row[players[0].xPos],0);
      digitalWrite(col[players[0].yPos],1);
      digitalWrite(col[players[0].yPos + 1],1);
      //write plyer 2
      digitalWrite(row[players[1].xPos],1);
      digitalWrite(col[players[1].yPos],0);
      digitalWrite(col[players[1].yPos + 1],0);
    }
    else if (refresh<15){
      //disable palywe 2
      digitalWrite(row[players[1].xPos],0);
      digitalWrite(col[players[1].yPos],1);
      digitalWrite(col[players[1].yPos + 1],1);

      digitalWrite(row[ping.roundedX],1);
      digitalWrite(col[ping.roundedY],0);
    }
    else{
      refresh = 0;
    }
  }
  
  //last encoders
  for (int i = 0; i < 2; i ++ ){
    lastEncoder[i][0] = currentEncoder[i][0];
    lastEncoder[i][1] = currentEncoder[i][1];
    lastEncoder[i][2] = currentEncoder[i][2];
  }
  lastTime = currentTime;
}