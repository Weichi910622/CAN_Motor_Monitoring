# CAN_Motor_Monitoring
CAN communication is currently the most common communication method used in automotive electronics.
This project implements CAN communication to transmit and receive messages in real time, allowing real-time monitoring and control of motor speed.

---

## Overview
- [Hardware Platform](#hardware-platform)
- [Sensor and Circuit Design](#sensor-and-circuit-design)
- [System Workflow](#system-workflow)
- [Processing Pipeline](#processing-pipeline)
- [Demo](#Demo)

---

## Hardware Platform

### STM32F103C8T6 Development Board for 3
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/aa7097f9-7a91-48aa-a5fe-5a4d830a329f" />

---

## Sensor and Circuit Design

Key components include:
- CAN Communication Module
- RedSensor for counting the real RPM
- Motor
- Motor Driver Module
- OLED
- Key

<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/4a4441ff-9b04-4706-97bb-04badf1ea1a3" />

---

## System Workflow

### CAN(Controller Area Network)

<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/e247591e-3e25-4d7a-b4f2-890aa4cdaa7f" />

### Speed Monitoring
- Control node transmits target speed by CAN.
- Motor and Monitor receive target speed by CAN.
- Count is detected by RedSensor.
- For the three fans, changes count to RPM.
- Motor transmits real speed by CAN.
- Monitor receives and shows the real and target speed.

### PID(Proportional-Integral-Derivative)

<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/f2104576-5ba6-4886-9b86-deb1f6a0e455" />

---

## Processing Pipeline

1. **Initialization Phase**
   
Control node changes target speed by key and transmits target speed to Motor and Monitor.

2. **Motor run**
   
Motor receives target speed and runs.

3. **Detection Start**
   
RedSensor monitors the real-time counts and changes to RPM.

4. **Close-Loop**
   
Controlling motor speed by PID.

5. **Monitoring**
   
Motor transmits real speed to Monitor.
---

## Demo
[![Demo](https://img.youtube.com/vi/LiPS_F5W8KQ/hqdefault.jpg)](https://youtube.com/shorts/LiPS_F5W8KQ?si=GE4byRCfGZH5ECCj)
