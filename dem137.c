void timer_rtn()
{
  unsigned __int8 v0; // al
  unsigned __int16 v1; // bx
  unsigned __int8 v2; // al
  unsigned __int8 v3; // al
  __int16 v4; // bx

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
        __outbyte(0x42u, (unsigned __int16)vt >> 8);
        v2 = __inbyte(0x61u);
        __outbyte(0x61u, v2 | 3);
      }
      cnt1 = 0;
      v4 = idx1 + 2;
      if ( (__int16)(idx1 + 2) > /* idx1_max */word_1018F )
        v4 = 0;
      idx1 = v4;
    }
  }
  __asm { iretw }
}

