#include <tcs3200.h>

#include <AccelStepper.h>

AccelStepper vertical(AccelStepper::DRIVER, 5, 4);
const int verticalLimit = -850;

tcs3200 colorSensor(8, 9, 10, 11, 12); // (s0, s1, s2, s3, out)

const int blockCount = 9;
const int totalColorCount = blockCount + 1; // plus ground
int colors[totalColorCount][3] = {          //! colors[0] is ground
    {66, 58, 85},    {200, 111, 200}, {90, 90, 166},   {125, 200, 333},
    {166, 200, 166}, {333, 250, 200}, {250, 100, 142}, {66, 62, 100},
    {142, 125, 166}, {333, 333, 333}};

int colorNumbers[totalColorCount] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
String colorNames[totalColorCount] = {
    "0-ground", "1-purple", "2-blue",  "3-cyan", "4-green",
    "5-yellow", "6-red",    "7-black", "8-gray", "9-white"};

void setup() {
  Serial.begin(9600);

  vertical.setMaxSpeed(2000);
  vertical.setAcceleration(2000);

  vertical.runToNewPosition(-150);
  vertical.setCurrentPosition(0);
}

void loop() {}