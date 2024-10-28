/*
 * CH32V003 + WS2812B demo
 * Strip on PC4 (pin 7 on so8 package)
 *
 *
 *
 */


// For SWIO programming
#include "debug.h"

#define TEST2
// Define the number of LEDs in the strip
#define num_leds 60

// Include the sine and rand lookup tables
#include "lookups.h"

// Include the Green Dragon Bitbanging driver
#include "GD_WS2812_DRIVER.h"

// Include the colour and animation functions
#include "LED_functions.h"
#include "LED_functions_nd.h"
#include "lis2dw12_reg.h"
#include "cordic_atan2.h"
#include "prand.h"

void i2c_ReadMultipleBytes(uint8_t DevAddr, uint8_t Reg, uint16_t length, uint8_t* buffer);
void i2c_WriteMultipleBytes(uint8_t DevAddr, uint8_t Reg, uint16_t length,const uint8_t* buffer);


void timer2_init(void);
void i2c_init(void);

void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));


void glitter(void);
void demo(void);
void gravity(void);



stmdev_ctx_t lis12_ctx;

volatile int ready_for_show = 0;
volatile uint_fast8_t trig10ms=0, trigsw=0;
volatile int swcooldown=10;

const int state_sz = 3;
enum State {S_glitter=0, S_demo=1, S_gravity=2} s_state=2;

uint8_t debug_i2c=0;

int random_seed=0;

void timer2_init() {
    //Switch button initialize
    //Systick initialize
    TIM_TimeBaseInitTypeDef TIMBase_InitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    //1ms period = 1khz. update interrupt every 10ms.
    TIMBase_InitStruct.TIM_Prescaler = 24 - 1;  //for 24MHz clock
    TIMBase_InitStruct.TIM_Period = 10000 - 1;
    TIMBase_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIMBase_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIMBase_InitStruct);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //MOST IMPORTANT LINE!
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    TIM_Cmd(TIM2, ENABLE);
}

void i2c_init(void){
    /*
     * Pins:
     * SDA  5 PC1
     * SCL  6 PC2
     *
     * 400KHz
     *
     * Device address (7-bit): 0x19
     */

    //GPIO Alternate function open drain configuration
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );

    GPIO_InitTypeDef GPIO_InitStructure={0};
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    //I2C peripheral configuration
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    I2C_InitTypeDef I2C_InitTSturcture={0};

    I2C_InitTSturcture.I2C_ClockSpeed = 400000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = 0x19;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );

    I2C_Cmd( I2C1, ENABLE );

    I2C_AcknowledgeConfig( I2C1, ENABLE );



}

uint8_t i2c_ReadOneByte(uint8_t ReadAddr)
{

    const uint8_t dev_addr = (0x19<<1);
    uint8_t temp=0;

    //wait for I2C to be ready
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );

    //Start Condition
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    //Send device address with write bit
    I2C_Send7bitAddress( I2C1, dev_addr, I2C_Direction_Transmitter );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    //Send 1B of data - register address
    I2C_SendData( I2C1, ReadAddr );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

    //repeated start
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    //Send device address with read bit
    I2C_Send7bitAddress( I2C1, dev_addr, I2C_Direction_Receiver );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

    //wait for the data to arrive
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_RXNE ) ==  RESET )


        I2C_AcknowledgeConfig( I2C1, DISABLE );

    temp = I2C_ReceiveData( I2C1 );
    I2C_GenerateSTOP( I2C1, ENABLE );

    return temp;
}

void i2c_ReadMultipleBytes(uint8_t DevAddr, uint8_t Reg, uint16_t length, uint8_t* buffer)
{
    //wait for I2C to be ready
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );

    //Start Condition
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    //Send device address with write bit
    I2C_Send7bitAddress( I2C1, DevAddr, I2C_Direction_Transmitter );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    //Send 1B of data - register address
    I2C_SendData( I2C1, Reg );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

    //repeated start
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    //Send device address with read bit
    I2C_Send7bitAddress( I2C1, DevAddr, I2C_Direction_Receiver );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

    I2C_AcknowledgeConfig( I2C1, ENABLE );

    for(int i=0; i<length-1;++i)
    {
        //wait for the data to arrive
        while( I2C_GetFlagStatus( I2C1, I2C_FLAG_RXNE ) ==  RESET );
        buffer[i] = I2C_ReceiveData( I2C1 );

    }
    //for the last byte
    I2C_AcknowledgeConfig( I2C1, DISABLE );
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_RXNE ) ==  RESET );
    buffer[length-1] = I2C_ReceiveData( I2C1 );



    I2C_GenerateSTOP( I2C1, ENABLE );

    return;
}


//DeviceAddress in standard 7-bit format (without right-shift)
uint8_t i2c_checkSlave(uint8_t DeviceAddress){
    uint8_t ret = 0;

    //wait for I2C to be ready
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );

    //Start Condition
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    //Send device address with write bit
    I2C_Send7bitAddress( I2C1, (DeviceAddress<<1), I2C_Direction_Transmitter );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    //wait for the data to arrive
    ret = I2C_GetFlagStatus( I2C1, I2C_FLAG_ADDR );

    I2C_GenerateSTOP( I2C1, ENABLE );


    return ret;
}


void i2c_WriteOneByte(uint8_t WriteAddr, uint8_t DataToWrite)
{
    const uint8_t dev_addr = (0x19<<1);

    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );
    I2C_GenerateSTART( I2C1, ENABLE );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress( I2C1, dev_addr, I2C_Direction_Transmitter );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );


    I2C_SendData( I2C1, WriteAddr );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );



    if( I2C_GetFlagStatus( I2C1, I2C_FLAG_TXE ) !=  RESET )
    {
        I2C_SendData( I2C1, DataToWrite );
    }

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    I2C_GenerateSTOP( I2C1, ENABLE );
}

void i2c_WriteMultipleBytes(uint8_t DevAddr, uint8_t Reg, uint16_t length,const uint8_t* buffer)
{
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET );
    I2C_GenerateSTART( I2C1, ENABLE );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress( I2C1, DevAddr, I2C_Direction_Transmitter );

    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );


    I2C_SendData( I2C1, Reg );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );


    for(int i=0;i<length;++i){
        if( I2C_GetFlagStatus( I2C1, I2C_FLAG_TXE ) !=  RESET )
        {
            I2C_SendData( I2C1, buffer[i] );
        }
        while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    }


    I2C_GenerateSTOP( I2C1, ENABLE );
}

void iwdg_init() {
    //LSI for IWDG
    RCC->RSTSCKR = RCC_LSION;
    while ((RCC->RSTSCKR & RCC_FLAG_LSIRDY) != SET);

    //IWDG config
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_16);
    IWDG_SetReload(4000);
    IWDG_ReloadCounter();
    IWDG_Enable();
    IWDG_ReloadCounter();
}

int32_t platform_write(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len)
{
    const uint8_t dev_addr = (0x19<<1);
    i2c_WriteMultipleBytes(dev_addr, Reg, len, Bufp);
    return 0;
}
int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
    const uint8_t dev_addr = (0x19<<1);
    i2c_ReadMultipleBytes(dev_addr, Reg, len, Bufp);
    return 0;
}


// The main function initialises the Delay and GPIO, sets the initial color values of the LED strip to off
int main(void)
{
    SystemCoreClockUpdate();
    // Initialise delay
    Delay_Init();


    // configures GPIOC pin 4 as Output Push/Pull for data out
    GPIO_INITZ();
    Delay_Ms(5);
    // Send initial LED colour values to the LED strip
    LED_OFF();
    clear();


    //When the last reset was caused by IWDG, turn on two red leds for a moment after a startup
    if(RCC->RSTSCKR & RCC_IWDGRSTF){
        setPixelColor(10, 20, 0, 0);
        setPixelColor(40, 20, 0, 0);
        SHOWTIME(BUFFER_LEDS);
        Delay_Ms(500);
        LED_OFF();
        clear();
    }

    SHOWTIME(BUFFER_LEDS);
#ifdef TEST2
    iwdg_init();
    //Delay_Ms(400);
#endif
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    //Switch button initialize PA2
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,  GPIO_PinSource2);

    EXTI_InitTypeDef EXTI_InitStructure = {0};
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_EnableIRQ(EXTI7_0_IRQn);
#ifdef TEST2
    //TIM2 as a Systick initialize
    timer2_init();
#endif
    //I2C initialize
    i2c_init();

    uint8_t i2cbuf[10];
    i2c_WriteOneByte(0x21, (1<<6)|(1<<2)); //soft reset
    Delay_Ms(10);
    i2c_WriteOneByte(0x21, (1<<7)|(1<<2) ); //ctrl2, boot
    Delay_Ms(1);
    i2c_WriteOneByte(0x20, (2<<4) ); //ctrl1, Low-Power mode 12.5 Hz
    Delay_Ms(1);

    i2c_ReadMultipleBytes((0x19<<1), 0x26, 6, i2cbuf);



    //LIS2DW12 initialize
    lis12_ctx.handle = I2C1;
    lis12_ctx.write_reg = platform_write;
    lis12_ctx.read_reg = platform_read;

    random_seed = (int)i2c_ReadOneByte(0x28);

    // Loop
    while (1) {
        //pushbutton switch event
        if(1 == trigsw){

            swcooldown = 10;
            s_state = (s_state+1) % state_sz;

            trigsw = 0;
        }
        //systick timer event
        if(1 == trig10ms){
            //software timer 1ms;
            if(0 < swcooldown) --swcooldown;

            //state machine
            switch (s_state) {
                case S_glitter:
                    glitter();
                    break;
                case S_demo:
                    demo();
                    break;
                case S_gravity:
                    gravity();
                    break;
                default:
                    glitter();
                    break;
            }


            //disable interrupts before
            NVIC_DisableIRQ(EXTI7_0_IRQn);
            SHOWTIME(BUFFER_LEDS); //1.8 ms for transmission
            NVIC_EnableIRQ(EXTI7_0_IRQn);
            trig10ms = 0;
        }

        //watchdog reset
        IWDG_ReloadCounter();
        __WFI(); //sleep






    } // end of while
} // end of Main

void TIM2_IRQHandler(void) {

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        trig10ms = 1;

        // this can be replaced with your code of flag so that in main's that flag can be handled
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //
    }
}

/*********************************************************************
 * @fn      EXTI0_IRQHandler
 *
 * @brief   This function handles EXTI0 Handler.
 *
 * @return  none
 */
void EXTI7_0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2)!=RESET)
    {
        if(0 == swcooldown) trigsw = 1;

        EXTI_ClearITPendingBit(EXTI_Line2);     /* Clear Flag */
    }
}

//Functions that contains animations. Each function should be executed in less than 8ms.
//Functions should store internal state in static types.


void demo(void){
    /*
    hCurtain1(32,12,0,5);
    Delay_Ms(300);
    hCurtain2(0,32,12,5);
    Delay_Ms(300);
    vCurtain1(12,0,32,1);
    Delay_Ms(300);
    vCurtain2(32,4,7,1);
    Delay_Ms(300);

    clear();

    for(uint_fast8_t i=0; i<5;++i)
    {
        fadeIn(4);
        fadeOut(4);
    }
    clear();
    eight(64,64,0,10);

    eight(0,64,0,10);
    eight(0,64,64,10);
    eight2(32,32,0,30);
    eight2(0,32,0,30);
    eight2(0,32,32,30);
    fadeOut(4);
    eight3(32,0,8,30);
    eight3b(8,0,32,30);
    clear();
    fill1(32,32,0,50);
    fill2(0,32,0,50);
    fill1(0,32,32,50);
    fill2(32,32,0,50);
    LED_RAINBOWS(20,60); // variables are delay speed in mS and width (number of leds)
    theaterChase2(32, 32, 32, 100, 6, 20); // White
    theaterChase2(32, 2, 1, 50, 4, 20); // Red
    theaterChase2(2, 3, 32, 50, 3, 20); // Blue

    */
    //after power-up start from some random animation step.
    static int animation_step = 2137;
    if(animation_step == 2137){
        animation_step = prand8(&random_seed) % 38;
    }

    switch (animation_step) {
    case 0:
        if(hCurtain1_nd(32,12,0,1) == 0) ++animation_step;
        break;
    case 1:
        if(delay_nd(30) == 0) ++animation_step;
        break;
    case 2:
        if(hCurtain2_nd(0,32,12,1) == 0) ++animation_step;
        break;
    case 3:
        if(delay_nd(30) == 0) ++animation_step;
        break;
    case 4:
        if(vCurtain1_nd(12,0,32,1) == 0) ++animation_step;
        break;
    case 5:
        if(delay_nd(30) == 0) ++animation_step;
        break;
    case 6:
        if(vCurtain2_nd(32,4,7,1) == 0) ++animation_step;
        break;
    case 7:
        if(delay_nd(30) == 0) ++animation_step;
        break;
    case 8:
        clear(); ++animation_step;
        break;
    case 9:
    case 11:
    case 13:
    case 15:
        if(fadeIn_nd(1) == 0) ++animation_step;
        break;
    case 10:
    case 12:
    case 14:
    case 16:
        if(fadeOut_nd(1) == 0) ++animation_step;
        break;
    case 17:
        clear(); ++animation_step;
        break;
    case 18:
        if(eight_nd(64,64,0,2) == 0) ++animation_step;
        break;
    case 19:
        if(eight_nd(0,64,0,2) == 0) ++animation_step;
        break;
    case 20:
        if(eight_nd(0,64,64,2) == 0) ++animation_step;
        break;
    case 21:
        if(eight2_nd(32,32,0,2) == 0) ++animation_step;
        break;
    case 22:
        if(eight2_nd(0,32,0,3) == 0) ++animation_step;
        break;
    case 23:
        if(eight2_nd(0,32,32,3) == 0) ++animation_step;
        break;
    case 24:
        if(eight3_nd(32,0,8,5) == 0) ++animation_step;
        break;
    case 25:
        if(eight3b_nd(8,0,32,5) == 0) ++animation_step;
        break;
    case 26:
        clear(); ++animation_step;
        break;
    case 27:
        if(fill1_nd(32,32,0,5) == 0) ++animation_step;
        break;
    case 28:
        if(fill2_nd(0,32,0,5) == 0) ++animation_step;
        break;
    case 29:
        if(fill1_nd(0,32,32,5) == 0) ++animation_step;
        break;
    case 30:
        if(fill2_nd(32,32,0,5) == 0) ++animation_step;
        break;
    case 31: //rainbow full
    case 32:
    case 33:
    case 34:
        if(rainbow_nd(1) == 0) ++animation_step;
        break;
    case 35: //white theater chase
        if(theaterChase2_nd(32, 32, 32, 10, 6, 10) == 0) ++animation_step;
        break;
    case 36: //white theater chase
        if(theaterChase2_nd(32, 2, 1, 5, 4, 20) == 0) ++animation_step;
        break;
    case 37: //white theater chase
        if(theaterChase2_nd(2, 3, 32, 5, 3, 20) == 0) ++animation_step;
        break;

    default:
        animation_step = 0;
        break;
    }



}

void gravity(void){

    unsigned char angle=12;
    unsigned int radius=0;

    unsigned char led_pos=0, remainder=0;

    int16_t gravityX = ((int16_t)i2c_ReadOneByte(0x29))* 256 + (int16_t)i2c_ReadOneByte(0x28);//OUT_X
    int16_t gravityY = ((int16_t)i2c_ReadOneByte(0x2B))* 256 + (int16_t)i2c_ReadOneByte(0x2A);//OUT_Y
    int16_t gravityZ = ((int16_t)i2c_ReadOneByte(0x2D))* 256 + (int16_t)i2c_ReadOneByte(0x2C);//OUT_Z

    if(gravityZ < -15000){ clear(); return;}

    atan2sqrt(gravityX, gravityY * -1, &angle, &radius); //returns 0-255 angle value
    if(radius <5000){ clear(); return;}
    if(angle <56){ clear(); return;}
    if(angle >198){ clear(); return;}
    //angle is divided by 8 to adjust for smaller count of leds. The remainder from range 0..7 is used
    //to set the brightness of current and next pixel to make animation more smooth
    led_pos = angle / 8;
    remainder = angle - led_pos * 8;
    led_pos += 4;

    clear();
    setPixelColor(led_pos, 0, 8-remainder, 8-remainder); //left eye
    setPixelColor(led_pos+1, 0, remainder, remainder); //left eye

    setPixelColor(39+((num_leds-led_pos) % 30), 0, 8-remainder, 8-remainder); //right eye
    setPixelColor(39+((num_leds-(led_pos+1)) % 30), 0, remainder, remainder); //right eye
}

void glitter(void){
    clear();
    if (prand8(&random_seed) > 230) {
        setPixelColor( prand8(&random_seed)%num_leds, 14, 14, 16);
    }

}


