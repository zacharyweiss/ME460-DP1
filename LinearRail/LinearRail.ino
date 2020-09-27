/* Zachary Weiss
** 26 Sept 2020
** Linear Rail Full&Half Step for ME460 */
#define OFF   0
#define NORTH 1
#define SOUTH 2
#define HALF  3
#define FULL  4

// globals and flags
int f_delay  = 2000, //5000,  // full step delay in micros
    h_delay  = 1000,  // half step delay in micros
    steptype = FULL;  // change between HALF and FULL
                      //   to modify step type

void setup() {
  // coil A
  pinMode(12,OUTPUT); // direction
  pinMode(3,OUTPUT);  // on/off
  pinMode(9,OUTPUT);  // brake

  // coil B
  pinMode(13,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(8,OUTPUT);

  shutdown();

  Serial.begin(115200);
}

void loop() {
  // init step and time counters,
  // define max distance in steps
  int step,
      stepmax = 300;
  double t0 = micros();
  
  // print which regime we're operating in, to contextualize
  // later data readouts
  Serial.println(steptype==FULL?"Full":"Half");

  if (steptype==FULL) {
    // 50 steps per 1 revolution
    for (step=0;step<=stepmax;step++) {  
      away(step,t0);
    }
    for (step=0;step<=stepmax;step++) {
      towards(step,t0);
    }
  }
  
  if (steptype==HALF) {
    for (step=0;step<=stepmax;step++) {  
      awayHalf(step,t0);
    }
    for (step=0;step<=stepmax;step++) {
      towardsHalf(step,t0);
    }
  }
  
  shutdown(); // turn off all
  while(1);   // hold until reset
}

// towards stepper
void towards(int s, double t0) {
  int ss = 0; // substep counter 
  coilA(NORTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(OFF);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(SOUTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(OFF);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  if(ss!=4){Serial.println("Full stepcount issue"); shutdown(); while(1);}
}

void towardsHalf(int s, double t0) {
  int ss = 0;
  coilA(OFF);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(OFF);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  if(ss!=8){Serial.println("Half stepcount issue"); shutdown(); while(1);}
}

// away from stepper
void away(int s, double t0) {
  int ss = 0;
  coilA(OFF);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(NORTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(OFF);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  coilA(SOUTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(f_delay);
  if(ss!=4){Serial.println("Full stepcount issue"); shutdown(); while(1);}
}

void awayHalf(int s, double t0) {
  int ss = 0;
  coilA(OFF);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(SOUTH);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(OFF);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(NORTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(OFF);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  coilA(NORTH);
  coilB(SOUTH);
  ss = readout(t0,s,ss);
  delayMicroseconds(h_delay);
  if(ss!=8){Serial.println("Half stepcount issue"); shutdown(); while(1);}
}

// case switching dependent on desired polarity
// turns off brake, selects direction and turns on/off
void coilA(int pole) {
  digitalWrite(9,LOW);
  if (pole == NORTH) {
    digitalWrite(12,HIGH);
    digitalWrite(3,HIGH);
  }
  if (pole == SOUTH) {
    digitalWrite(12,LOW);
    digitalWrite(3,HIGH);
  }
  if (pole == OFF) {
    digitalWrite(12,LOW);
    digitalWrite(3,LOW);
  }
}

void coilB(int pole) {
  digitalWrite(8,LOW);
  if (pole == NORTH) {
    digitalWrite(13,HIGH);
    digitalWrite(11,HIGH);
  }
  if (pole == SOUTH) {
    digitalWrite(13,LOW);
    digitalWrite(11,HIGH);
  }
  if (pole == OFF) {
    digitalWrite(13,LOW);
    digitalWrite(11,LOW);
  }
}

// measures current in A & B
// prints time, step #, and current
int readout(double t0, int s, int ss) {
  float i_A = float(analogRead(0)) / 1023.0 * 5.0 / 1.65 * 1000.0,
        i_B = float(analogRead(1)) / 1023.0 * 5.0 / 1.65 * 1000.0;
  int truestep = steptype==FULL ? s*4 + ss : s*8 + ss;
  Serial.println(
    String((micros()-t0)/1000000)
    +"\t"
    +String(truestep)
    +"\t"
    +String(i_A)
    +"\t"
    +String(i_B)
  );
  ss+=1;
  return ss;
}

// turn all off
void shutdown() {
  digitalWrite (12,LOW);
  digitalWrite (3,LOW);
  digitalWrite (9,LOW);
  digitalWrite (13,LOW);
  digitalWrite (11,LOW);
  digitalWrite (8,LOW);
}
