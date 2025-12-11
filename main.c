#include "TM4C123.h"

#define TRIG   (1U << 4)   // PA4
#define ECHO   (1U << 6)   // PB6
#define IR_PIN (1U << 0)   // PE0
#define Bus16_000MHz 24
// Motor pins
#define IN1 (1U << 0)
#define IN2 (1U << 1)
#define IN3 (1U << 2)
#define IN4 (1U << 3)

// LEDs on Port F
#define RED_LED   (1U << 1)
#define GREEN_LED (1U << 3)

// Function prototypes
void delayMs(int n);
void init_ports(void);
void init_ultrasonic(void);
uint32_t measure_distance(void);

void motor_forward(void);
void motor_stop(void);

void uart5_init(void);
void uart5_sendChar(char c);
void uart5_sendString(char *str);
char uart5_readChar(void);

// Modes
#define MODE_AUTONOMOUS 0
#define MODE_MANUAL     1

int mode = MODE_AUTONOMOUS;

// ==========================================================
// MAIN
// ==========================================================
int main(void)
{
    uint32_t dist, ir;
    char rx;

    init_ports();
    init_ultrasonic();
    uart5_init();

    uart5_sendString("System Ready. Mode = Autonomous\r\n");

    while (1)
    {
        // -------- Bluetooth Check ----------
        if (!(UART5->FR & (1U << 4)))
        {
            rx = uart5_readChar();

            if (rx == 'A') {
                mode = MODE_AUTONOMOUS;
                uart5_sendString("Switched to Autonomous Mode\r\n");
            }
            else if (rx == 'M') {
                mode = MODE_MANUAL;
                uart5_sendString("Switched to Manual Mode\r\n");
            }
            else if (mode == MODE_MANUAL)
            {
                if (rx == '1') {
                    motor_forward();
                    uart5_sendString("Car moving forward\r\n");
                }
                else if (rx == '2') {
                    motor_stop();
                    uart5_sendString("Car stopping\r\n");
                }
            }
        }

        // -------- Autonomous Mode ----------
        if (mode == MODE_AUTONOMOUS)
        {
            dist = measure_distance();
            ir   = GPIOE->DATA & IR_PIN;

            if (dist <= 25) {
                motor_stop();
                continue;
            }

            if (ir == 0) {
                motor_forward();
            } else {
                motor_stop();
            }
        }
    }
}

// ==========================================================
// PORT INITIALIZATION
// ==========================================================
void init_ports(void)
{
    SYSCTL->RCGCGPIO |= (1U<<0) | (1U<<1) | (1U<<4) | (1U<<5);

    // Motor pins PB0–PB3
    GPIOB->DIR |= (IN1 | IN2 | IN3 | IN4);
    GPIOB->DEN |= (IN1 | IN2 | IN3 | IN4);

    // TRIG PA4
    GPIOA->DIR |= TRIG;
    GPIOA->DEN |= TRIG;

    // ECHO PB6
    GPIOB->DIR &= ~ECHO;
    GPIOB->DEN |= ECHO;
    GPIOB->AFSEL |= ECHO;
    GPIOB->PCTL &= ~(0xF << 24);
    GPIOB->PCTL |=  (0x7 << 24);

    // IR PE0
    GPIOE->DIR &= ~IR_PIN;
    GPIOE->DEN |= IR_PIN;

    // LEDs PF1 (Red) & PF3 (Green)
    SYSCTL->RCGCGPIO |= (1U << 5);  // Enable Port F clock
    GPIOF->DIR |= RED_LED | GREEN_LED;
    GPIOF->DEN |= RED_LED | GREEN_LED;
}

// ==========================================================
// ULTRASONIC
// ==========================================================
void init_ultrasonic(void)
{
    SYSCTL->RCGCTIMER |= 1;

    TIMER0->CTL &= ~1;
    TIMER0->CFG = 0x04;
    TIMER0->TAMR = 0x17;
    TIMER0->CTL |= 0x0C;
    TIMER0->CTL |= 1;
}

uint32_t measure_distance(void)
{
    uint32_t rising, falling, ticks;
    float time, distance_cm;

    GPIOA->DATA &= ~TRIG;
    delayMs(1);
    GPIOA->DATA |= TRIG;
    delayMs(1);
    GPIOA->DATA &= ~TRIG;

    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    rising = TIMER0->TAR;

    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    falling = TIMER0->TAR;

    ticks = (falling > rising) ? (falling - rising) : (rising - falling);

    time = ticks * 62.5e-9f;
    distance_cm = (time * 34300) / 2;

    return (uint32_t)distance_cm;
}

// ==========================================================
// MOTOR CONTROL (with LEDs)
// ==========================================================
void motor_forward(void)
{
    GPIOB->DATA = IN1 | IN3;

    // Green ON, Red OFF
    GPIOF->DATA = GREEN_LED;
}

void motor_stop(void)
{
    GPIOB->DATA = 0;

    // Red ON, Green OFF
    GPIOF->DATA = RED_LED;
}

// ==========================================================
// UART5 (HC-05 Bluetooth)
// ==========================================================
void uart5_init(void)
{
    SYSCTL->RCGCUART |= (1U << 5);
    SYSCTL->RCGCGPIO |= (1U << 2);

    GPIOC->AFSEL |= (1U << 4) | (1U << 5);
    GPIOC->PCTL &= ~0x00FF0000;
    GPIOC->PCTL |=  0x00110000;
    GPIOC->DEN |= (1U << 4) | (1U << 5);

    UART5->CTL &= ~1;
    UART5->IBRD = 104;
    UART5->FBRD = 11;
    UART5->LCRH = 0x70;
    UART5->CTL |= 0x301;
}

void uart5_sendChar(char c)
{
    while (UART5->FR & (1U << 5));
    UART5->DR = c;
}

void uart5_sendString(char *str)
{
    while (*str)
        uart5_sendChar(*str++);
}

char uart5_readChar(void)
{
    while (UART5->FR & (1U << 4));
    return UART5->DR;
}

// ==========================================================
// DELAY
// ==========================================================
void delayMs(int n)
{
    int i, j;
    for (i=0; i<n; i++)
        for (j=0; j<3180; j++);
}
