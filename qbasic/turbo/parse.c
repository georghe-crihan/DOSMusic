/* Turbo Basic Owner's Handbook documents passing String Descriptors to INLINE
 * procedures (aka Assembly code), as well as RUNTIME's and user's memory map
 * (in particular the Scalar Segment and String Segment) from where the data is
 * read.
 * BUG: Does not output the last string.
 */
#include <stdio.h>
#include <stdlib.h>

/* String segment */
u_int16_t seg = 0xC020;
/* Scalar segment: String Descriptor array */
u_int16_t descr = 0xB420;
u_int16_t descr_end = 0xBF82;

typedef struct __attribute__((packed)) {
  u_int16_t len;
  u_int16_t off;
} strdescr;

int main(int argc, char **argv)
{
FILE *infile = NULL;
strdescr *dp = (strdescr *)malloc(descr_end-descr);
char *s = (char *)malloc(65536);

infile = fopen(argv[1], "rb");
fseek(infile, (long)descr, SEEK_SET);
fread(dp, 1, descr_end-descr, infile);
printf("Descriptor array length: %d\n", descr_end-descr);
for (strdescr *p = dp; p < dp + descr_end-descr; ++p) {
    fseek(infile, (long) seg + p->off, SEEK_SET);
    /* The high bit might or might not be set, just mask it out */
    u_int16_t l = (p->len) & ~0x8000;
    fread(s, 1, l, infile);
    s[l] = '\0';
    printf("Length: %d, offset: %X (%X), string: %s\n", l, p->off, (unsigned int)((p-dp) * sizeof(strdescr)), s); 
}
free(dp);
free(s);
fclose(infile);

return 0;
}
