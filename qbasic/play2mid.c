#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* floor(); */
#include <ctype.h>

/* https://www.freebasic.net/forum/viewtopic.php?p=248014#p248014 */

/* FIXME: Binary Midi & Track are not C strings and should have a length! */

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


static unsigned char *WriteFourBytes(unsigned char *Result, unsigned long Value)
{
  int i = 0;

  Result[i++] = (unsigned char)(Value & 0xff);
  Value >>= 8;
  Result[i++] = (unsigned char)(Value & 0xff);
  Value >>= 8;
  Result[i++] = (unsigned char)(Value & 0xff);
  Value >>= 8;
  Result[i++] = (unsigned char)(Value & 0xff);
  return Result;
}


static unsigned char _fbplay_internal_translateNote(char *toTranslate)
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

static int _fbplay_internal(char *Result, unsigned char channel, char *playstr)
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
  int l, i = 0, p = 0;
  char STR1[256];

  Result[0] = '\0';
  memset(note_stack, 0, sizeof(note_stack));
  while (p < strlen(playstr)) {
    switch (tolower(playstr[p++])) {
    case 'n':
      *number = '\0'; i = 0;
      tch = playstr[p];
      while (tch >= 48 && tch <= 57) {
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
	sprintf(Result + strlen(Result), "%s%c%c%c",
		WriteVarLen(STR1, &l, (long)floor(240 * next_event + 0.5)),
		channel + 0x90, idx, volume);
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
	sprintf(toTranslate, "%c", ch);
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
	while (ch >= 48 && ch <= 57) {
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
	idx = octave * 12 + _fbplay_internal_translateNote(toTranslate);
	sprintf(Result + strlen(Result), "%s%c%c%c",
		WriteVarLen(STR1, &l, (unsigned long)floor(240 * next_event + 0.5)),
		channel + 0x90, idx, volume);
	next_event = duration * (1 - note_len_mod);
	note_stack[0]++;
	note_stack[note_stack[0]] = idx;
      break;
      case 'p':
	  *number = '\0'; i = 0;
	  ch = playstr[p];
	  while (ch >= 48 && ch <= 57) {
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
		while (ch >= 48 && ch <= 57) {
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
		  while (ch >= 48 && ch <= 57) {
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
		    while (ch >= 48 && ch <= 57) {
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
			while (ch >= 48 && ch <= 57) {
			  number[i++] = ch;
			  p++;
			  if (p < strlen(playstr))
			    ch = playstr[p];
			  else
			    break;
			}
                        number[i] = '\0';
			sprintf(Result + strlen(Result), "%s%c%c",
				WriteVarLen(STR1, &l, 0L), channel + 0xc0,
				(char)atoi(number));
        break;
        case 'v':
			  *number = '\0'; i = 0;
			  ch = playstr[p];
			  while (ch >= 48 && ch <= 57) {
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
	sprintf(Result + strlen(Result), "%s%c%c",
	  WriteVarLen(STR1, &l, (unsigned long)floor(240 * duration * note_len_mod + 0.5)),
	  channel + 0x80, note_stack[i]);
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
  unsigned char Midi[256], Track[256];
  FILE *F = NULL;
  unsigned char STR1[256], STR2[256];

  memset(Midi, 0, sizeof(Midi));
  tracklen = _fbplay_internal(Track, 0, playstr);
  if (tracklen != 0) {
    sprintf(Midi + midilen, "MTrk%s%s\377/",
	    WriteFourBytes(STR2, tracklen + 4L), Track);
    Tracks++;
  }
  tracklen = _fbplay_internal(Track, 1, playstr1);
  if (tracklen != 0) {
    sprintf(Midi + midilen, "MTrk%s%s\377/",
	    WriteFourBytes(STR1, tracklen + 4L), Track);
    Tracks++;
  }
  sprintf(Midi, "MThd\006%c%cx%s",
	  (Tracks > 1) ? 1 : 0, (unsigned char)Tracks, memcpy(STR1, Midi, midilen));
  F = fopen(midiFileName, "wb");
  Tracks = fwrite(Midi, 1, midilen, F);
  fclose(F);
}
