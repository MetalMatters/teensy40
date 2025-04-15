#ifndef MOTORCONTROLLER_h
#define MOTORCONTROLLER_h

// Manual override buttons
const int forwardButtonPin = 16;
const int backwardButtonPin = 15;
const int selectMotorPin = 18;
const int selectSpeedPin = 21;

// DM542 interface pins
#define build_piston 3
#define powder_piston 2
#define recoater_pin 4
#define direction_pin 5
#define pulse_pin 6

#define FORWARD 0
#define REVERSE 1

// Function to determine the length of an array
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

class MotorController {
  public:
    MotorController();
    void move(int select, int steps, int frequency, int direction);
    void manualOverride();
    void toggleMotor();
    void toggleSpeed();

  private:
    int motorSpeeds[5] = {2000, 1500, 800, 500, 200};
    int speedSetting = 0;
    int selectedMotor = -1;
    int frequency = motorSpeeds[0];
    // unsigned long stepInterval = 1000000 / frequency; // Microseconds per step
    void pulse(int frequency);
    void enableMotor(int select);
    void disableAllMotors();
};

#endif