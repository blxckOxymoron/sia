#pragma region HEADERS

#include <tcs3200.h>

#include <AccelStepper.h>

#pragma endregion HEADERS

#pragma region CONSTANTS

AccelStepper horizontal(AccelStepper::DRIVER, 3, 2); // (type, step, dir)
// bar [mm] * steps per rotation * circumference [mm/rot]
const int horizontalLimit = 7000;

AccelStepper vertical(AccelStepper::DRIVER, 5, 4);
const int verticalLimit = -850;
const int verticalSetupPosition = -180;

AccelStepper claw(AccelStepper::DRIVER, 7, 6);
const int clawLimit = 4700;

const int blockCount = 9;
const int totalColorCount = blockCount + 1;

String colorNames[totalColorCount] = {
    "0-ground", "1-purple", "2-blue",  "3-cyan", "4-green",
    "5-yellow", "6-red",    "7-black", "8-gray", "9-white"};

int blockPositions[blockCount] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

const int sortAreaPadding = 120; // steps (at least blockScanOffset)

const double sourceAreaSplit = 2.0 / 3.0;

const int movementDelay = 200;

#pragma endregion CONSTANTS

#pragma region FUNCTIONS

void scanColors() {
  //* SCAN THE COLORS
  Serial.println("\nSCANNING COLORS \nenter block number when above block\n");

  static const int speedDivisor = 15;

  horizontal.setMaxSpeed(horizontal.maxSpeed() / speedDivisor);
  horizontal.moveTo(horizontalLimit * sourceAreaSplit);

  while (horizontal.distanceToGo() != 0) {
    horizontal.run();

    if (Serial.available()) {
      int blockNumber = Serial.parseInt();
      if (blockNumber > 0 && blockNumber <= blockCount) {
        blockPositions[blockNumber - 1] = horizontal.currentPosition();
        Serial.print("block ");
        Serial.print(colorNames[blockNumber]);
        Serial.print(" at ");
        Serial.println(blockPositions[blockNumber - 1]);
      } else {
        Serial.println("invalid block number");
      }
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

void setupVertical() {
  vertical.runToNewPosition(verticalSetupPosition);
  vertical.setCurrentPosition(0);
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
}

void loop() {
  // TODO Wait for some sort of signal

  //? 0 Position is claw fully opened, fully on the left, and fully down
  Serial.println("Enter \n 'up' to setup claw \n 'go' to start sorting \n "
                 "'home' to move to home position");

  while (!Serial.available())
    continue;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "up") {
    setupVertical();
  } else if (input == "go") {
    scanColors();
    moveBlocks();
    moveToHome();
  } else if (input == "home") {
    moveToHome();
  } else {
    Serial.println("invalid input");
  }
}

#pragma endregion ARDUINO_FUNCTIONS
