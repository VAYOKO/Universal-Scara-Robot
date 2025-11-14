#include <AccelStepper.h>
#include <MultiStepper.h>
#include <elapsedMillis.h>

elapsedMillis printtime;

// มอเตอร์ 3 ตัว
AccelStepper stepper1(AccelStepper::FULL2WIRE, 3, 6);
AccelStepper stepper2(AccelStepper::FULL2WIRE, 2, 5);
AccelStepper stepper3(AccelStepper::FULL2WIRE, 4, 7);

// ตัวแปรเก็บตำแหน่ง
long pos1[20];
long pos2[20];
long pos3[20];
bool vaccum_state[20];
bool current_veccum_state = false;
int limit_switch[3] = { 11, 10, 9 };  //z x y
bool HOMEING_MODE = false;
int DISTANCE = 100;
int step = 0;
bool state_1 = false;
int en_shield = 8;
int en_motor = 12;
int Status = 0;
// ตัวแปรควบคุมการเคลื่อนที่จาก GUI
bool moveCommandActive = false;

void setup() {
  Serial.begin(115200);
  printtime = 0;

  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);

  pinMode(en_shield, HIGH);
  pinMode(en_motor, HIGH);

  digitalWrite(en_shield, LOW);
  digitalWrite(en_motor, LOW);

  // ตั้งค่า Stepper
  stepper1.setMaxSpeed(300000.0);
  stepper1.setAcceleration(2000.0);
  stepper2.setMaxSpeed(300000.0);
  stepper2.setAcceleration(2000.0);
  stepper3.setMaxSpeed(300000.0);
  stepper3.setAcceleration(2000.0);
}

// ตรวจสอบว่าหยุดครบทุกแกนหรือยัง
bool steppers_are_stopped() {
  return stepper1.distanceToGo() == 0 && stepper2.distanceToGo() == 0 && stepper3.distanceToGo() == 0;
}

void loop() {
  // ถ้ามีคำสั่งเคลื่อนที่จาก GUI


  // ส่วนของ step ปกติ (manual mode)


  // อ่าน Serial
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    // === โหมด GUI "M,pos1,pos2,pos3,vacuum" ===


    // === ปุ่มควบคุมแบบ manual ===
    if (cmd == '4') {
      digitalWrite(en_shield, LOW);
      digitalWrite(en_motor, LOW);
    }
    if (cmd == '3') {
      digitalWrite(en_shield, HIGH);
      digitalWrite(en_motor, HIGH);
    }

    if (cmd == 'H') {
      Status = 1;
      Homeing();
    }
    if (cmd == 'b') DISTANCE += 100;
    if (cmd == 'B') DISTANCE -= 100;
    if (cmd == 'a') DISTANCE += 1000;
    if (cmd == 'A') DISTANCE -= 1000;
    if (cmd == 'c') DISTANCE += 10;
    if (cmd == 'C') DISTANCE -= 10;

    if (cmd == 'n') {
      current_veccum_state = 1;
      vaccum_state[step] = 1;
      state_1 = false;
    }
    if (cmd == 'N') {
      current_veccum_state = 0;
      vaccum_state[step] = 0;
      state_1 = false;
    }

    if (cmd == 'y') pos1[step] += DISTANCE;
    if (cmd == 'Y') pos1[step] -= DISTANCE;
    if (cmd == 'i') pos2[step] += DISTANCE;
    if (cmd == 'I') pos2[step] -= DISTANCE;
    if (cmd == 'p') pos3[step] += DISTANCE;
    if (cmd == 'P') pos3[step] -= DISTANCE;

    if (cmd == 'r') {
      pos1[step] = stepper1.currentPosition();
      pos2[step] = stepper2.currentPosition();
      pos3[step] = stepper3.currentPosition();
      vaccum_state[step] = current_veccum_state;
      state_1 = false;

      if (step < 19) {
        step++;
        if (pos1[step] == 0 && pos2[step] == 0 && pos3[step] == 0)
          pos1[step] = pos1[step - 1], pos2[step] = pos2[step - 1], pos3[step] = pos3[step - 1];
      }
      current_veccum_state = vaccum_state[step];
    }

    if (cmd == 'v' && step < 19) {
      step++;
      current_veccum_state = vaccum_state[step];
      state_1 = false;
    }
    if (cmd == 'V' && step > 0) {
      step--;
      current_veccum_state = vaccum_state[step];
      state_1 = false;
    }
  }
  stepper1.moveTo(pos1[step]);
  stepper2.moveTo(pos2[step]);
  stepper3.moveTo(pos3[step]);
  stepper1.run();
  stepper2.run();
  stepper3.run();


  data_out();
}
// ส่งข้อมูลออกทุก 100ms



void data_out() {
  if (printtime >= 100) {
    printtime = 0;
    Serial.print(step);
    Serial.print(",");
    Serial.print(stepper1.currentPosition());
    Serial.print(",");
    Serial.print(stepper2.currentPosition());
    Serial.print(",");
    Serial.print(stepper3.currentPosition());
    Serial.print(",");
    Serial.print(stepper1.speed());
    Serial.print(",");
    Serial.print(stepper2.speed());
    Serial.print(",");
    Serial.print(stepper3.speed());
    Serial.print(",");
    Serial.print(DISTANCE);
    Serial.print(",");
    Serial.print(digitalRead(limit_switch[0]));
    Serial.print(",");
    Serial.print(digitalRead(limit_switch[1]));
    Serial.print(",");
    Serial.print(digitalRead(limit_switch[2]));
    Serial.print(",");
    Serial.println(Status);
  }
}
int Homeming_step = 0;
void Homeing() {

  while (Status == 1) {
    while (Homeming_step == 0)

    {
      stepper3.setSpeed(5000);
      stepper3.runSpeed();
      Serial.println("Homeing (3) ...");
      if (digitalRead(11) == LOW) {
        stepper3.stop();
        stepper3.setCurrentPosition(0);
        Homeming_step += 1;
        Serial.println("Done (3)");
      }
    }


    while (Homeming_step == 1) {
      stepper2.setSpeed(1000);
      stepper2.runSpeed();
      Serial.println("Homeing (2) ...");
      if (digitalRead(10) == HIGH) {
        stepper2.runToNewPosition(-30);
        stepper2.run();
        stepper2.stop();
        stepper2.setCurrentPosition(0);

        Homeming_step +=1;
        Serial.println("Done (2)");
        
      }
    }


    while (Homeming_step == 2) {
      stepper1.setSpeed(1000);
      stepper1.runSpeed();
      Serial.println("Homeing (1) ...");
      if (digitalRead(9) == HIGH) {
        stepper1.runToNewPosition(3800);
        stepper1.run();
        stepper1.stop();
        stepper1.setCurrentPosition(0);

        Homeming_step = 0;
        Serial.println("Done (1)");
        Status = 0;
      }
    }
  }
}
