#include <tcs3200.h>

#include <AccelStepper.h>

AccelStepper horizontal(AccelStepper::DRIVER, 2, 3); // (type, step, dir)
// bar [mm] * steps per rotation * circumference [mm/rot]
const int horizontalLimit = 7000;

AccelStepper vertical(AccelStepper::DRIVER, 4, 5);
const int verticalLimit = -850;

AccelStepper claw(AccelStepper::DRIVER, 6, 7);
const int clawLimit = 4700;

tcs3200 colorSensor(8, 9, 10, 11, 12); // (s0, s1, s2, s3, out)

const int blockCount = 9;
const int totalColorCount = blockCount + 1; // plus ground
int colors[totalColorCount][3] = {          //! colors[0] is ground
    {0, 0, 0},    {250, 250, 250}, {0, 0, 0},    {142, 34, 41}, {166, 125, 71},
    {35, 55, 38}, {150, 50, 43},   {22, 25, 45}, {0, 0, 0},     {0, 0, 0}};

int colorNumbers[totalColorCount] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
String colorNames[totalColorCount] = {
    "0-ground", "1-purple", "2-blue",  "3-cyan", "4-green",
    "5-yellow", "6-red",    "7-black", "8-gray", "9-white"};

int blockPositions[blockCount] = {-1, -1, -1, -1, -1, -1, -1, -1};

const int blockScanOffset = 50; // steps
const int blockWidth = 100;     // steps

const double sourceAreaSplit = 2.0 / 3.0;

const int movementDelay = 200;

void setup() {
  Serial.begin(9600);

  horizontal.setMaxSpeed(1000);
  horizontal.setAcceleration(2000);

  claw.setMaxSpeed(2000);
  claw.setAcceleration(2000);

  vertical.setMaxSpeed(2000);
  vertical.setAcceleration(2000);

  vertical.runToNewPosition(-50);
  vertical.setCurrentPosition(0);
}

void loop() {
  // TODO Wait for some sort of signal

  //? 0 Position is claw fully opened, fully on the left, and fully down

  scanColors();

  moveBlocks();

  moveToHome();

  while (true)
    continue;
}

void scanColors() {

  //* SCAN THE COLORS
  static const int colorReadInterval = 500; // ms
  static const int speedDivisor = 5;

  int lastColorRead = -1;

  horizontal.setMaxSpeed(horizontal.maxSpeed() / speedDivisor);
  horizontal.moveTo(horizontalLimit * sourceAreaSplit);

  while (horizontal.distanceToGo() != 0) {
    int now = millis();
    if (now - lastColorRead > colorReadInterval) {
      lastColorRead = now;

      int block =
          colorSensor.closestColor(colors, colorNumbers, totalColorCount);

      if (block != -1) {
        Serial.print("Found color ");
        Serial.println(colorNames[block + 1]);
        blockPositions[block] = horizontal.currentPosition() - blockScanOffset;
      }
    }

    horizontal.run();
  }

  horizontal.setMaxSpeed(horizontal.maxSpeed() * speedDivisor);
}

void moveBlocks() {

  //* MOVE THE BLOCKS

  vertical.runToNewPosition(verticalLimit);
  delay(movementDelay);

  for (int i = 0; i < blockCount; i++) {

    if (blockPositions[i] == -1) {
      Serial.print("No position for block ");
      Serial.println(colorNames[i + 1]);
      continue;
    }

    horizontal.runToNewPosition(blockPositions[i]);
    delay(movementDelay);
    vertical.runToNewPosition(0);
    delay(movementDelay);
    claw.runToNewPosition(clawLimit);
    delay(movementDelay);
    vertical.runToNewPosition(verticalLimit);
    delay(movementDelay);

    // area is from max - blockWidth to max * (1-sourceAreaSplit) + blockWidth
    // padding left and right of bockWidth
    // 0 should be max - blockWidth
    // 9 should be max * (1-sourceAreaSplit) + blockWidth

    static const int destinationAreaWidth =
        horizontalLimit * (1 - sourceAreaSplit) - 2 * blockWidth;
    static const int destinationAreaStart =
        horizontalLimit * sourceAreaSplit + blockWidth;
    static const int spacePerBlock = destinationAreaWidth / blockCount;

    horizontal.runToNewPosition(destinationAreaStart + spacePerBlock * i);
    delay(movementDelay);
    vertical.runToNewPosition(0);
    delay(movementDelay);
    claw.runToNewPosition(0);
    delay(movementDelay);
    vertical.runToNewPosition(verticalLimit);
    delay(movementDelay);
  }
}

void moveToHome() {

  //* MOVE TO HOME

  vertical.runToNewPosition(verticalLimit);
  delay(movementDelay);
  horizontal.runToNewPosition(0);
  delay(movementDelay);
}