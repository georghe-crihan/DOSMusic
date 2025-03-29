
/*

                           Virus Effects in Win32:
                        generating & playing wave file

        To play wave in windows you may use PlaySound() routine,
        described in mmsystem.hlp/.h

        BOOL PlaySound(
          LPCSTR pszSound,      // pointer to file name/file location/...
          HMODULE hmod,         // sucks, i.e. NULL
          DWORD fdwSound        // flags
        );

        PlaySound() has nice flag, SND_MEMORY. It means that pointer to
        .wav file name (pszSound) is pointer to file location in memory.
        So we may generate .wav file directly in memory.

                              about this program

        1. allocate some memory for .wav file, fill .wav header
        2. generate .wav data (convert old good pc-speaker melody (holms.c)
           into wave (pcm) format)
        3. play generated wave using PlaySound()
        4. play melody using pc-speaker (direct io, so Win9X only)
        5. play melody using soundblaster & pc-speaker simultaneously
        6. save generated file into TEST.WAV

                          about generating wave data
                       (pc-speaker data -> wave conversion)

        Однако, блин, почитали буржуи - и хватит. ;-)

        Наиболее интересной вещью для нас является генерация данных
        для wave-файла, ибо с хеадером все понятно. (из программы)

        Итак, у нас есть данные для pc-спикера.
        Представляют они собой число нот (здесь-звуков одной частоты),
        и два массива, частот (в герцах) и их длительностей (в миллисекундах).

        И надо нам сделать из этого всего данные, в самом простом .wav
        PCM-формате. Что же такое ентон формат?

        Ну например если параметры звука - 2 канала, 16-битовые сэмплы и
        44100 сэмплов в секунду, то .wav PCM файл выглядит так:

        58 bytes        заголовок
        word/word       данные для левого/правого канала
        ...
        каждые 44100 дворда - одна секунда

        Вот теперь можно спросить, что такое данные для левого/правого канала.

        амплитуда звуковой волны (громкость)
                                             V
        |.... ooo....           ...oooo.....         ... F1
        |   oo   oo .           . oo   oo  .  |       .
        | oo       o.           oo       oo.  |       .oo F2
        |o     <----o----X-----o---->      o  |       o
       -0-----------o----------o-----------o---------o-------------->Время,
        |           .o        o .          .o        o.              [сэмплы]
        <----------X--oo----oo->.          . oo    oo .              1/44100с
        |           ....o.oo.....          ....oooo....
        |<----------------------Y----------------------->

        Каждое 16-ти или 8-битное или еще там какое значение PCM данных -
        - суть значение амплитуды звуковой волны в данный момент времени.
        Для соответствующего канала, понятно.

        Вот собственно и все с форматом файла.

        А чтобы произвести тон с частотой F [Гц] и длительностью D [мс],
        надо просто изменять знак апмлитуды каждые F раз в секунду,
        и делать так в течение времени D.

        Если поглядеть на рисунок, то можо увидеть,
        что X=44100/F, а Y=D/1000*44100.

        Функции F1 (".") и F2 ("o") хотя и отличаются, но период их
        одинаков, и значит производят они одну и ту же частоту.
        А поскольку у этих функций одинаковы еще и амплитуды, то
        отличаются они только тембром, но впрочем, это уже не в тему.

        Скажем только, что если, например, значение амплитуды меняется
        меньше где-то 20 раз в секунду (в частности не меняется вобще),
        или меняется чаще 20000 раз в секунду, то звука мы якобы не услышим.

        В качестве эффекта для размышления -
        можно увеличивать громкость звука в одном канале, постепенно
        уменьшая ее в другом, и тогда звук будет "переходить" из
        одной колонки в другую.
        Для проверки эффекта пользуем строчку с (*)

                                             (c) 1999 Z0MBiE, z0mbie.cjb.net
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma hdrstop

#include "holms.c"              // data

#define WAVE_FORMAT_PCM 0x0001

#define WAVE_MAXSEC     20      // max playback time in seconds

#define WAVE_CHANNELS   2       // our wave parameters.
#define WAVE_FREQ       44100   //  (if you will change it, some lines
#define WAVE_BITRATE    16      //   in wave_write() should be uncommented)

#define WAVE_BYTESPERSAMPLE     (WAVE_CHANNELS*WAVE_BITRATE/8)
#define WAVE_BYTESPERSEC        (WAVE_BYTESPERSAMPLE*WAVE_FREQ)
#define WAVE_MAXBYTES           (WAVE_BYTESPERSEC*WAVE_MAXSEC)
#define WAVE_MAXSAMPLES         (WAVE_FREQ*WAVE_MAXSEC)

#pragma pack(push)
#pragma pack(1)

struct wave_struct
{
  uint8_t  riff_sign[4];   // ='RIFF'
  uint32_t riff_size;   // filesize-8=datasize+sizeof(wavehdr)-8=datasize+50
  uint8_t  wavefmt_sign[8];// ='WAVEfmt '
  uint32_t wavefmt_size;   // sizeof(PCMWAVEFORMAT)=12h
//WAVEFORMAT wavefmt;   // (5 following entries)
  uint16_t  wFormatTag;     // type=WAVE_FORMAT_PCM=1
  uint16_t  nChannels;      // channels=1/2
  uint32_t nSamplesPerSec; // freq=11025/22050/44100
  uint32_t nBytesPerSec;   // bytespersecond=channels*freq*bitrate/8=176400
  uint16_t  nBlockAlign;    // =1/2/4
  uint32_t wBitsPerSample;// =8/16
  uint8_t  fact_sign[4];// ='fact'
  uint32_t fact_hdrsize;// =4
  uint32_t entry_count; // =datasize/4
  uint8_t  data_sign[4];// ='data'
  uint32_t data_size;   // =datasize
  uint8_t  data[WAVE_MAXBYTES];
  uint32_t align;       // 'coz writing to data by DWORDs
};

#pragma pack(pop)

wave_struct *wave = (wave_struct*) malloc(sizeof(wave_struct));

void wave_init()
{
  memcpy(wave->riff_sign,"RIFF",4);
  wave->riff_size=50;
  memcpy(wave->wavefmt_sign,"WAVEfmt ",8);
  wave->wavefmt_size=0x12;
  wave->wFormatTag=WAVE_FORMAT_PCM;
  wave->nChannels=WAVE_CHANNELS;
  wave->nSamplesPerSec=WAVE_FREQ;
  wave->nBytesPerSec=WAVE_BYTESPERSEC;
  wave->nBlockAlign=WAVE_BYTESPERSAMPLE;
  wave->wBitsPerSample=WAVE_BITRATE;
  memcpy(wave->fact_sign,"fact",4);
  wave->fact_hdrsize=4;
  wave->entry_count=0;
  memcpy(wave->data_sign,"data",4);
  wave->data_size=0;
  memset(&wave->data, 0, sizeof(wave->data));
}

void wave_write(int a, int b)
{
  if (wave->data_size>=WAVE_MAXBYTES)
  {
    printf("error: MAX_SIZE reached\n");
    exit(0);
  }
//if (WAVE_BITRATE==8) { a>>=8; b>>=8; };       // need if WAVE_BITRATE!=16
  uint32_t d;
//if (WAVE_CHANNELS==1) d=(a+b)>>1; else        // need if WAVE_CHANNELS!=2
  d=(a<<WAVE_BITRATE)+b;
  *(uint32_t*)&wave->data[wave->data_size]=d;
  wave->riff_size+=WAVE_BYTESPERSAMPLE;
  wave->data_size+=WAVE_BYTESPERSAMPLE;
  wave->entry_count++;
}

#define WAVE_QUALITY 16384              // how many sin(x)'s to precalculate
#define WAVE_AMP     20000              // max volume. -32768..32767

int sin_table[WAVE_QUALITY+1];

int main(int argc, char **argv)
{
  // pre-calculated sinus table needed to generate wave faster
  if (argc > 1) printf("initializing sintable...\n");
  for (int i=0; i<WAVE_QUALITY; i++)
    sin_table[i] = sin( (float)i*M_PI*2/WAVE_QUALITY ) * WAVE_AMP;

  // calculate total music size
  int totaltime=0; // [ms]
  for (int i=0; i<music_notes; i++)
    totaltime += music_delay[i];
  if (argc > 1) printf("total music time = %i ms\n", totaltime);

  if (totaltime>WAVE_MAXSEC*1000)
  {
    fprintf(stderr, "***ERROR***: music size too large (or WAVE_MAXSEC too small)\n");
    exit(0);
  }

  if (argc > 1) printf("generating waveform...\n");
  wave_init();
  for (int note=0; note<music_notes; note++)
  {
    int freq  = music_freq[note];                       // [Hz]
    int delay = music_delay[note] * WAVE_FREQ / 1000;   // [ms]
    for (int t=0; t<delay; t++)
    {
      int w=sin_table[((int)((float)t*freq*WAVE_QUALITY/WAVE_FREQ))&(WAVE_QUALITY-1)];

      if (note>music_notes/2)          // for second half of melody
      if (w>0) w=WAVE_AMP; else w=-WAVE_AMP; // make pc-speaker alike sound

      wave_write(w,w);
//    wave_write((float)w*t/delay, (float)w*(delay-t)/delay); // (*)

    }
  }

  FILE *f = stdout;

  if (argc > 1)
      f=fopen(argv[1],"wb");

  fwrite(wave,1,58+wave->data_size,f);

  if (argc > 1)
      fclose(f);

  return 0;
}

