#pragma region HEADERS

#include "ColorMatch.h"
#include <AccelStepper.h>
#include <MD_TCS230.h>

#pragma endregion HEADERS

#pragma region CONSTANTS

AccelStepper horizontal(AccelStepper::DRIVER, 3, 2); // (type, step, dir)
// bar [mm] * steps per rotation * circumference [mm/rot]
const int horizontalLimit = 7000;

AccelStepper vertical(AccelStepper::DRIVER, 5, 4);
const int verticalLimit = -850;

AccelStepper claw(AccelStepper::DRIVER, 7, 6);
const int clawLimit = 4700;

//! OUT MUST BE ON PORT 5
// s0 -> 10
// s1 -> 11
// s2 -> 12
// s3 -> 13
MD_TCS230 colorSensor(12, 13, 10, 11); // (s2, s3, s0, s1)

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

int blockPositions[blockCount] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

const int blockScanOffset = 60;  // steps (calculated at 96steps)
const int sortAreaPadding = 120; // steps (at least blockScanOffset)

const double sourceAreaSplit = 2.0 / 3.0;

const int movementDelay = 200;

#pragma endregion CONSTANTS

#pragma region COLOR_MATCH_FUNCTIONS

uint8_t colorMatch(colorData *rgb)
// Root mean square distance between the color and colors in the table.
// FOr a limited range of colors this method works ok using RGB
// We don't work out the square root or the mean as it has no effect on the
// comparison for minimum. Square of the distance is used to ensure that
// negative distances do not subtract from the total.
{
  int32_t d;
  uint32_t v, minV = 999999L;
  uint8_t minI;

  for (uint8_t i = 0; i < ARRAY_SIZE(ct); i++) {
    v = 0;
    for (uint8_t j = 0; j < RGB_SIZE; j++) {
      d = ct[i].rgb.value[j] - rgb->value[j];
      v += (d * d);
    }
    if (v < minV) // new best
    {
      minV = v;
      minI = i;
    }
    if (v == 0) // perfect match, no need to search more
      break;
  }

  return (minI);
}

#pragma endregion COLOR_MATCH_FUNCTIONS

#pragma region FUNCTIONS

void scanColors() {

  //* SCAN THE COLORS
  static const int colorReadInterval = 500; // ms
  static const int speedDivisor = 5;
  static const int conecutiveBlockCountThreshold = 3; // min 1

  int lastColorReadTime = -1;

  int lastBlock = -1;
  int consecutiveBlockCount = 0;

  horizontal.setMaxSpeed(horizontal.maxSpeed() / speedDivisor);
  horizontal.moveTo(horizontalLimit * sourceAreaSplit);

  while (horizontal.distanceToGo() != 0) {
    horizontal.run();

    int now = millis();
    if (now - lastColorReadTime > colorReadInterval) {
      lastColorReadTime = now;

      int block = -1; // TODO
      // colorSensor.closestColor(colors, colorNumbers, totalColorCount);

      if (block != lastBlock &&
          consecutiveBlockCount >= conecutiveBlockCountThreshold) {
        blockPositions[block] = horizontal.currentPosition() - blockScanOffset;

        Serial.print("Found block ");
        Serial.print(colorNames[block + 1]);
        Serial.print(" at ");
        Serial.println(blockPositions[block]);
      }

      if (block == -1 || block != lastBlock)
        consecutiveBlockCount = 0;
      else
        consecutiveBlockCount++;

      lastBlock = block;
    }
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

    // area is from max - sortAreaPadding to max * (1-sourceAreaSplit) +
    // sortAreaPadding padding left and right of bockWidth 0 should be max -
    // sortAreaPadding 9 should be max * (1-sourceAreaSplit) + sortAreaPadding

    static const int destinationAreaWidth =
        horizontalLimit * (1 - sourceAreaSplit) - 2 * sortAreaPadding;
    static const int destinationAreaStart =
        horizontalLimit * sourceAreaSplit + sortAreaPadding;
    static const int spacePerBlock = destinationAreaWidth / (blockCount - 1);

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

#pragma endregion FUNCTIONS

#pragma region ARDUINO_FUNCTIONS

void setup() {
  Serial.begin(9600);

  horizontal.setMaxSpeed(1000);
  horizontal.setAcceleration(2000);

  claw.setMaxSpeed(2000);
  claw.setAcceleration(2000);

  vertical.setMaxSpeed(2000);
  vertical.setAcceleration(2000);

  vertical.runToNewPosition(-150);
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

#pragma endregion ARDUINO_FUNCTIONS