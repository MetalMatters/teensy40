#include "MotionMGR.h"

XY2_100* _galvo;
LaserController* _laser;
GCodeCommand* currentGcode;

MotionMGR::MotionMGR(GCodeCommand *buf)
{
  buffer = buf;
  psu = new RD60xxController();

  fan = new PWMController(FAN_PIN);
  laser5W = new PWMController(CAMERA_LASER_PIN);

  // FPGA Interface pins
  pinMode(recoater_trig, OUTPUT);
  pinMode(hopper_trig, OUTPUT);
  pinMode(busy_flag, INPUT);
}

void MotionMGR::begin(XY2_100* galvo)
{
  _galvo = galvo;
  _status = IDLE;
}

MotionStatus MotionMGR::getStatus()
{
  return _status;
} 

void MotionMGR::tic()
{
  //Ensure the nanosecond clock is updated periodically
  nanos();

  switch (_status) {
    case IDLE: processGcodes(); break;
    case INTERPOLATING: interpolateMove(); break;
    default: break;
  }
  setGalvoPosition(CURRENT_CMD_X, CURRENT_CMD_Y);
}

bool MotionMGR::processGcodes()
{
  bool gcodeFound = false;
  if(!isBufferEmpty())
  {
    processGcode(&(buffer[tail]));
    gcodeFound = true;   
    buffer[tail].valid = false;
    tail = ((tail + 1) % BUFFER_SIZE);
  }
  return gcodeFound;
}

/* Busy wait the echo replies sent from the 3D printer
   control board, ensuring no other execution occurs until movement is complete */
// void MotionMGR::awaitMovement() {
//   String echoData = "";
//   bool eol = false;

//   while (true) {

//     if (Serial2.available() > 0) {

//       char ch = Serial2.read(); 
//       echoData += ch;
//       // End of line
//       if (ch == '\n') {
//         eol = true;
//       }
//     }

//     if (eol) {
//       //"paused" is sent as part of the keepalive message once Marlin's buffer is clear i.e. movement is complete
//       Serial.println(echoData);
//       if (echoData.indexOf("paused") >= 0) {
//         break; 
//       }

//       echoData = "";
//       eol = false;
//     }
//     delayMicroseconds(1);
//   }
// }

void MotionMGR::processGcode(GCodeCommand* cmd)
{

  CURRENT_CODE = cmd->code;

  //Hack because I am lazy - really needs Mcode handling
  if(cmd->code == 99){

    if(cmd->P != -1.0){
      psu->enablePSU((uint16_t)cmd->P);
    } else {
      psu->enablePSU();
    }

    Serial.println("ok");

    return;
  } else if(cmd->code == 98){

    psu->disablePSU();

    Serial.println("ok");

    return;
  } else if(cmd->code == 97){

    //Toggle hopper trigger
    digitalWrite(hopper_trig, HIGH);
    delay(5);
    digitalWrite(hopper_trig, LOW);
    //Wait on busy flag
    do {
      delay(50);
    } while(digitalRead(busy_flag) == 1);
    
    Serial.println("ok");

    return;
  } else if(cmd->code == 96){
 
    //Toggle recoater trigger
    digitalWrite(recoater_trig, HIGH);
    delay(5);
    digitalWrite(recoater_trig, LOW);
    //Wait on busy flag
    do {
      delay(50);
    } while(digitalRead(busy_flag) == 1);
    
    Serial.println("ok");
    
    return;
  } else if(cmd->code == 95){
 
    if(cmd->P != -1.0){
      //ESC minimum and maximum pulse widths
      if(cmd->P >= 1000 and cmd->P <= 2000){
        fan->setWidth((uint16_t)cmd->P);
      }
    } else {
      fan->initPWM(1000); //1000us (Fan off)
    }
    
    Serial.println("ok");
    
    return;
  } else if(cmd->code == 94){
 
    if(cmd->P != -1.0){
      laser5W->setWidth((uint16_t)cmd->P);
    } else {
      laser5W->initPWM(100);  //Inverted TTL power control, 100us/20000us (50Hz)
    }
    
    Serial.println("ok");
    
    return;
  }

  // Feedrate
  if(cmd->F != -1.0)  setVal(&CURRENT_F, cmd->F);

    // Handle X and Y as before
  if (cmd->X != -1.0 || cmd->Y != -1.0) {
    // Existing handling for X and Y...
    setXY(cmd);
    _status = INTERPOLATING;
    return;
  }


  /*
  // I,J,Z movements must complete before returning to the main loop
  if (cmd->I != -1.0 || cmd->J != -1.0 || cmd->Z != -1.0) {

    String newGcode = "G";
    newGcode += String(cmd->code);

    if (cmd->I != -1.0) {
      newGcode += " A";
      newGcode += String(cmd->I);
    }
    if (cmd->J != -1.0) {
      newGcode += " B";
      newGcode += String(cmd->J);
    }
    if (cmd->Z != -1.0) {
      newGcode += " Z";
      newGcode += String(cmd->Z);
    }
    //If feedrate was defined in this instance, forward it to stepper controller
    if (cmd->F != -1.0) {
      newGcode += " F";
      newGcode += String(cmd->F);
    }

    // Echo command
    Serial.println(newGcode);
    // Send command to 3D printer
    Serial2.println(newGcode);
    // Mcodes block Marlin's buffer to generate keepalive feedback, pending gcode movements
    Serial2.println(M0_PAUSE);
    // BLOCKING: Prevent any other execution from occurring until the movement is complete
    awaitMovement();
    // Unblock Marlin's buffer / cease the generation of keepalive messages
    Serial2.println(M108_RESUME);
    // Signal to host that execution is complete, send next gcode
    Serial.println("ok");
    
  }

  */
}

void MotionMGR::setVal(double* varToSet, double valToSet)
{
  //If a new value wasn't declared, leave the existing value in tact
  if(valToSet == -1) return;

  if(valToSet!=MAX_VAL)
    *varToSet = valToSet;
}
void MotionMGR::setValG91(double* varToSet, double valToSet, double base)
{
  if(valToSet!=MAX_VAL)
    *varToSet = base + valToSet;
  else
    *varToSet = base;
}
void MotionMGR::setXY(GCodeCommand* cmd)
{
  if(CURRENT_ABSOLUTE) {                               //G90 - ABSOLUTE
    setVal(&CURRENT_TO_X, cmd->X);
    setVal(&CURRENT_TO_Y, cmd->Y);
  }
  else{                                               //G91 - RELATIVE
    setValG91(&CURRENT_TO_X, cmd->X,CURRENT_FROM_X);
    setValG91(&CURRENT_TO_Y, cmd->Y,CURRENT_FROM_Y);
  }
}

void MotionMGR::interpolateMove()
{
  if(isMoveFirstInterpolation)
  {
    if(CURRENT_CODE == 0 || CURRENT_CODE == 28)
    {
      //dont interpolate
      CURRENT_FROM_X = CURRENT_TO_X;
      CURRENT_FROM_Y = CURRENT_TO_Y;
      CURRENT_CMD_X = CURRENT_TO_X;
      CURRENT_CMD_Y = CURRENT_TO_Y;
      _status = IDLE;
      isMoveFirstInterpolation = true;
      // Signal to host that execution is complete, send next gcode
      Serial.println("ok");
      Serial.send_now();
      return;
    }
    if(CURRENT_CODE == 1)
    {
      CURRENT_DISTANCE_X = CURRENT_TO_X-CURRENT_FROM_X;
      CURRENT_DISTANCE_Y = CURRENT_TO_Y-CURRENT_FROM_Y;

      double moveLength = calculateMoveLengthNanos(CURRENT_DISTANCE_X, CURRENT_DISTANCE_Y, CURRENT_F, &CURRENT_DURATION);

      CURRENT_STARTNANOS = nanos();
      CURRENT_ENDNANOS = CURRENT_STARTNANOS + CURRENT_DURATION;
      isMoveFirstInterpolation = false;
    }
  }

  if(nanos() >= CURRENT_ENDNANOS)
  {
    //done interpolating
    CURRENT_FROM_X = CURRENT_TO_X;
    CURRENT_FROM_Y = CURRENT_TO_Y;
    CURRENT_CMD_X = CURRENT_TO_X;
    CURRENT_CMD_Y = CURRENT_TO_Y;
    _status = IDLE;
    isMoveFirstInterpolation = true;
    // Signal to host that execution is complete, send next gcode
    Serial.println("ok");
    Serial.send_now();
    return;
  }
  else
  {
    double fraction_of_move = (double)(nanos()-CURRENT_STARTNANOS)/CURRENT_DURATION;
    CURRENT_CMD_X = (CURRENT_FROM_X + (CURRENT_DISTANCE_X*fraction_of_move));
    CURRENT_CMD_Y = (CURRENT_FROM_Y + (CURRENT_DISTANCE_Y*fraction_of_move));
    return;
  }

}

/* 
  MotionMGR::calculateMoveLengthNanos
    Velocity is presumed to be in mm/s
    lengthOfMove = calc hypotenuse a^2+b^2=c^2
    result <-- (mm)/(mm/s) = s   (movelength/moveVolocity) -> (example) 2units / (4units/second) = 0.5seconds *1000 = 500ms
 */
double MotionMGR::calculateMoveLengthNanos(double xdist, double ydist, double moveVelocity, uint64_t* result)  {  

  double lengthOfMove = sqrt( (0.0 + xdist)*(0.0 + xdist)  + (0.0 + ydist)*(0.0 + ydist) );
  double duration = lengthOfMove/(moveVelocity/60.0);
  *result = (uint64_t)(duration*1E9);
  return lengthOfMove;
}







