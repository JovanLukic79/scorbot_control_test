
// This alternate version of the code does not require
// atomic.h. Instead, interrupts() and noInterrupts() 
// are used. Please use this code if your 
// platform does not support ATOMIC_BLOCK.

// A class to compute the control signal
class SimplePID{
  private:
    float kp, kd, ki, umax; // Parameters
    float eprev, eintegral; // Storage

  public:
  // Constructor
  SimplePID() : kp(1), kd(0), ki(0), umax(255), eprev(0.0), eintegral(0.0){}

  // A function to set the parameters
  void setParams(float kpIn, float kdIn, float kiIn, float umaxIn){
    kp = kpIn; kd = kdIn; ki = kiIn; umax = umaxIn;
  }

  // A function to compute the control signal
  void evalu(int value, int target, float deltaT, int &pwr, int &dir){
    // error
    int e = target - value;
  
    // derivative
    float dedt = (e-eprev)/(deltaT);
  
    // integral
    eintegral = eintegral + e*deltaT;
  
    // control signal
    float u = kp*e + kd*dedt + ki*eintegral;
  
    // motor power
    pwr = (int) fabs(u);
    if( pwr > umax ){
      pwr = umax;
    }
  
    // motor direction
    dir = 1;
    if(u<0){
      dir = -1;
    }
  
    // store previous error
    eprev = e;
  }
  
};

#include <ros.h>
#include <std_msgs/UInt16MultiArray.h>
#include <util/atomic.h>
#include <sensor_msgs/joint_states.h>

ros::NodeHandle  nh;

// How many motors
#define NMOTORS 6
double base_motor1;
double link1_motor2;
double link2_motor3;
double rotgrip_motor4;
double rotgrip_motor5;
double gripper_motor6;

// Pins
const int enca[] = {2,3,18,19,20,21};
const int encb[] = {22,23,24,25,26,27};
const int pwm[] = {8,9,10,11,12,13};
const int in1[] = {49,48,45,44,41,39};
const int in2[] = {50,47,46,43,42,40};

/*
 Microswitches
  
  
 */




//ROS MOVEIT program publishes in radians



//if doesn't work, change back to cmd_msg.position[0]...(but it should)
void arm_actuate_cb(const std_msgs::UInt16MultiArray& msg){
  base_motor1 = degreeToTicksbase(msg.data[0]);
  link1_motor2 = degreeToTicks(msg.data[1]);
  link2_motor3 = degreeToTicks(msg.data[2]);
  rotgrip_motor4 = degreeToTicksrot(msg.data[3]);
  rotgrip_motor5 = degreeToTicksrot(msg.data[4]);
  gripper_motor6 = degreeToTicksGripper(msg.data[5]);
}



double degreeToTicksbase(float position_ticks)
{

  position_ticks = position_ticks * 82.5;

  return position_ticks;

}


double degreeToTicks(float position_ticks)
{

  position_ticks = position_ticks * 77.6;

  return position_ticks;

}


double degreeToTicksrot(float position_ticks)
{

  position_ticks = position_ticks * 17.02;

  return position_ticks;

}

double degreeToTicksGripper(float position_ticks)
{

  position_ticks = position_ticks * 18.33;

  return position_ticks;

}


// Globals
long prevT = 0;
volatile int posi[] = {0,0,0,0,0,0};

// PID class instances
SimplePID pid[NMOTORS];


ros::Subscriber<std_msgs::UInt16MultiArray> sub("/arduino/arm_actuate", &arm_actuate_cb);


void setup() {
  nh.getHardware()->setBaud(115200);
  nh.initNode();
  nh.subscribe(sub);
  
  for(int k = 0; k < NMOTORS; k++){
    pinMode(enca[k],INPUT);
    pinMode(encb[k],INPUT);
    pinMode(pwm[k],OUTPUT);
    pinMode(in1[k],OUTPUT);
    pinMode(in2[k],OUTPUT);

    pid[k].setParams(1,0.009,0.0001,255);
  }
  
   attachInterrupt(digitalPinToInterrupt(enca[0]),readEncoder<0>,RISING);
   attachInterrupt(digitalPinToInterrupt(enca[1]),readEncoder<1>,RISING);
   attachInterrupt(digitalPinToInterrupt(enca[2]),readEncoder<2>,RISING);
   attachInterrupt(digitalPinToInterrupt(enca[3]),readEncoder<3>,RISING);
   attachInterrupt(digitalPinToInterrupt(enca[4]),readEncoder<4>,RISING);
   attachInterrupt(digitalPinToInterrupt(enca[5]),readEncoder<5>,RISING);
}




void loop() {
  nh.spinOnce();

  // set target position
  int target[NMOTORS];
  target[0] = -base_motor1;
  target[1] = -link1_motor2;
  target[2] = link2_motor3;
  target[3] = rotgrip_motor4;
  target[4] = rotgrip_motor5;
  target[5] = gripper_motor6;


  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  // Read the position
  int pos[NMOTORS];
  noInterrupts(); // disable interrupts temporarily while reading
  for(int k = 0; k < NMOTORS; k++){
      pos[k] = posi[k];
    }
 
  interrupts(); // turn interrupts back on
  
  // loop through the motors
  for(int k = 0; k < NMOTORS; k++){
    int pwr, dir;
    // evaluate the control signal
    pid[k].evalu(pos[k],target[k],deltaT,pwr,dir);
    // signal the motor
    setMotor(dir,pwr,pwm[k],in1[k],in2[k]);
  }

  }

void setMotor(int dir, int pwmVal, int pwm, int in1, int in2){
  analogWrite(pwm,pwmVal);
  if(dir == 1){
    digitalWrite(in1,HIGH);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    digitalWrite(in1,LOW);
    digitalWrite(in2,HIGH);
  }
  else{
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);
  }  
}

template <int j>
void readEncoder(){
  int b = digitalRead(encb[j]);
  if(b > 0){
    posi[j]++;
  }
  else{
    posi[j]--;
  }
}
