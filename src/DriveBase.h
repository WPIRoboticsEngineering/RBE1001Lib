
class DriveBase {
  private:
    int leftPinPWM, rightPinPWM;
    int leftPinDIR, rightPinDIR;
  public:
    DriveBase(int leftPWM, int rightPWM,int leftDIR, int rightDIR);

    // set left and right motor speeds
    void motors(int leftSpeed, int rightSpeed);
};
