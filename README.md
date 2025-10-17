# CAN_Motor_Monitoring
CAN communication is currently one of the most common communication methods used in automotive electronics.  
This project implements CAN communication to transmit and receive messages in real time, enabling real-time monitoring and control of motor speed.

---

## Overview
- [Hardware Platform](#hardware-platform)
- [Sensor and Circuit Design](#sensor-and-circuit-design)
- [System Workflow](#system-workflow)
- [Processing Pipeline](#processing-pipeline)
- [Demo](#demo)

---

## Hardware Platform

### STM32F103C8T6 Development Board  
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/aa7097f9-7a91-48aa-a5fe-5a4d830a329f" />

---

## Sensor and Circuit Design

Key components include:
- CAN communication module  
- RedSensor for counting the actual RPM  
- Motor  
- Motor driver module  
- OLED display  
- Key input  

<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/4a4441ff-9b04-4706-97bb-04badf1ea1a3" />

---

## System Workflow

### CAN (Controller Area Network)
<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/e247591e-3e25-4d7a-b4f2-890aa4cdaa7f" />

### Speed Monitoring
- The **control node** transmits the target speed via CAN.  
- The **motor** and **monitor** nodes receive the target speed via CAN.  
- The **RedSensor** detects the pulse count.  
- For the three fans, the pulse count is converted to RPM.  
- The **motor node** transmits the actual speed via CAN.  
- The **monitor node** receives and displays both the target and actual speeds.

### PID (Proportional–Integral–Derivative)
<img width="600" height="400" alt="image" src="https://github.com/user-attachments/assets/f2104576-5ba6-4886-9b86-deb1f6a0e455" />

---

## Processing Pipeline

1. **Initialization Phase**  
   The control node adjusts the target speed using the key and transmits it to both the motor and monitor nodes.

2. **Motor Operation**  
   The motor receives the target speed and begins running.

3. **Detection Phase**  
   The RedSensor monitors the real-time pulse count and converts it to RPM.

4. **Closed-Loop Control**  
   The motor speed is regulated using a PID controller.

5. **Monitoring Phase**  
   The motor transmits the actual speed to the monitor for display.
---

## Demo
[![Demo](https://img.youtube.com/vi/LiPS_F5W8KQ/hqdefault.jpg)](https://youtube.com/shorts/LiPS_F5W8KQ?si=GE4byRCfGZH5ECCj)
