#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <Servo.h>
#include <EEPROM.h>
#include <RobotOpen.h>

#define MECANUM true
#define TANK false
#define DRIVEMODE boolean

#define ROLL_SPEED 127

/* I/O Setup */
ROJoystick usb1(1);         // Joystick #1
ROPWM lf_wheel_pwm(0);              
ROPWM rf_wheel_pwm(1);              
ROPWM lr_wheel_pwm(2);
ROPWM rr_wheel_pwm(3);
ROPWM conv_pwm(4);
ROPWM shoot_pwm(5);
ROPWM roller_pwm(6);
RODigitalIO dig0Out(0,OUTPUT);
RODigitalIO dig1Out(1,OUTPUT);
RODigitalIO dig2Out(2,OUTPUT);
RODigitalIO dig3Out(3,OUTPUT);
RODigitalIO dig4Out(4,OUTPUT);
RODigitalIO dig5Out(5,OUTPUT);
RODigitalIO dig6Out(6,OUTPUT);
RODigitalIO dig7Out(7,OUTPUT);
RODigitalIO dig8Out(8,OUTPUT);
RODigitalIO PressureSensor(19,INPUT);
ROSolenoid sol0(0);
ROSolenoid sol1(1);


// I am making these variables global, to provide easier access to some functions
int wheel_speed_reduction = 1;
int x, y, rotate;    
DRIVEMODE current_mode = MECANUM;
int lf,rf,lr,rr;
boolean keep_rolling = true;

void setup()
{
  /* initiate comms */
  RobotOpen.begin(&enabled,&disabled, &timedtasks);
}

void regulate_pressure(){
  // @TODO AAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHHHHHHH!!
}

void jump_wheels(DRIVEMODE mode){
  // @TODO Confirm this out with Collin
  if (mode == TANK){
    dig3Out.off();
    dig4Out.off();
    dig5Out.on();
    dig6Out.off();
    dig7Out.off();
    dig8Out.on(); 
  }else{
    dig3Out.off();
    dig4Out.on();
    dig5Out.off();
    dig6Out.off();
    dig7Out.on();
    dig8Out.off();
  }
}

void set_mode(DRIVEMODE mode){
  jump_wheels(mode);
  current_mode = mode;
}

void set_mecanum_throttle_values(){
  lf = ( x + y + rotate)/wheel_speed_reduction;
  rf = ( x - y + rotate)/wheel_speed_reduction;
  lr = (-x + y + rotate)/wheel_speed_reduction;
  rr = (-x - y + rotate)/wheel_speed_reduction;
}

void set_tank_throttle_values(){
  int rotation_coefficient = 1;
  int rotation_offset = rotate/rotation_coefficient;
  lf = (+y + rotation_offset)/wheel_speed_reduction;
  rf = (-y + rotation_offset)/wheel_speed_reduction;
  lr = (+y + rotation_offset)/wheel_speed_reduction;
  rr = (-y + rotation_offset)/wheel_speed_reduction;
}

void normalize_wheel_throttles(){
  int maximum = max(max(abs(lf), abs(rf)), max(abs(lr), abs(rr)));
  if (maximum > 127) {
    lf = (lf / maximum) * 127;
    rf = (rf / maximum) * 127;
    lr = (lr / maximum) * 127;
    rr = (rr / maximum) * 127;
  }
}

void pwn_write_throttle_values(){
  // Set PWMs, shifted back to [0..255]
  lf_wheel_pwm.write(lf + 127);
  rf_wheel_pwm.write(rf + 127);
  lr_wheel_pwm.write(lr + 127);
  rr_wheel_pwm.write(rr + 127);
}

void pwm_write_conv(){
  int conv = usb1.lTrigger()/2;
  conv_pwm.write(conv);    
}

void pwm_write_shoot(){
  int shoot = usb1.lTrigger()/2;
  shoot_pwm.write(shoot); 
}

void compress_more_air(){
    dig0Out.off();
    dig1Out.off();
    dig2Out.on();
}

void dont_compress_more_air(){
    dig0Out.off();
    dig1Out.off();
    dig2Out.off();
}

void enabled() {
  // get desired translation and rotation, scaled to [-127..128] (0 neutral)
  x = usb1.leftX() - 127;
  y = (255 - usb1.leftY()) - 127;
  rotate = usb1.rightX() - 127;

  // calculate wheel throttles
  if (current_mode == MECANUM)
    set_mecanum_throttle_values();
  else
    set_tank_throttle_values();

  normalize_wheel_throttles();
  pwn_write_throttle_values();
  
  pwm_write_conv();
  pwm_write_shoot();
  
  if (usb1.btnRShoulder())
    set_mode(TANK);
  else if (usb1.btnLShoulder())
    set_mode(MECANUM);
  else{
    sol1.off();
    sol0.off();
  }
  
  if (usb1.btnX()){
    keep_rolling = !keep_rolling;
  }
  
  if (keep_rolling)
    if (usb1.btnY())
      roller_pwm.write(127 - ROLL_SPEED);
    else
      roller_pwm.write(127 + ROLL_SPEED);
  
  if (PressureSensor.read()){
    dont_compress_more_air();
    RODashboard.debug("sensor high");
  }
  else{
    compress_more_air();
    RODashboard.debug("sensor low");
  }
  
  
  RODashboard.publish("sensorinput", PressureSensor.read());
}



/* This is called while the robot is disabled
 * All outputs are automatically disabled (PWM, Solenoid, Digital Outs)
 */
void disabled() {
  // safety code
}


/* This loop ALWAYS runs - only place code here that can run during a disabled state
 * This is also a good spot to put driver station publish code
 */
void timedtasks() {
  RODashboard.publish("Uptime Seconds", ROStatus.uptimeSeconds());
  RODashboard.publish("wheel_speed_reduction", wheel_speed_reduction);
}

// !!! DO NOT MODIFY !!!
void loop() {
  RobotOpen.syncDS();
}
