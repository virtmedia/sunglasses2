/*
 * lookups.h
 *
 *  Created on: 27 Aug 2023
 *      Author: User
 *
 *      non-blocking animation functions. If the function returns 0,
 *      it means that animation made full cycle.
 */

#ifndef LED_FUNCTIONS_ND_H_
#define LED_FUNCTIONS_ND_H_

#include "GD_WS2812_DRIVER.h"

int delay_nd(int cycles){
    static int int_state = 0;

     if(0 < (cycles - int_state) ) ++int_state;
     else int_state = 0;

    return int_state;
}

int fadeIn_nd(uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait;

    for(uint_fast8_t j=0;j<num_leds;++j){
        setPixelColor(j, i,i,i);
    }

    ++int_state; if(int_state>(32 * wait)) int_state = 0;
    return int_state;

}

int fadeOut_nd(uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait;

    for(uint_fast8_t j=0;j<num_leds;++j){
        setPixelColor(j, 32-i,32-i,32-i);
    }

    ++int_state; if(int_state>(32 * wait)) int_state = 0;
    return int_state;

}

//running dot
int eight_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait /2;

    if((int_state % 2) == 0){
        setPixelColor(i, r,g,b);
    }
    else{
        setPixelColor(i, 0,0,0);
    }
    ++int_state; if(int_state>(num_leds * wait * 2)) int_state = 0;
    return int_state;
}

//running dot filling
int eight2_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait;

    setPixelColor(i, r,g,b);

    ++int_state; if(int_state>(num_leds * wait)) int_state = 0;
    return int_state;
}

//running dot with tail
int eight3_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    const int tail_length = 3;
    static int int_state = 0;
    int i = int_state / wait;

    setPixelColor(i, r,g,b);
    uint_fast8_t rt=r, gt=g, bt=b;
    for (int j = 0; j < tail_length; ++j) {
        rt /= 2; gt /=2; bt /=2; //fade out each consecutive tail pixel
        if(i>j) setPixelColor(i-1-j, rt,gt,bt);
    }
    if(i>tail_length) setPixelColor(i-1-tail_length, 0,0,0);//last pixel just after tail turned off

    ++int_state; if(int_state>((num_leds + tail_length) * wait)) int_state = 0;
    return int_state;
}

//running dot with tail - backwards
int eight3b_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    const int tail_length = 3;
    static int int_state = 0;
    int i = int_state / wait;

    setPixelColor(num_leds-1-i, r,g,b);
    uint_fast8_t rt=r, gt=g, bt=b;
    for (int j = 0; j < tail_length; ++j) {
        rt /= 2; gt /=2; bt /=2; //fade out each consecutive tail pixel
        if(i>j) setPixelColor(num_leds-1-(i-1-j), rt,gt,bt);
    }
    if(i>tail_length) setPixelColor(num_leds-1-(i-1-tail_length), 0,0,0);//last pixel just after tail turned off

    ++int_state; if(int_state>((num_leds + tail_length) * wait)) int_state = 0;
    return int_state;
}

//fill symmetrically both "eyes"
int fill1_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait;

    setPixelColor(i, r,g,b);
    setPixelColor(i+(num_leds/2), r,g,b);

    ++int_state; if(int_state>((num_leds/2) * wait)) int_state = 0;
    return int_state;

}

//fill symmetrically both "eyes" -backward
int fill2_nd( uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait)
{
    static int int_state = 0;
    int i = int_state / wait;

    setPixelColor(num_leds-1-i, r,g,b);
    setPixelColor(num_leds-1-(i+(num_leds/2)), r,g,b);

    ++int_state; if(int_state>((num_leds/2) * wait)) int_state = 0;
    return int_state;

}

//Theatre-style crawling lights.
int theaterChase2_nd(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait, uint8_t every, uint8_t cycles) {
    static int int_state = 0;
    int i = int_state / wait;

    // turn every [gap] led on, the rest off
    clear();

    for (int j=(i % every); j < num_leds; j=j+every) {
        setPixelColor(j, r,g,b);    //turn every third pixel on
    }

    ++int_state; if(int_state>(3*cycles * wait)) int_state = 0;
    return int_state;

}


//horizontal curtain
int hCurtain1_nd(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait){
    const int boost = 4;
    static int int_state = 0;

    int j = int_state / wait;

        for (int i = 0; i < num_leds; ++i) {
            if(led_x_pos[i] <= j) setPixelColor(i, r,g,b);
        }

    int_state = int_state + boost; if(int_state>(256 * wait)) int_state = 0;
    return int_state;
}

//horizontal curtain - backwards
int hCurtain2_nd(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait){
    const int boost = 4;
    static int int_state = 0;

    int j = int_state / wait;

        for (int i = 0; i < num_leds; ++i) {
            if(led_x_pos[i] <= j) setPixelColor(num_leds-1-i, r,g,b);
        }

    int_state = int_state + boost; if(int_state>(256 * wait)) int_state = 0;
    return int_state;
}

//vertical curtain
int vCurtain1_nd(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait){
    const int boost = 4;
    static int int_state = 0;

    int j = int_state / wait;

        for (int i = 0; i < num_leds; ++i) {
            if(led_y_pos[i] <= j) setPixelColor(i, r,g,b);
        }

    int_state = int_state + boost; if(int_state>(256 * wait)) int_state = 0;
    return int_state;
}

//vertical curtain
int vCurtain2_nd(uint_fast8_t r, uint_fast8_t g, uint_fast8_t b, uint8_t wait){
    const int boost = 4;
    static int int_state = 0;

    int j = int_state / wait;

        for (int i = 0; i < num_leds; ++i) {
            if(led_y_pos[i] <= j) setPixelColor(num_leds-1-i, r,g,b);
        }

    int_state = int_state + boost; if(int_state>(256 * wait)) int_state = 0;
    return int_state;
}

int rainbow_nd(uint8_t wait){
    static int int_state = 0;
    int j = int_state / wait;

    uint8_t *c;

    for(int i=0; i< num_leds; i++) {
        c=Wheel(((i * 256 / num_leds) + j) & 255);
        BUFFER_LEDS[i][0] = *c/10 ;
        BUFFER_LEDS[i][1] = *(c+1)/10 ;
        BUFFER_LEDS[i][2] = *(c+2)/10 ;
    }


    ++int_state; if(int_state>(256 * wait)) int_state = 0;
    return int_state;

}


#endif /* LED_FUNCTIONS_ND_H_ */
