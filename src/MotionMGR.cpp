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

  motors = new MotorController();

  // Triggers low for every 45um increment in layer height
  pinMode(layer_complete_flag, INPUT_PULLUP);
  pinMode(reset_flag, OUTPUT);
  // Reset the layer height complete flag
  digitalWrite(reset_flag, HIGH);

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

  motors->manualOverride();
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

void MotionMGR::processGcode(GCodeCommand* cmd)
{

  //Response string to be returned to the PC, updated with result of execution
  String resp = "Error";

  CURRENT_CODE = cmd->code;

    // Feedrate
  if(cmd->F != -1.0)  setVal(&CURRENT_F, cmd->F);

    // Handle X and Y as before
  if (cmd->X != -1.0 || cmd->Y != -1.0) {
    // Existing handling for X and Y...
    setXY(cmd);
    _status = INTERPOLATING;
    return;
  }

  //Hresp because I am lazy - really needs Mcode handling
  if(cmd->code == 99){

    if(cmd->P != -1.0){
      resp = psu->enablePSU((uint16_t)cmd->P);
    } else {
      resp = psu->enablePSU();
    }

    Serial.println(resp);

    return;
  } else if(cmd->code == 98){

    resp = psu->disablePSU();

    Serial.println(resp);

    return;
  } else if(cmd->code == 97){

    uint8_t lc_flag = digitalRead(layer_complete_flag);

    // Continue movement until distance reached
    //while(lc_flag == 1){

      //Drop the build piston by the intended layer height + compensation for powder raking
      motors->move(build_piston, BUILD_STEPS, PISTON_FREQ, REVERSE);
      delay(500);
      //lc_flag = digitalRead(layer_complete_flag);
    //}
    // Reset the layer height
    //digitalWrite(reset_flag, LOW);
    //delay(100);
    //digitalWrite(reset_flag, HIGH);

    // Supply the recoater with powder

    motors->move(powder_piston, POWDER_STEPS, PISTON_FREQ, FORWARD);
    
    Serial.println("ok");

    return;
  } else if(cmd->code == 96){
    //return the build piston to its intended layer height + account for slop

    motors->move(recoater_pin, RECOATER_STEPS, RECOATER_FREQ, REVERSE);
    delay(100);
    motors->move(recoater_pin, RECOATER_STEPS+12, RECOATER_FREQ, FORWARD);
    delay(100);
    
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
    
    delay(200);

    Serial.println("ok");
    
    return;
  } else if(cmd->code == 94){
 
    if(cmd->P != -1.0){
      laser5W->setWidth((uint16_t)cmd->P);
    } else {
      laser5W->initPWM(100);  //Inverted TTL power control, 100us/20000us (50Hz)
    }
    
    delay(200);

    Serial.println("ok");
    
    return;
  }

  // // Feedrate
  // if(cmd->F != -1.0)  setVal(&CURRENT_F, cmd->F);

  //   // Handle X and Y as before
  // if (cmd->X != -1.0 || cmd->Y != -1.0) {
  //   // Existing handling for X and Y...
  //   setXY(cmd);
  //   _status = INTERPOLATING;
  //   return;
  // }
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
      //Serial.send_now();
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
    //Serial.send_now();
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







