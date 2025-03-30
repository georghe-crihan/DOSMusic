#include <stdio.h>
#include <dos.h>
#include <conio.h> /* getch() */

typedef char int8_t;
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef short int16_t;

volatile int8_t flag1 = 0; 
volatile int8_t flag2 = -1;
volatile u_int8_t tick = 0;
volatile u_int8_t total_ticks = 0;
volatile u_int16_t idx = 0;
volatile u_int16_t idx_max = 0xDA;

static u_int16_t tbl[] = {
    0x2BCA, 5, 2, 0x2BC2, 0x3705, 5, 0x4185, 5, 0x5782, 0x5282,
    0x4902, 0x5282, 0x578A, 5, 0x5285, 5, 0x4902, 2, 0x4182, 0x3A42,
    0x3702, 0x3A42, 0x418A, 5, 0x3A45, 5, 0x3702, 2, 0x370A, 0x20C5,
    2, 0x20C2, 0x2BC2, 2, 0x20C2, 2, 0x2BC2, 2, 0x20C2, 2, 0x2BCA,
    5, 2, 0x2BC2, 0x3705, 5, 0x4185, 5, 0x6E02, 0x6202, 0x5782,
    0x6202, 0x6E0A, 5, 0x6205, 5, 0x5782, 2, 0x5282, 0x5782, 0x6202,
    0x5782, 0x5282, 2, 0x6202, 2, 0x5282, 2, 0x6202, 2, 0x4902,
    2, 0x5782, 2, 0x5282, 0x5782, 0x6202, 0x5782, 0x5282, 2, 0x6202,
    2, 0x5282, 2, 0x6202, 2, 0x4902, 2, 0x5782, 2, 0x5282, 2,
    0x5282, 0x4902, 0x4182, 2, 0x4182, 0x3A42, 0x3702, 2, 0x4182,
    2, 0x578A, 5, 0x6202, 2, 0x5287, 0x5781, 1, 0x5794, 0x1111
};

void (__interrupt __far *prev_int_1c)();

void __interrupt __far timer_rtn()
  {
  u_int8_t v0; /* al */
  u_int16_t v1; /* bx */
  u_int8_t v2; /* al */
  u_int8_t v3; /* al */
  int16_t v4; /* bx */
  u_int16_t vt;

  _disable();
  if ( flag1 != -1 )
  {
    if ( flag2 != -1 )
    {
      v0 = inp(0x61u);
      outp(0x61u, v0 & 0xFC);
    }
    flag1 = ~flag1;
    flag2 = ~flag2;
  }
  if ( flag2 != -1 )
  {
    if ( tick < total_ticks )
    {
      ++tick;
    }
    else
    {
      vt = tbl[idx];
      /* v1 = *MK_FP(2, (_WORD)tbl + idx) >> 5; */
      v1 = vt >> 5;
      /* total_ticks = (*MK_FP(2, (_WORD)tbl + idx) & 0x3F) - 1; */
      total_ticks = (vt & 0x3F) - 1;
      if ( v1 <= 0x12u )
      {
        v3 = inp(0x61u);
        outp(0x61u, v3 & 0xFC);
      }
      else
      {
        vt = 0x1234DDu / v1;
        outp(0x42u, vt);
        outp(0x42u, (u_int16_t)vt >> 8);
        v2 = inp(0x61u);
        outp(0x61u, v2 | 3);
      }
      tick = 0;
      v4 = idx + 2;
      if ( (int16_t)(idx + 2) > idx_max)
        v4 = 0;
      idx = v4;
    }
  }
    _chain_intr( prev_int_1c );
}


void main()
  {
    prev_int_1c = _dos_getvect( 0x1c );
    _dos_setvect( 0x1c, timer_rtn );

    /* Wait for key press */
    getch();
    _dos_setvect( 0x1c, prev_int_1c );
  }
