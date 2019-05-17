// Schmidt Press Code
//Troy Williams
//5-14-2019
//Include Necessary Libaries
#include <Controllino.h>
/* Code Function -
  INPUTS
  I4-Encoder Wire
  I5-Encoder Wire
  I6-Encoder Wire
  I7-Encoder Wire
  I8 - Confirm selection of encoder
  I9 - Input for single hit pedal
  I10- Input for multi-hit pedal
  OUTPUTS-
  ActivateAirCylinder-D03
  StepPin=DO1
  DirectionPin = DO0
  StepperLogicPower = AO0
*/
//INPUTS
const int MainAirStatus = CONTROLLINO_AI0;
const int CylinderLLSwitch = CONTROLLINO_AI1;
const int CylinderULSwitch = CONTROLLINO_AI2;
const int BlackSelectorWire = CONTROLLINO_AI3;
const int WhiteSelectorWire = CONTROLLINO_AI4;
const int RedSelectorWire = CONTROLLINO_AI5;
const int GraySelectorWire = CONTROLLINO_AI6;
const int ConfirmSelector = CONTROLLINO_AI7;
const int SingleTrigger = CONTROLLINO_AI8;
const int CycleTrigger = CONTROLLINO_AI9;
const int ControlOnSwitch = CONTROLLINO_AI10;

//OUTPUTS
const int MainAir = CONTROLLINO_DO0;
const int PressStatusLight = CONTROLLINO_DO1;
const int SelectorLight = CONTROLLINO_DO2;
const int PressStroke = CONTROLLINO_DO3;
const int PowerOnLight = CONTROLLINO_DO4;


//Misc Variables
int CycleCountSetting;
unsigned long CurrentTime;
unsigned long LastCycleCheckTime;
unsigned long CycleCheckTime = 100;
unsigned long LastBlinkTime;
int CurrentCycleCount;
int SelectorSwitchBlinkDelay = 350;
int HitCounter;
int LastHitCounter;
int CycleStrokeDelayTime = 0;
int SingleStrokeDelayTime = 75;
unsigned long DebounceTime = 125;
unsigned long PedalPressTime;
unsigned long PreviousPedalPressTime;
int SingleTriggerReading;
int SingleTriggerState;
int PreviousSingleTriggerReading;
int CycleTriggerReading;
int CycleTriggerState;
int PreviousCycleTriggerReading;

void setup() {
  pinMode(MainAirStatus, INPUT);
  pinMode(CylinderULSwitch, INPUT);
  pinMode(CylinderLLSwitch, INPUT);
  pinMode(BlackSelectorWire, INPUT);
  pinMode(WhiteSelectorWire, INPUT);
  pinMode(BlackSelectorWire, INPUT);
  pinMode(WhiteSelectorWire, INPUT);
  pinMode(RedSelectorWire, INPUT);
  pinMode(GraySelectorWire, INPUT);
  pinMode(ConfirmSelector, INPUT);
  pinMode(SingleTrigger, INPUT);
  pinMode(CycleTrigger, INPUT);
  pinMode(ControlOnSwitch, INPUT);

  pinMode(MainAir, OUTPUT);
  pinMode(PressStatusLight, OUTPUT);
  pinMode(SelectorLight, OUTPUT);
  pinMode(PressStroke, OUTPUT);
  pinMode(PowerOnLight, OUTPUT);

  digitalWrite(PressStroke, LOW);
  delay(300);
  Serial.begin(9600);
}

void loop() {
  unsigned long CurrentTime = millis();
  //Check the main inputs every CycleCheckTime interval
  if (abs(CurrentTime - LastCycleCheckTime) > CycleCheckTime) {
    HitCounter = 0;
    LastHitCounter = 0;
    LastCycleCheckTime = millis();
    CurrentCycleCount = ReadEncoder();
    if (digitalRead(ConfirmSelector) == HIGH) {
      CycleCountSetting = CurrentCycleCount;
      digitalWrite(SelectorLight, LOW);
    }
    if (digitalRead(ControlOnSwitch) == HIGH) {
      digitalWrite(PowerOnLight, HIGH);
      digitalWrite(MainAir, HIGH);
    }
  }
  // Debounce the foot pedals
  //SingleTriggerDebouncing
  SingleTriggerReading = digitalRead(SingleTrigger);
  if (PreviousSingleTriggerReading != SingleTriggerReading) {
    PedalPressTime = millis();
  }
  PreviousSingleTriggerReading = SingleTriggerReading;
  Serial.println(millis() - PedalPressTime);
  if (SingleTriggerReading == HIGH && (millis() - PedalPressTime) > DebounceTime && digitalRead(PressStatusLight) == HIGH) {
    SingleTriggerState = HIGH;
  }

  //CycleTriggerDebouncing
  CycleTriggerReading = digitalRead(CycleTrigger);
  if (PreviousCycleTriggerReading != CycleTriggerReading) {
    PedalPressTime = millis();
  }
  PreviousCycleTriggerReading = CycleTriggerReading;
  if (CycleTriggerReading == HIGH && (millis() - PedalPressTime) > DebounceTime && digitalRead(PressStatusLight) == HIGH) {
    CycleTriggerState = HIGH;
  }

  //Blink the Cycle Count indicator light
  if (CurrentCycleCount != CycleCountSetting && abs(CurrentTime - LastBlinkTime) > SelectorSwitchBlinkDelay ) {
    digitalWrite(SelectorLight, !digitalRead(SelectorLight));
    LastBlinkTime = millis();
    digitalWrite(PressStatusLight, LOW);
  }

  //If all systems are go, turn on the press status light
  if (digitalRead(SingleTrigger) == LOW && digitalRead(PowerOnLight) == HIGH && digitalRead(CycleTrigger) == LOW
      && CurrentCycleCount == CycleCountSetting && digitalRead(MainAirStatus) == HIGH && digitalRead(CylinderULSwitch) == HIGH
      && CycleTriggerState == LOW && SingleTriggerState == LOW) {
    digitalWrite(PressStatusLight, HIGH);
  }

  //If the main air shuts down (emergency stop), turn off the status light and power on light. - System must be rebooted.
  if (digitalRead(MainAirStatus) == LOW) {
    digitalWrite(PressStatusLight, LOW);
    digitalWrite(PowerOnLight, LOW);
  }
  //Single stroke actuation. Turns status light off during pressing then back on.
  if (SingleTriggerState == HIGH && digitalRead(PressStatusLight) == HIGH &&
      digitalRead(CylinderULSwitch) == HIGH) {
    int StrokeComplete = LOW;
    digitalWrite(PressStatusLight, LOW);
    while (StrokeComplete == LOW && digitalRead(MainAirStatus) == HIGH) {
      if (digitalRead(CylinderULSwitch) == HIGH) {
        delay(SingleStrokeDelayTime);
        digitalWrite(PressStroke, HIGH);
      }
      if (digitalRead(CylinderLLSwitch) == HIGH) {
        delay(SingleStrokeDelayTime);
        digitalWrite(PressStroke, LOW);
        StrokeComplete = HIGH;
      }
    }
    SingleTriggerState = LOW;
  }
  //Cycle stroke actuation.  Turns status light off during pressing then back on.
  if (CycleTriggerState == HIGH && digitalRead(PressStatusLight) == HIGH &&
      digitalRead(CylinderULSwitch) == HIGH) {
    digitalWrite(PressStatusLight, LOW);
    while (HitCounter <= CurrentCycleCount && digitalRead(MainAirStatus) == HIGH) {
      if (digitalRead(CylinderULSwitch) == HIGH && HitCounter == LastHitCounter) {
        delay(CycleStrokeDelayTime);
        digitalWrite(PressStroke, HIGH);
        HitCounter = HitCounter + 1;
      }
      if (digitalRead(CylinderLLSwitch) == HIGH && HitCounter != LastHitCounter) {
        delay(CycleStrokeDelayTime);
        digitalWrite(PressStroke, LOW);
        LastHitCounter = HitCounter;
      }
    }
    digitalWrite(PressStroke, LOW);
  }
  CycleTriggerState = LOW;
}


//Reading the cycle selection encoder. Returns proper count.
int ReadEncoder() {
  int SelectedHitCount;
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 0;
    // Serial.println("HC:0");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 1;
    //Serial.println("HC:1");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 2;
    // Serial.println("HC:2");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 3;
    //Serial.println("HC:3");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 4;
    // Serial.println("HC:4");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 5;
    // Serial.println("HC:5");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 6;
    // Serial.println("HC:6");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == LOW) {
    SelectedHitCount = 7;
    // Serial.println("HC:7");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 8;
    // Serial.println("HC:8");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 9;
    // Serial.println("HC:9");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 10;
    // Serial.println("HC:10");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == LOW && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 11;
    // Serial.println("HC:11");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 12;
    // Serial.println("HC:12");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == LOW && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 13;
    // Serial.println("HC:13");
  }
  if (digitalRead(BlackSelectorWire) == LOW && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 14;
    //Serial.println("HC:14");
  }
  if (digitalRead(BlackSelectorWire) == HIGH && digitalRead(WhiteSelectorWire) == HIGH && digitalRead(RedSelectorWire) == HIGH && digitalRead(GraySelectorWire) == HIGH) {
    SelectedHitCount = 15;
    //Serial.println("HC:15");
  }
  return SelectedHitCount; // = SelectedHitCount - 1; //Change the variable to the number of ball rotations.
}
