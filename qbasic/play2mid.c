#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* floor(); */
#include <ctype.h>

/* https://www.freebasic.net/forum/viewtopic.php?p=248014#p248014
 * https://www.developpez.net/forums/d2109212/autres-langages/pascal/free-pascal/production-d-fichier-midi-partir-d-chaine-caracteres-facon-qbasic/#post11719421
 */

static unsigned char *WriteVarLen(unsigned char *Result, int *len, unsigned long Value)
{
  *len = 0;

  Result[(*len)++] = (unsigned char)(Value & 0x7f);
  while (Value > 0x7f) {
    Value >>= 7;
    Result[(*len)++] = (unsigned char)((Value & 0x7f) | 0x80);
  }
  return Result;
}

static unsigned char translateNote(char *toTranslate)
{
  if (!strcmp(toTranslate, "c"))
    return 0;

  if (!strcmp(toTranslate, "cs") ||
      !strcmp(toTranslate, "db"))
    return 1;

  if (!strcmp(toTranslate, "d"))
    return 2;

  if (!strcmp(toTranslate, "ds") ||
      !strcmp(toTranslate, "eb"))
    return 3;

  if (!strcmp(toTranslate, "e") ||
      !strcmp(toTranslate, "fb"))
    return 4;

  if (!strcmp(toTranslate, "f") ||
      !strcmp(toTranslate, "es"))
    return 5;

  if (!strcmp(toTranslate, "fs") ||
      !strcmp(toTranslate, "gb"))
    return 6;

  if (!strcmp(toTranslate, "g"))
    return 7;

  if (!strcmp(toTranslate, "gs") ||
      !strcmp(toTranslate, "ab"))
    return 8;

  if (!strcmp(toTranslate, "a"))
    return 9;

  if (!strcmp(toTranslate, "as") ||
      !strcmp(toTranslate, "bb"))
    return 10;

  if (!strcmp(toTranslate, "b") ||
      !strcmp(toTranslate, "cb"))
    return 11;

  return 0;
}

static long StrToIntDef(char *s, long def)
{
  char *p;
  long result;

  result = strtol(s, &p, 10);
  if (p == s)
    return def;
  return result;
}

static int _fbplay_internal(unsigned char *Result, unsigned char channel, char *playstr)
{
  long tempo = 120;
  unsigned char note_len = 4;
  double note_len_mod = 1.0;
  unsigned char octave = 4, volume = 127;
  unsigned char note_stack[129];
  unsigned char chord;
  double next_event = 0.0;
  double duration;
  unsigned char idx;
  char number[256];
  char ch, tch;
  char toTranslate[256];
  int len = 0;
  int l = 0, i = 0, p = 0;
  unsigned char buf[64];

  Result[0] = '\0';
  memset(note_stack, 0, sizeof(note_stack));
  while (p < strlen(playstr)) {
    ch = tolower(playstr[p++]);
    switch (ch) {
    case 'n':
      *number = '\0'; i = 0;
      tch = playstr[p];
      while (isdigit(tch)) {
	number[i++] = tch;
	p++;
	if (p < strlen(playstr))
	  tch = playstr[p];
	else
	  break;
      }
      number[i] = '\0';
      idx = atoi(number);
      if (idx == 0)
	next_event += 60.0 / tempo * (4.0 / note_len) / 60;
      else {
	duration = 60.0 / tempo * (4.0 / note_len);
        memcpy(Result + len, 
		WriteVarLen(buf, &l, (long)floor(240 * next_event + 0.5)), l);
        len += l;
        Result[len++] = channel + 0x90;
        Result[len++] = idx;
        Result[len++] = volume;
	next_event = duration * (1 - note_len_mod);
	note_stack[0]++;
	note_stack[note_stack[0]] = idx;
      }
    break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
	duration = 60.0 / tempo * (4.0 / note_len);
	toTranslate[0] = ch; toTranslate[1] = '\0';
	*number = '\0';
	ch = playstr[p];
	if (ch == '-') {
	  strcat(toTranslate, "b");
	  p++;
	} else {
	  if (ch == '+' || ch == '#') {
	    strcat(toTranslate, "s");
	    p++;
	  }
	}
	ch = playstr[p]; i = 0;
	while (isdigit(ch)) {
	  number[i++] = ch;
	  p++;
	  if (p < strlen(playstr))
	    ch = playstr[p];
	  else
	    break;
	}
        number[i] = '\0';
	if (StrToIntDef(number, 0L) != 0)
	  duration = duration * 4 / atoi(number);
	if (ch == '.')
	  duration *= 1.5;
	idx = octave * 12 + translateNote(toTranslate);
        memcpy(Result + len, 
		WriteVarLen(buf, &l, (unsigned long)floor(240 * next_event + 0.5)), l);
        len += l;
	Result[len++] = channel + 0x90;
        Result[len++] = idx;
        Result[len++] = volume;
	next_event = duration * (1 - note_len_mod);
	note_stack[0]++;
	note_stack[note_stack[0]] = idx;
      break;
      case 'p':
	  *number = '\0'; i = 0;
	  ch = playstr[p];
	  while (isdigit(ch)) {
	    number[i++] = ch;
	    p++;
	    if (p <= strlen(playstr))
	      ch = playstr[p];
	    else
	      break;
	  }
          number[i] = '\0';
	  next_event += 60.0 / tempo * 4 / atoi(number);
        break;
        case '>':
	    if (octave < 7)
	      octave++;
        break;
	case '<':
	    if (octave > 1)
	      octave--;
        break;
	case 'o':
             *number = '\0'; i = 0;
	     ch = playstr[p];
		while (isdigit(ch)) {
		  number[i++] = ch;
		  p++;
		  if (p < strlen(playstr))
		    ch = playstr[p];
		  else
		    break;
		}
             number[i] = '\0';
	     octave = atoi(number);
        break;
        case 't':	
		if (ch == 't') {
		  *number = '\0'; i = 0;
		  ch = playstr[p];
		  while (isdigit(ch)) {
		    number[i++] = ch;
		    p++;
		    if (p < strlen(playstr))
		      ch = playstr[p];
		    else
		      break;
		  }
                  number[i] = '\0';
		  tempo = atoi(number);
        break;
        case 'l':
		    *number = '\0'; i = 0;
		    ch = playstr[p];
		    while (isdigit(ch)) {
		      number[i++] = ch;
		      p++;
		      if (p < strlen(playstr))
			ch = playstr[p];
		      else
			break;
		    }
                    number[i] = '\0';
		    note_len = StrToIntDef(number, 1L);
        break;
        case 'm':
		      ch = tolower(playstr[p]);
		      p++;
		      if (ch == 's')
			note_len_mod = 3.0 / 4;
		      if (ch == 'n')
			note_len_mod = 7.0 / 8;
		      if (ch == 'l')
			note_len_mod = 1.0;
        break;
        case 'i':
			*number = '\0'; i = 0;
			ch = playstr[p];
			while (isdigit(ch)) {
			  number[i++] = ch;
			  p++;
			  if (p < strlen(playstr))
			    ch = playstr[p];
			  else
			    break;
			}
                        number[i] = '\0';
			memcpy(Result + len, 
				WriteVarLen(buf, &l, 0L), l);
                        len += l;
                        Result[len++] = channel + 0xc0;
			Result[len++] = (char)atoi(number);
        break;
        case 'v':
			  *number = '\0'; i = 0;
			  ch = playstr[p];
			  while (isdigit(ch)) {
			    number[i++] = ch;
			    p++;
			    if (p < strlen(playstr))
			      ch = playstr[p];
			    else
			      break;
			  }
                          number[i] = '\0';
			  volume = atoi(number);
	break;
	case '{':
			  chord = 1;
        break;
        case '}':
                          chord = 0;
        break;
        default:
                     ;
        }
    }
    if (chord > 0) {
      if (chord == 2)
	next_event = 0.0;
      else
	chord = 2;
    } else {
      for (i = 1; i <= note_stack[0]; i++) {
        memcpy(Result + len, 
	  WriteVarLen(buf, &l, (unsigned long)floor(240 * duration * note_len_mod + 0.5)), l);
        len += l;
	Result[len++] = channel + 0x80;
        Result[len++] = note_stack[i];
	duration = 0.0;
      }
      note_stack[0] = 0;
    }
  }
  return len;
}


void Play(char *midiFileName, char *playstr, char *playstr1)
{
  long Tracks = 0;
  int tracklen = 0, midilen = 0;
  int l = 0;
  unsigned char Midi[4096], Track[4096];
  FILE *F = NULL;
  char Header[32];
  struct __attribute__((packed)) {
    unsigned long sig;
    unsigned long len;
  } miditrack;

  miditrack.sig = 'krTM'; /* MTrk */
  memset(Midi, 0, sizeof(Midi));
  tracklen = _fbplay_internal(Track, 0, playstr);
  if (tracklen != 0) {
    miditrack.len = tracklen+4;
    memcpy(Midi + midilen, &miditrack, sizeof(miditrack));
    midilen += sizeof(miditrack);
    memcpy(Midi + midilen, Track, tracklen);
    midilen += tracklen;
    memcpy(Midi + midilen, "\0\xff\x2f\0", 4);
    midilen += 4;
    Tracks++;
  }
  tracklen = _fbplay_internal(Track, 1, playstr1);
  if (tracklen != 0) {
    miditrack.len = tracklen+4;
    memcpy(Midi + midilen, &miditrack, sizeof(miditrack));
    midilen += sizeof(miditrack);
    memcpy(Midi + midilen, Track, tracklen);
    midilen += tracklen;
    memcpy(Midi + midilen, "\0\xff\x2f\0", 4);
    midilen += 4;
    Tracks++;
  }
  memcpy(Header, "MThd\0\0\0\6\0\0\0\0\0\x78", 14);
  Header[9]= (Tracks > 1) ? 1 : 0;
  Header[11] = (unsigned char)(Tracks);
  F = fopen(midiFileName, "wb");
  fwrite(Header, 1, 14, F);
  fwrite(Midi, 1, midilen, F);
  fclose(F);
}
