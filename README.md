# EGEC565_Final_Project
This is the project for my EGEC 565 project (Line Follower Robot with Auto and Manual modes) 

| Module             | Pin             | TM4C123 Pin            |
| ------------------ | --------------- | ---------------------- |
| HC-SR04 TRIG       | Trigger         | **PA4**                |
| HC-SR04 ECHO       | Echo            | **PB6 (T0CCP0)**       |
| HC-SR04 VCC        | 5V              | 5V                     |
| HC-SR04 GND        | GND             | GND                    |
| IR Line Sensor OUT | Output          | **PE0**                |
| IR Line Sensor VCC | 5V              | 5V                     |
| IR Line Sensor GND | GND             | GND                    |
| L298N IN1          | Motor A control | **PB0**                |
| L298N IN2          | Motor A control | **PB1**                |
| L298N IN3          | Motor B control | **PB2**                |
| L298N IN4          | Motor B control | **PB3**                |
| L298N ENA          | jumper ON       | FULL SPEED             |
| L298N ENB          | jumper ON       | FULL SPEED             |
| L298N +12V         | Motor power     | Battery (+)            |
| L298N GND          | Common GND      | TM4C GND + Battery GND |



┌──────────────────────────────────────────────┐
│        HC-05 Bluetooth – TM4C123 Pins        │
├──────────────────────────────────────────────┤
│ HC-05 TXD  →  TM4C PC4  (UART1 RX)           │
│ HC-05 RXD  →  TM4C PC5  (UART1 TX)           │
│ HC-05 VCC  →  5V Supply                      │
│ HC-05 GND  →  Common Ground                  │
└──────────────────────────────────────────────┘
