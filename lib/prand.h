/*
 * prand.h
 *
 *  Created on: Sep 30, 2024
 *      Author: Aleksander
 */

#ifndef LIB_PRAND_H_
#define LIB_PRAND_H_

#include <stdint.h>

uint32_t prand32(int *seed){
    const int a=22695477;
    const int c=1;
    *seed = a* *seed + c;
    return (uint32_t)*seed;
}

uint16_t prand16(int *seed){
    return (uint16_t)(prand32(seed)>>4);
}

uint8_t prand8(int *seed){
    return (uint8_t)(prand32(seed)>>13);
}

#endif /* LIB_PRAND_H_ */
