
class DriveBase {
  private:
    int leftPin, rightPin;
  public:
    DriveBase(int left, int right);

    // set left and right motor speeds
    void motors(int leftSpeed, int rightSpeed);
};
