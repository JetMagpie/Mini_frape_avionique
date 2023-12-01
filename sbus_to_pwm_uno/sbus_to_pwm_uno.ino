#include <Servo.h>//the servo output library


uint16_t CH[18];//channel values are stored here
uint8_t buf[26];
char sbus_flag=0;
const int minAngle = 30; // servo angle range
const int maxAngle = 150;

//On crée une array contenant tous les servos
Servo g_liste_servos[5];


void setup()
{   
  int liste_pins_servos[5] = {3,5,6,9,10};
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
  g_liste_servos[3].write(rot);
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
  //Gestion d'erreurs
  if(Serial.available()<=25)
    goto close_function;
  
  while(true)
    if((buf[0]=Serial.read())==0x0f)//the sbus communication begins with 0x0f
      break;
  
  for(int i=1;i<26;i++)
    buf[i]=Serial.read();
  
  close_function:
  Sbus_Data_Count(buf);
}

void Sbus_Data_Count(uint8_t *buf){//function translating sbus data to integers
  if(buf[0]==0x0f&&buf[24]==0x00){
    int map_indexs[16] = {1,2,3,5,6,7,9,10,12,13,14,16,17,18,20,21}; //Relation indexs CH et buf
    for(int i =0;i<16;i++){
      //Formule générale
      CH[i] = (static_cast<int16_t>(buf[map_indexs[i]]) >> i*3%8) | (static_cast<int16_t>(buf[map_indexs[i]]) << (8-i*3%8));
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
