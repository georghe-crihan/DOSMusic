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

  Result[*len++] = (unsigned char)(Value & 0x7f);
  while (Value > 0x7f) {
    Value >>= 7;
    Result[*len++] = (unsigned char)((Value & 0x7f) | 0x80);
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

  if (!strcmp(toTranslate, "e"))
    return 4;

  if (!strcmp(toTranslate, "fb"))
    return 4;

  if (!strcmp(toTranslate, "f"))
    return 5;

  if (!strcmp(toTranslate, "es"))
    return 5;

  if (!strcmp(toTranslate, "fs"))
    return 6;

  if (!strcmp(toTranslate, "gb"))
    return 6;

  if (!strcmp(toTranslate, "g"))
    return 7;

  if (!strcmp(toTranslate, "gs"))
    return 8;

  if (!strcmp(toTranslate, "ab"))
    return 8;

  if (!strcmp(toTranslate, "a"))
    return 9;

  if (!strcmp(toTranslate, "as"))
    return 10;

  if (!strcmp(toTranslate, "bb"))
    return 10;

  if (!strcmp(toTranslate, "b"))
    return 11;

  if (!strcmp(toTranslate, "cb"))
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

static char *_fbplay_internal(char *Result, unsigned char channel, char *playstr)
{
  char result[256];
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
  int l, i, p = 0;
  char STR1[256];

  *result = '\0';
  for (i = 0; i <= 128; i++)
    note_stack[i] = 0;
  while (p < strlen(playstr)) {
    ch = tolower(playstr[p]);
    p++;
    switch (ch) {
    case 'n':
      *number = '\0';
      tch = playstr[p];
      while (tch >= 48 && tch <= 57) {
	sprintf(number + strlen(number), "%c", tch);
	p++;
	if (p < strlen(playstr))
	  tch = playstr[p];
	else
	  tch = '\0';
      }
      idx = atoi(number);
      if (idx == 0)
	next_event += 60.0 / tempo * (4.0 / note_len) / 60;
      else {
	duration = 60.0 / tempo * (4.0 / note_len);
	sprintf(result + strlen(result), "%s%c%c%c",
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
	ch = playstr[p];
	while (ch >= 48 && ch <= 57) {
	  sprintf(number + strlen(number), "%c", ch);
	  p++;
	  if (p < strlen(playstr))
	    ch = playstr[p];
	  else
	    ch = '\0';
	}
	if (StrToIntDef(number, 0L) != 0)
	  duration = duration * 4 / atoi(number);
	if (ch == '.')
	  duration *= 1.5;
	idx = octave * 12 + _fbplay_internal_translateNote(toTranslate);
	sprintf(result + strlen(result), "%s%c%c%c",
		WriteVarLen(STR1, &l, (unsigned long)floor(240 * next_event + 0.5)),
		channel + 0x90, idx, volume);
	next_event = duration * (1 - note_len_mod);
	note_stack[0]++;
	note_stack[note_stack[0]] = idx;
      break;
      case 'p':
	  *number = '\0';
	  ch = playstr[p];
	  while (ch >= 48 && ch <= 57) {
	    sprintf(number + strlen(number), "%c", ch);
	    p++;
	    if (p <= strlen(playstr))
	      ch = playstr[p];
	    else
	      ch = '\0';
	  }
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
		*number = '\0';
		ch = playstr[p];
		while (ch >= 48 && ch <= 57) {
		  sprintf(number + strlen(number), "%c", ch);
		  p++;
		  if (p < strlen(playstr))
		    ch = playstr[p];
		  else
		    ch = '\0';
		}
		octave = atoi(number);
        break;
        case 't':	
		if (ch == 't') {
		  *number = '\0';
		  ch = playstr[p];
		  while (ch >= 48 && ch <= 57) {
		    sprintf(number + strlen(number), "%c", ch);
		    p++;
		    if (p < strlen(playstr))
		      ch = playstr[p];
		    else
		      ch = '\0';
		  }
		  tempo = atoi(number);
        break;
        case 'l':
		    *number = '\0';
		    ch = playstr[p];
		    while (ch >= 48 && ch <= 57) {
		      sprintf(number + strlen(number), "%c", ch);
		      p++;
		      if (p < strlen(playstr))
			ch = playstr[p];
		      else
			ch = '\0';
		    }
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
			*number = '\0';
			ch = playstr[p];
			while (ch >= 48 && ch <= 57) {
			  sprintf(number + strlen(number), "%c", ch);
			  p++;
			  if (p < strlen(playstr))
			    ch = playstr[p];
			  else
			    ch = '\0';
			}
			sprintf(result + strlen(result), "%s%c%c",
				WriteVarLen(STR1, &l, 0L), channel + 0xc0,
				(char)atoi(number));
        break;
        case 'v':
			  *number = '\0';
			  ch = playstr[p];
			  while (ch >= 48 && ch <= 57) {
			    sprintf(number + strlen(number), "%c", ch);
			    p++;
			    if (p < strlen(playstr))
			      ch = playstr[p];
			    else
			      ch = '\0';
			  }
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
	sprintf(result + strlen(result), "%s%c%c",
	  WriteVarLen(STR1, &l, (unsigned long)floor(240 * duration * note_len_mod + 0.5)),
	  channel + 0x80, note_stack[i]);
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 314:
 * Note: Null character at end of sprintf control string [148] */
	duration = 0.0;
      }
      note_stack[0] = 0;
    }
  }
  return Result;
}


void Play(char *midiFileName, char *playstr, char *playstr1)
{
  long Tracks = 0;
  unsigned char Midi[256], Track[256];
  FILE *F = NULL;
  unsigned char STR1[256], STR2[256];

  *Midi = '\0';
  _fbplay_internal(Track, 0, playstr);
  if (*Track != '\0') {
    sprintf(Midi + strlen(Midi), "MTrk%s%s\377/",
	    WriteFourBytes(STR2, strlen(Track) + 4L), Track);
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 333:
 * Note: Null character at end of sprintf control string [148] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 333:
 * Note: Null character at end of sprintf control string [148] */
    Tracks++;
  }
  _fbplay_internal(Track, 1, playstr1);
  if (*Track != '\0') {
    sprintf(Midi + strlen(Midi), "MTrk%s%s\377/",
	    WriteFourBytes(STR1, strlen(Track) + 4L), Track);
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 339:
 * Note: Null character at end of sprintf control string [148] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 339:
 * Note: Null character at end of sprintf control string [148] */
    Tracks++;
  }
  sprintf(Midi, "MThd\006%c%cx%s",
	  (Tracks > 1) ? 1 : 0, (unsigned char)Tracks, strcpy(STR1, Midi));
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 342:
 * Note: Null character in sprintf control string [147] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 342:
 * Note: Null character in sprintf control string [147] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 342:
 * Note: Null character in sprintf control string [147] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 342:
 * Note: Null character at end of sprintf control string [148] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 342:
 * Note: Null character at end of sprintf control string [148] */
/* p2c: /Users/mac/Sandbox/DOSMusic/qbasic/play2mid.pas, line 343:
 * Note: Null character at end of sprintf control string [148] */
  F = fopen(midiFileName, "wb");
  Tracks = fwrite(Midi, 1, strlen(Midi), F);
  fclose(F);
}
