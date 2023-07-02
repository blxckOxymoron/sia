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

int colorScanMax[totalColorCount][3] = {
    {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},
    {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1},
};
int colorScanMin[totalColorCount][3] = {
    {INT16_MAX, INT16_MAX, INT16_MAX}, {INT16_MAX, INT16_MAX, INT16_MAX},
    {INT16_MAX, INT16_MAX, INT16_MAX}, {INT16_MAX, INT16_MAX, INT16_MAX},
    {INT16_MAX, INT16_MAX, INT16_MAX}, {INT16_MAX, INT16_MAX, INT16_MAX},
    {INT16_MAX, INT16_MAX, INT16_MAX}, {INT16_MAX, INT16_MAX, INT16_MAX},
    {INT16_MAX, INT16_MAX, INT16_MAX}, {INT16_MAX, INT16_MAX, INT16_MAX},
};

int colorNumbers[totalColorCount] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
String colorNames[totalColorCount] = {
    "0-ground", "1-purple", "2-blue",  "3-cyan", "4-green",
    "5-yellow", "6-red",    "7-black", "8-gray", "9-white"};

void setup() {
  Serial.begin(9600);
  Serial.println("Calibration started");

  Serial.print("Type a number to calibrate:");
  for (const String &color : colorNames) {
    Serial.print(" ");
    Serial.print(color);
  }
  Serial.println();

  Serial.println("To test the sensor, type 'test-on' or 'test-off'");
  Serial.println("To print info about all colors, type 'info'");

  vertical.setMaxSpeed(2000);
  vertical.setAcceleration(2000);

  vertical.runToNewPosition(-150);
  vertical.setCurrentPosition(0);
}

const int callibrationCount = 20;
const int callibrationDelay = 100;

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "test-on") {
      colorTesting();
    } else if (input == "test-off") {
      Serial.println("Test not running");
    } else if (input == "info") {
      printAllColorInfo();
    } else {
      int colorNumber = input.toInt();
      if (colorNumber >= 0 && colorNumber < totalColorCount) {
        Serial.print("Calibrating ");
        Serial.println(colorNames[colorNumber]);
        calibrateColor(colorNumber);
      } else {
        Serial.println("Invalid input");
      }
    }
  }
}

void colorTesting() {
  Serial.println("Test enabled");

  while (!Serial.available() || Serial.readStringUntil('\n') != "test-off") {
    printColorReading();
    delay(1000);
  }

  Serial.println("Test disabled");
}

void printColorReading() {
  Serial.print("detected: ");
  Serial.print(colorSensor.closestColor(colors, colorNames, totalColorCount));
  Serial.print("\tred: ");
  Serial.print(colorSensor.colorRead('r'));
  Serial.print("\tgreen: ");
  Serial.print(colorSensor.colorRead('g'));
  Serial.print("\tblue: ");
  Serial.println(colorSensor.colorRead('b'));
}

void calibrateColor(int color) {
  int totalRed = 0;
  int totalGreen = 0;
  int totalBlue = 0;

  for (int i = 0; i < callibrationCount; i++) {
    int red = colorSensor.colorRead('r');
    int green = colorSensor.colorRead('g');
    int blue = colorSensor.colorRead('b');

    totalRed += red;
    totalGreen += green;
    totalBlue += blue;

    colorScanMax[color][0] = max(colorScanMax[color][0], red);
    colorScanMax[color][1] = max(colorScanMax[color][1], green);
    colorScanMax[color][2] = max(colorScanMax[color][2], blue);

    colorScanMin[color][0] = min(colorScanMin[color][0], red);
    colorScanMin[color][1] = min(colorScanMin[color][1], green);
    colorScanMin[color][2] = min(colorScanMin[color][2], blue);

    delay(callibrationDelay);
  }

  colors[color][0] = totalRed / callibrationCount;
  colors[color][1] = totalGreen / callibrationCount;
  colors[color][2] = totalBlue / callibrationCount;

  printColorInfo(color);
}

void printColorInfo(int color) {
  Serial.print("Color info for ");
  Serial.println(colorNames[color]);

  Serial.println("\t{r,\tg,\tb}");
  Serial.print("max:\t");
  printArray(colorScanMax[color], 3);
  Serial.println();
  Serial.print("min:\t");
  printArray(colorScanMin[color], 3);
  Serial.println();
  Serial.print("avg:\t");
  printArray(colors[color], 3);
  Serial.println();
}

void printArray(int array[], int size, String separator = ",\t") {
  Serial.print("{");
  for (int i = 0; i < size; i++) {
    if (i != 0)
      Serial.print(separator);
    Serial.print(array[i]);
  }
  Serial.print("}");
}

void printAllColorInfo() {
  Serial.println("Printing info for all colors");
  Serial.println();
  for (int i = 0; i < totalColorCount; i++) {
    printColorInfo(i);
    Serial.println();
  }

  Serial.print("{");
  for (int i = 0; i < totalColorCount; i++) {
    if (i != 0)
      Serial.print(",\t");
    printArray(colors[i], 3, ", ");
  }
  Serial.println("}");
}