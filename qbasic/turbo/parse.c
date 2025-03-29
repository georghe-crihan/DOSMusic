#include <stdio.h>
#include <stdlib.h>

unsigned short seg = 0xC020;
unsigned short descr = 0xB420;
unsigned short descr_end = 0xBF82;

typedef struct __attribute__((packed)) {
  unsigned short len;
  unsigned short off;
} strdescr;

int main(int argc, char **argv)
{
FILE *infile = NULL;
strdescr *dp = (strdescr *)malloc(descr_end-descr);
char *s = (char *)malloc(64000);

infile = fopen(argv[1], "rb");
fseek(infile, (long)descr, SEEK_SET);
fread(dp, 1, descr_end-descr, infile);
printf("Descriptor length: %d\n", descr_end-descr);
for (strdescr *p = dp; p < dp + descr_end-descr; ++p) {
    fseek(infile, (long) seg + p->off, SEEK_SET);
    unsigned short l = (p->len) & ~0x8000;
    printf("Length: %d\n", l); 
    fread(s, 1, l, infile);
    s[l] = '\0';
    printf("Length: %d, string: %s\n", l, s); 
}
free(dp);
free(s);
fclose(infile);

return 0;
}
