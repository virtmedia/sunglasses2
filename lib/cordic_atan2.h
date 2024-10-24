/*
 * cordic_atan2.h
 *
 *  Created on: Sep 28, 2024
 *      Author: Aleksander
 */

#ifndef LIB_CORDIC_ATAN2_H_
#define LIB_CORDIC_ATAN2_H_

const unsigned int sin_lut[7] = {12540, 6393, 3212, 1608, 804, 402, 201};
const unsigned int cos_lut[7] = {30274, 32138, 32610, 32729, 32758, 32766, 32767};
const unsigned int tan_lut[6] = {32768, 13573, 6518, 3227, 1610, 804};

/*
 * 8-bit CORDIC atan2sqrt
 * Calculates the angle and radius of the vector pointing towards (x,y)
 * The angle is returned as an unsigned char. 256 means 2pi or 360 degrees.
 * radius is the length of the vector as an int.
 */
void atan2sqrt(int x0, int y0, unsigned char *angle, unsigned int * radius) {   //max input values for x and y must be confirmed.

    int tmp;
    long x1, y1;
    unsigned char phi, currentAngle, k;
    char sign;

    *angle=0; sign=0;
    if(y0<0) {x0=-x0; y0=-y0; *angle=128;} //rotate 128 cw
    if(x0<0) {tmp=x0; x0=y0; y0=-tmp; *angle+=64;} //rotate 64 cw
    if(y0>x0) {sign=1; *angle+=64; tmp=x0; x0=y0; y0=tmp;}
    currentAngle=16; phi=0; k=0;
    while (k<=4){
        x1=(long)cos_lut[k]*(long)x0;
        x1+=(long)sin_lut[k]*(long)y0;
        y1=-(long)sin_lut[k]*(long)x0;
        y1+=(long)cos_lut[k]*(long)y0;
        if (y1>=0) {x0=x1>>15; y0=y1>>15; phi+=currentAngle;}
        currentAngle=currentAngle>>1;  //means: currAngle=currAngle/2;
        k++;
        }
    *radius=x0;
    *angle+=((sign)?-phi:+phi); //faster than: if(sign) {angle-=phi;} else {angle+=phi;} ? To be confirmed...
    // angle 2pi = 256, size of char
}


#endif /* LIB_CORDIC_ATAN2_H_ */
