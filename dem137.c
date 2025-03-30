#include <stdio.h>
#include <dos.h>

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef short int16_t;

volatile u_int8_t flag1; 
volatile u_int8_t flag2;
volatile u_int16_t cnt1;
volatile u_int16_t cnt1_max;

void (__interrupt __far *prev_int_1c)();

void __interrupt __far timer_rtn()
  {
  u_int8_t v0; // al
  u_int16_t v1; // bx
  u_int8_t v2; // al
  u_int8_t v3; // al
  int16_t v4; // bx
  u_int16_t vt;

  _disable();
  if ( flag1 != -1 )
  {
    if ( flag2 != -1 )
    {
      v0 = __inbyte(0x61u);
      __outbyte(0x61u, v0 & 0xFC);
    }
    flag1 = ~flag1;
    flag2 = ~flag2;
  }
  if ( flag2 != -1 )
  {
    if ( cnt1 < cnt1_max )
    {
      ++cnt1;
    }
    else
    {
      vt = tbl1[idx1];
      //v1 = *MK_FP(2, (_WORD)tbl1 + idx1) >> 5;
      v1 = vt >> 5;
      //cnt1_max = (*MK_FP(2, (_WORD)tbl1 + idx1) & 0x3F) - 1;
      cnt1_max = (vt & 0x3F) - 1;
      if ( v1 <= 0x12u )
      {
        v3 = __inbyte(0x61u);
        __outbyte(0x61u, v3 & 0xFC);
      }
      else
      {
        vt = 0x1234DDu / v1;
        __outbyte(0x42u, vt);
        __outbyte(0x42u, (u_int16_t)vt >> 8);
        v2 = __inbyte(0x61u);
        __outbyte(0x61u, v2 | 3);
      }
      cnt1 = 0;
      v4 = idx1 + 2;
      if ( (int16_t)(idx1 + 2) > /* idx1_max */word_1018F )
        v4 = 0;
      idx1 = v4;
    }
  }
    _chain_intr( prev_int_1c );
}


void main()
  {
    prev_int_1c = _dos_getvect( 0x1c );
    _dos_setvect( 0x1c, timer_rtn );

    /* Wait for key press */
    _dos_setvect( 0x1c, prev_int_1c );
  }
