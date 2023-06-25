#include <tcs3200.h>

#include <AccelStepper.h>

AccelStepper horizontal(AccelStepper::DRIVER, 2, 3); // (type, step, dir)
// bar [mm] * steps per rotation * circumference [mm/rot]
const int horizontalLimit = 2000;

AccelStepper vertical(AccelStepper::DRIVER, 4, 5);
const int verticalLimit = 2000;

AccelStepper claw(AccelStepper::DRIVER, 6, 7);
const int clawLimit = 2000;

tcs3200 colorSensor(8, 9, 10, 11, 12); // (s0, s1, s2, s3, out)

const int colorCount = 10;
int colors[colorCount][3] = { //! colors[0] is ground
    {0, 0, 0},    {250, 250, 250}, {0, 0, 0},    {142, 34, 41}, {166, 125, 71},
    {35, 55, 38}, {150, 50, 43},   {22, 25, 45}, {0, 0, 0},     {0, 0, 0}};

int colorNumbers[colorCount] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
String colorNames[colorCount] = {"ground",  "1-purple", "2-blue", "3-cyan",
                                 "4-green", "5-yellow", "6-red",  "7-black",
                                 "8-gray",  "9-white"};

int colorPositions[colorCount - 1] = {-1, -1, -1, -1, -1, -1, -1, -1};

void setup() {
  horizontal.setMaxSpeed(1000);
  horizontal.setAcceleration(100);

  vertical.setMaxSpeed(1000);
  vertical.setAcceleration(100);

  claw.setMaxSpeed(1000);
  claw.setAcceleration(100);
}

void loop() {
  // TODO Wait for some sort of signal

  //? 0 Position is claw fully opened, fully on the left, and fully down

  //* SCAN THE COLORS
  int lastColorRead = -1;
  static const int colorReadInterval = 1000;

  horizontal.setMaxSpeed(horizontal.maxSpeed() / 2);
  horizontal.moveTo(horizontalLimit);

  while (horizontal.distanceToGo() != 0) {
    int now = millis();
    if (now - lastColorRead > colorReadInterval) {
      lastColorRead = now;

      int color = colorSensor.closestColor(colors, colorNumbers, colorCount);
      if (color != -1 && colorPositions[color] == -1) {
        colorPositions[color] = horizontal.currentPosition();
      }
    }

    horizontal.run();
  }

  horizontal.setMaxSpeed(horizontal.maxSpeed() * 2);
}
