#include "TM4C123.h"   // CMSIS header for TM4C123

#define TRIG   (1U << 4)   // PA4
#define ECHO   (1U << 6)   // PB6 (Timer0 CCP0)
#define IR_PIN (1U << 0)   // PE0

// Motor pins - PB0 to PB3
#define IN1 (1U << 0)
#define IN2 (1U << 1)
#define IN3 (1U << 2)
#define IN4 (1U << 3)

// LED pins on Port F
#define RED_LED   (1U << 1)
#define GREEN_LED (1U << 3)

// Function prototypes
void delayMs(int n);
void init_ports(void);
void init_ultrasonic(void);
uint32_t measure_distance(void);

void motor_forward(void);
void motor_stop(void);

// ===================== MAIN =====================
int main(void)
{
    uint32_t dist;
    uint32_t ir;

    init_ports();
    init_ultrasonic();

    while (1)
    {
        dist = measure_distance();       // ultrasonic reading
        ir   = GPIOE->DATA & IR_PIN;     // IR sensor

        // --- OBSTACLE CHECK ---
        if (dist <= 25)
        {
            motor_stop();
            continue;
        }

        // --- LINE FOLLOWING ---
        if (ir == 0)   // IR detects black line
        {
            motor_forward();
        }
        else
        {
            motor_stop();
        }
    }
}

// ===================== PORT INITIALIZATION =====================
void init_ports(void)
{
    // Enable clocks: Port A, B, E, F
    SYSCTL->RCGCGPIO |= (1U<<0) | (1U<<1) | (1U<<4) | (1U<<5);

    // ===== MOTOR PINS (PB0–PB3) =====
    GPIOB->DIR |= (IN1 | IN2 | IN3 | IN4);
    GPIOB->DEN |= (IN1 | IN2 | IN3 | IN4);

    // ===== TRIGGER (PA4) =====
    GPIOA->DIR |= TRIG;
    GPIOA->DEN |= TRIG;

    // ===== ECHO (PB6) =====
    GPIOB->DIR &= ~ECHO;
    GPIOB->DEN |= ECHO;
    GPIOB->AFSEL |= ECHO;           // enable alternate function
    GPIOB->PCTL &= ~(0xF << 24);
    GPIOB->PCTL |=  (0x7 << 24);    // PB6 → T0CCP0

    // ===== IR SENSOR (PE0) =====
    GPIOE->DIR &= ~IR_PIN;
    GPIOE->DEN |= IR_PIN;

    // ===== LED PINS (PF1 RED, PF3 GREEN) =====
    GPIOF->LOCK = 0x4C4F434B;       // Unlock PF0–PF4
    GPIOF->CR = 0x1F;
    GPIOF->DIR |= RED_LED | GREEN_LED;
    GPIOF->DEN |= RED_LED | GREEN_LED;
    GPIOF->DATA &= ~(RED_LED | GREEN_LED);   // LEDs off
}

// ===================== ULTRASONIC INITIALIZATION =====================
void init_ultrasonic(void)
{
    SYSCTL->RCGCTIMER |= 1;   // Enable Timer0

    TIMER0->CTL &= ~1;
    TIMER0->CFG = 0x04;       // 16-bit mode
    TIMER0->TAMR = 0x17;      // Edge-time capture mode
    TIMER0->CTL |= 0x0C;      // Capture both edges
    TIMER0->CTL |= 1;         // Enable timer
}

// ===================== ULTRASONIC MEASUREMENT =====================
uint32_t measure_distance(void)
{
    uint32_t rising, falling, ticks;
    float time, distance_cm;

    // Trigger pulse
    GPIOA->DATA &= ~TRIG;
    delayMs(1);
    GPIOA->DATA |= TRIG;
    delayMs(1);
    GPIOA->DATA &= ~TRIG;

    // Rising edge
    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    rising = TIMER0->TAR;

    // Falling edge
    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    falling = TIMER0->TAR;

    if (falling >= rising)
        ticks = falling - rising;
    else
        ticks = rising - falling;

    time = (ticks * 62.5e-9f);     // 62.5ns tick at 16 MHz
    distance_cm = (time * 34300) / 2;

    return (uint32_t)distance_cm;
}

// ===================== MOTOR CONTROL =====================
void motor_forward(void)
{
    GPIOB->DATA = IN1 | IN3;     // Motor A forward, Motor B forward

    // LED STATUS
    GPIOF->DATA &= ~RED_LED;     // Red OFF
    GPIOF->DATA |= GREEN_LED;    // Green ON
}

void motor_stop(void)
{
    GPIOB->DATA = 0;

    // LED STATUS
    GPIOF->DATA &= ~GREEN_LED;   // Green OFF
    GPIOF->DATA |= RED_LED;      // Red ON
}

// ===================== DELAY =====================
void delayMs(int n)
{
    volatile int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 3180; j++);
}
