#include "inc/tm4c123gh6pm.h"

#include <stdio.h>
#include <stdbool.h>

#include <stdint.h>
#define MULTIPLIER 0.0343


// function prototypes for the motor
void movestop(void);
void moveforward(void);
void movebackward(void);
void turnright(void);
void turnleft(void);
void portB_init(void);

//function prototypes for ultrasonic sensor
#define ECHO (1U<<6) //PB6
#define TRIG (1U<<4) //PA4(OUTPUT)
#define BLUE_LED (1U<<2) //PF2 BLUE LED

static uint32_t highEdge,lowEdge;
static uint32_t ddistance;
static const double _16MHz_1clock = 62.5e-9;
void Delay_MicroSecond(uint32_t time);
void Timer0_init(void);
void portA_init(void);
uint32_t measureD(void);

//static uint32_t distance_in_cm;

//function prototypes for servo
#define PA3 0x08
void servo_init(int pin);
void servo_45_degree(int pin);
void servo_90_degree(int pin);
void servo_135_degree(int pin);

//miscellaneous functions
uint32_t lookright(int pin);
uint32_t read_distance(void);
uint32_t lookleft(int pin);

//variables
#define maximum_distance 200
 static bool goesforward =false;
 static uint32_t distance=100;

int main(void) {

    //initialisation of the motors
    portB_init();

    // initialization of the ultrasonic sensor
    portA_init();
    Timer0_init();
    //initialization for the servo
    servo_init(PA3);

    servo_90_degree(PA3);
    Delay_MicroSecond(2000000);

    distance=read_distance();
    Delay_MicroSecond(100000);

    distance=read_distance();
    Delay_MicroSecond(100000);

    distance=read_distance();
    Delay_MicroSecond(100000);

    distance=read_distance();
    Delay_MicroSecond(100000);

    while(1) {
        uint32_t distanceright=0;
        uint32_t distanceleft=0;
        Delay_MicroSecond(50000);

        if(distance<=20)
        {
            movestop();
            Delay_MicroSecond(300000);
            movebackward();
            Delay_MicroSecond(40000);
            movestop();
            Delay_MicroSecond(300000);
            distanceright=lookright(PA3);
            Delay_MicroSecond(300000);
            distanceleft=lookleft(PA3);
            Delay_MicroSecond(300000);

            if(distance>=distanceleft)
            {
                turnright();
                movestop();

            }else
            {
                turnleft();
                movestop();

            }

        }else
        {
            moveforward();
        }

        distance=read_distance();
    }

}

//miscallaneous functions
uint32_t lookright(int pin)
{
    servo_45_degree(pin);
    Delay_MicroSecond(500000);
    uint32_t distance=read_distance();
    Delay_MicroSecond(100000);
    servo_90_degree(pin);
    return distance;
}
uint32_t lookleft(int pin)
{
    servo_135_degree(pin);
    Delay_MicroSecond(500000);
    uint32_t distance=read_distance();
    Delay_MicroSecond(100000);
    servo_90_degree(pin);
    return distance;
}

uint32_t read_distance(void){
    Delay_MicroSecond(70000);
    uint32_t cm=measureD();
    if(cm==0){
        cm=250;
    }
    return cm;
}



//functions for motor

void portB_init(void){
    SYSCTL_RCGC2_R |= 0x02;
    ;
    ;
    ;
  //port B initialization
    GPIO_PORTB_LOCK_R=GPIO_LOCK_KEY;
    GPIO_PORTB_CR_R=0x0E;

    GPIO_PORTB_AFSEL_R |=0x0;
    GPIO_PORTB_DEN_R |=0x0F;
    GPIO_PORTB_DIR_R |=0x0F;

}

void left_motor_forward(void){
    //set PB0
GPIO_PORTB_DATA_R |=0x01;
//clear PB1
GPIO_PORTB_DATA_R &= (~0x02);
}

void left_motor_backward(void){
    //clear PB0
    GPIO_PORTB_DATA_R &=(~0x01);
    //set PB1
    GPIO_PORTB_DATA_R |= 0x02;

}

void left_motor_stop(void){
    //clear PB0,PB1
    GPIO_PORTB_DATA_R &=(~0x03);
}

void right_motor_forward(void){
    //set PB2
GPIO_PORTB_DATA_R |=0x04;
//clear PB3
GPIO_PORTB_DATA_R &= (~0x08);
}

void right_motor_backward(void){
    //set PB3
GPIO_PORTB_DATA_R |=0x08;
//clear PB2
GPIO_PORTB_DATA_R &= (~0x04);
}

void right_motor_stop(void){
    //clear PB3,PB2
    GPIO_PORTB_DATA_R &=(~0x0C);
}

void movestop(){
    left_motor_stop();
    right_motor_stop();
}

void moveforward() {

    left_motor_forward();
    right_motor_forward();

}


    void movebackward() {

    left_motor_backward();
    right_motor_backward();

}

void turnright(){
    left_motor_forward();
    right_motor_backward();
}

void turnleft() {
    right_motor_forward();
    left_motor_backward();
}



//functions for servo

void servo_init(int pin){
    SYSCTL_RCGC2_R |=0x01;
    GPIO_PORTA_DIR_R |=pin;
    GPIO_PORTA_DEN_R |=pin;
}

//use timer1A for delay
void Delay_MicroSecond(uint32_t time){
    int i;
    SYSCTL_RCGCTIMER_R |=0x02;
    TIMER1_CTL_R =0x0;
    TIMER1_CFG_R =0x04;
    TIMER1_TAMR_R =0x02;
    TIMER1_TAILR_R =16-1;
    TIMER1_ICR_R =0x1;
    TIMER1_CTL_R |=0x1;
    for(i=0;i<time;i++){
        while((TIMER1_RIS_R & 0x01)==0);
        TIMER1_ICR_R=0x1;
    }

}

void servo_45_degree(int pin){
    int i;
    for(i=0;i<50;i++){
    GPIO_PORTA_DATA_R |=pin;
    Delay_MicroSecond(2100);
    GPIO_PORTA_DATA_R &=~(pin);
    Delay_MicroSecond(17900);
    }

}

//straight

void servo_90_degree(int pin){
    int i;
    for(i=0;i<50;i++){
    GPIO_PORTA_DATA_R |=pin;
    Delay_MicroSecond(3200);
    GPIO_PORTA_DATA_R &=~(pin);
    Delay_MicroSecond(16800);
    }

}

void servo_135_degree(int pin){
    int i;
    for(i=0;i<50;i++){
    GPIO_PORTA_DATA_R |=pin;
    Delay_MicroSecond(43000);
    GPIO_PORTA_DATA_R &=~(pin);
    Delay_MicroSecond(15700);
    }

}



//functions for ultrasonic sensor

void Timer0_init(void){
    SYSCTL_RCGCTIMER_R |=(1U<<0);
    SYSCTL_RCGC2_R |=(1U<<1);
    GPIO_PORTB_DIR_R &=~ECHO;
    GPIO_PORTB_DEN_R |=ECHO;
    GPIO_PORTB_AFSEL_R |=ECHO;
    GPIO_PORTB_PCTL_R &=~0x0F000000;
    GPIO_PORTB_PCTL_R |=0x07000000;

    TIMER0_CTL_R &=~1;
    TIMER0_CFG_R =0x04;
    TIMER0_TAMR_R =0x17;
    TIMER0_CTL_R |=0x0C;
    TIMER0_CTL_R |=1;
}

void portA_init(void){
SYSCTL_RCGC2_R |=(1U<<0);
GPIO_PORTA_DIR_R |=TRIG;
GPIO_PORTA_DEN_R |=TRIG;
}

uint32_t measureD(void)
{
    GPIO_PORTA_DATA_R &= ~TRIG;
    Delay_MicroSecond(12);
    GPIO_PORTA_DATA_R |=TRIG;
    Delay_MicroSecond(12);
    GPIO_PORTA_DATA_R &= ~TRIG;

    TIMER0_ICR_R=4;
    while((TIMER0_RIS_R & 4)==0){};
        if(GPIO_PORTB_DATA_R & (1<<6)){
            highEdge=TIMER0_TAR_R;
            TIMER0_ICR_R=4;

            while((TIMER0_RIS_R & 4)==0){};
            lowEdge=TIMER0_TAR_R;
                ddistance=lowEdge-highEdge;
                ddistance = _16MHz_1clock * (double)MULTIPLIER * ddistance;

            }
        return ddistance;
        }
