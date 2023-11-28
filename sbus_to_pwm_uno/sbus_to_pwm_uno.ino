#include <Servo.h>//the servo output library
#include <array>
#include <map>

uint16_t CH[18];//channel values are stored here
uint8_t buf[26];
char sbus_flag=0;
const int minAngle = 30; // servo angle range
const int maxAngle = 150;

//On crée une array contenant tous les servos
std::array<Servo,5> g_liste_servos;


void setup()
{   
  std::map<int,int> liste_pins_servos = { //Relation servo/pin
    {0,3}
    {1,5}
    {2,6}
    {3,9}
    {4,10}
  }
  int i(0);
  //On assigne à chaque servo un pin
  for(Servo &servo : g_liste_servos)
    servo.attach(liste_pins_servos[i++]);
  Serial.begin(100000);
}

void setServoAngle(Servo s, int angle) {//mapping the channel value to pwm output for servos
  int Angle = map(angle, 512, 1536, minAngle, maxAngle);
  s.write(Angle);
}

void moteur(int throttle) {//the mapping of throttle is different than servos
  int rot = map(throttle, 512, 1536, 60, 130);
  servo3.write(rot);
}

void loop(){
  delay(5);
  int i(0);
  for(Servo &servo : g_liste_servos)
    if(i == 2)
      moteur(CH[i++]);
    else
      setServoAngle(servo,CH[i++]);
}

void serialEvent(void){//this function is activated by serial input, and saves sbus data in buf
    if(Serial.available()>25){
      while(sbus_flag==0){
        buf[0]=Serial.read();
        if(buf[0]==0x0f)//the sbus communication begins with 0x0f
        {sbus_flag=1;}
      }
      if(sbus_flag==1){
        for(int i=1;i<26;i++){
          buf[i]=Serial.read();
        }
        sbus_flag=0;
      }
  }
  Sbus_Data_Count(buf);
}

void Sbus_Data_Count(uint8_t *buf){//function translating sbus data to integers
  if(buf[0]==0x0f&&buf[24]==0x00){
    std::array<int,16> map_indexs = {1,2,3,5,6,7,9,10,12,13,14,16,17,18,20,21}; //Relation indexs CH et buf
    for(int i =0;i<16;i++){
      //Formule générale
      CH[i] = (static_cast<int16_t>buf[map_indexs[i]] >> i*3%8) | (static_cast<int16_t>(buf[map_indexs[i]]) << (8-i*3%8));
      //Exceptions
      switch(i){
        case 2:
        case 10:
          CH[i] |= static_cast<int16_t>(buf[map_indexs[i]+2]) << 10; //Cas 2 et 10
          break;
        case 5:
        case 13:
          CH[i] |= static_cast<int16_t>(buf[map_indexs[i]+2]) << 9; //Cas 5 et 13
          break;     
      }
      //Fin formule générale
      CH[i] &= 0x07FF;
    }
  }
}
