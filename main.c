#include "TM4C123.h"   // Using Keil CMSIS-style header

#define TRIG   (1U << 4)   // PA4
#define ECHO   (1U << 6)   // PB6 (Timer0 CCP0)
#define IR_PIN (1U << 0)   // PE0

// Motor pins on Port B (L298N)
#define IN1 (1U << 0)
#define IN2 (1U << 1)
#define IN3 (1U << 2)
#define IN4 (1U << 3)

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
        dist = measure_distance();
        ir   = GPIOE->DATA & IR_PIN;

        // Stop if obstacle is close
        if (dist <= 25)
        {
            motor_stop();
            continue;
        }

        // Line following: IR = 0 means BLACK line
        if (ir == 0)
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
    // Enable clocks
    SYSCTL->RCGCGPIO |= (1U<<0) | (1U<<1) | (1U<<4) | (1U<<5);

    // ===== MOTOR (PB0–PB3) =====
    GPIOB->DIR |= (IN1 | IN2 | IN3 | IN4);
    GPIOB->DEN |= (IN1 | IN2 | IN3 | IN4);

    // ===== TRIGGER (PA4) =====
    GPIOA->DIR |= TRIG;
    GPIOA->DEN |= TRIG;

    // ===== ECHO (PB6) =====
    GPIOB->DIR &= ~ECHO;
    GPIOB->DEN |= ECHO;
    GPIOB->AFSEL |= ECHO;
    GPIOB->PCTL &= ~(0xF << 24);
    GPIOB->PCTL |=  (0x7 << 24);  // T0CCP0 function

    // ===== IR SENSOR (PE0) =====
    GPIOE->DIR &= ~IR_PIN;
    GPIOE->DEN |= IR_PIN;
}

// ===================== ULTRASONIC INITIALIZATION =====================
void init_ultrasonic(void)
{
    SYSCTL->RCGCTIMER |= 1;   // Enable Timer0

    TIMER0->CTL &= ~1;
    TIMER0->CFG = 0x04;       // 16-bit mode
    TIMER0->TAMR = 0x17;      // Edge-time, capture
    TIMER0->CTL |= 0x0C;      // Both edges
    TIMER0->CTL |= 1;         // Enable timer
}

// ===================== ULTRASONIC READ =====================
uint32_t measure_distance(void)
{
    uint32_t rising, falling, ticks;
    float time, distance_cm;

    // Send trigger pulse
    GPIOA->DATA &= ~TRIG;
    delayMs(1);
    GPIOA->DATA |= TRIG;
    delayMs(1);
    GPIOA->DATA &= ~TRIG;

    // Wait rising edge
    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    rising = TIMER0->TAR;

    // Wait falling edge
    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    falling = TIMER0->TAR;

    // Compute ticks
    if (falling > rising)
        ticks = falling - rising;
    else
        ticks = rising - falling;

    // Convert to time
    time = (ticks * 62.5e-9f);   // 1 tick = 62.5 ns
    distance_cm = (time * 34300) / 2;

    return (uint32_t)distance_cm;
}

// ===================== MOTOR CONTROL =====================
void motor_forward(void)
{
    GPIOB->DATA = IN1 | IN3;   // IN1=1 IN2=0, IN3=1 IN4=0
}

void motor_stop(void)
{
    GPIOB->DATA = 0;
}

// ===================== DELAY =====================
void delayMs(int n)
{
    int i, j;
    for (i=0; i<n; i++)
        for (j=0; j<3180; j++)
            ;
}
