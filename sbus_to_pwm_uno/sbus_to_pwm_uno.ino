#include <Servo.h>//the servo output library

uint16_t CH[18];//channel values are stored here
char sbus_flag=0;
const int minAngle = 30; // servo angle range
const int maxAngle = 150;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

void setup()
{   
  servo1.attach(3);//servo pins
  servo2.attach(5);
  servo3.attach(6);
  servo4.attach(9);
  servo5.attach(10);
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
  setServoAngle(servo1,CH[0]);
  setServoAngle(servo2,CH[1]);
  moteur(CH[2]);
  setServoAngle(servo4,CH[3]);
  setServoAngle(servo5,CH[4]);

void serialEvent(void)//this function is activated by serial input, and saves sbus data in buf
{
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
    CH[ 0] = ((int16_t)buf[ 1] >> 0 | ((int16_t)buf[ 2] << 8 )) & 0x07FF;
    CH[ 1] = ((int16_t)buf[ 2] >> 3 | ((int16_t)buf[ 3] << 5 )) & 0x07FF;
    CH[ 2] = ((int16_t)buf[ 3] >> 6 | ((int16_t)buf[ 4] << 2 )  | (int16_t)buf[ 5] << 10 ) & 0x07FF;
    CH[ 3] = ((int16_t)buf[ 5] >> 1 | ((int16_t)buf[ 6] << 7 )) & 0x07FF;
    CH[ 4] = ((int16_t)buf[ 6] >> 4 | ((int16_t)buf[ 7] << 4 )) & 0x07FF;
    CH[ 5] = ((int16_t)buf[ 7] >> 7 | ((int16_t)buf[ 8] << 1 )  | (int16_t)buf[ 9] <<  9 ) & 0x07FF;
    CH[ 6] = ((int16_t)buf[ 9] >> 2 | ((int16_t)buf[10] << 6 )) & 0x07FF;
    CH[ 7] = ((int16_t)buf[10] >> 5 | ((int16_t)buf[11] << 3 )) & 0x07FF;
    CH[ 8] = ((int16_t)buf[12] << 0 | ((int16_t)buf[13] << 8 )) & 0x07FF;
    CH[ 9] = ((int16_t)buf[13] >> 3 | ((int16_t)buf[14] << 5 )) & 0x07FF;
    CH[10] = ((int16_t)buf[14] >> 6 | ((int16_t)buf[15] << 2 )  | (int16_t)buf[16] << 10 ) & 0x07FF;
    CH[11] = ((int16_t)buf[16] >> 1 | ((int16_t)buf[17] << 7 )) & 0x07FF;
    CH[12] = ((int16_t)buf[17] >> 4 | ((int16_t)buf[18] << 4 )) & 0x07FF;
    CH[13] = ((int16_t)buf[18] >> 7 | ((int16_t)buf[19] << 1 )  | (int16_t)buf[20] <<  9 ) & 0x07FF;
    CH[14] = ((int16_t)buf[20] >> 2 | ((int16_t)buf[21] << 6 )) & 0x07FF;
    CH[15] = ((int16_t)buf[21] >> 5 | ((int16_t)buf[22] << 3 )) & 0x07FF;
  }
}
