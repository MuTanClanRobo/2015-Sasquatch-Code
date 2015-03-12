#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include <Servo.h>
#include <EEPROM.h>
#include <RobotOpen.h>

#define MECANUM true
#define TANK false
#define DRIVEMODE boolean

#define PRESSURESTATUS int
#define TOO_HIGH 2
#define ALRIGHT 1
#define TOO_LOW 0

/* I/O Setup */
ROJoystick usb1(1);         // Joystick #1
ROPWM lf_wheel_pwm(0);              
ROPWM rf_wheel_pwm(1);              
ROPWM lr_wheel_pwm(2);
ROPWM rr_wheel_pwm(3);
ROPWM pwm4(4);
ROPWM pwm5(5);
RODigitalIO dig0Out(0,OUTPUT);
RODigitalIO dig1Out(1,OUTPUT);
RODigitalIO dig2Out(2,OUTPUT);
RODigitalIO PressureSensor(19,INPUT);
ROSolenoid sol0(0);
ROSolenoid sol1(1);


// I am making these variables global, to provide easier access to some functions
int wheel_speed_reduction = 1;
int x, y, rotate;    
DRIVEMODE current_mode = MECANUM;
int lf,rf,lr,rr;

void regulate_pressure(){

}

void jump_front_wheels(DRIVEMODE mode){
    // @TODO Fill this out with Collin
}

void jump_rear_wheels(DRIVEMODE mode){
    // @TODO Fill this out with Collin
}

void switch_mode(DRIVEMODE mode){
  jump_rear_wheels(mode);
  jump_front_wheels(mode);
  current_mode = mode;
}

void setup()
{
  /* initiate comms */
  RobotOpen.begin(&enabled,&disabled, &timedtasks);
}

void set_mecanum_throttle_values(){
  lf = (x + y + rotate)/wheel_speed_reduction;
  rf = (x - y + rotate)/wheel_speed_reduction;
  lr = (-x + y + rotate)/wheel_speed_reduction;
  rr = (-x - y + rotate)/wheel_speed_reduction;
}

void set_tank_throttle_values(){
  int rotaion_coefficient = 1;
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
  
  // Set PWMs, shifted back to [0..255]
  lf_wheel_pwm.write(lf + 127);
  rf_wheel_pwm.write(rf + 127);
  lr_wheel_pwm.write(lr + 127);
  rr_wheel_pwm.write(rr + 127);

  // WHAT THE FLYING FUCK IS GOING ON???!!!
  if (usb1.btnY()){
    dig0Out.off();
    dig1Out.off();
    dig2Out.on();
    RODashboard.debug("sensor high");}
    else{
    dig0Out.off();
    dig1Out.off();
    dig2Out.off();
    RODashboard.debug("sensor low");}
  
  if(usb1.btnA())
    sol0.on();
  else
    sol0.off();
  if(usb1.btnB())
    sol1.on();
  else
    sol1.off();
    
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
