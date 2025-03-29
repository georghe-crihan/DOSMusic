#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <assert.h>

typedef struct lzssx
{
    const uint8_t *src_cur;
    const uint8_t *src_limit;
    const uint8_t *dest;
    uint8_t *dest_cur;
    const uint8_t *dest_limit;
    unsigned int bits;
    size_t num_bits;
    jmp_buf jmp;
} lzssx;

static inline void lzssx_error(lzssx *state)
{
    longjmp(state->jmp, 0);
}

static inline void lzssx_fetch_bits(lzssx *state)
{
    if (state->src_cur + 1 > state->src_limit)
    {
        lzssx_error(state);
    }

    state->bits = state->src_cur[1] << 8 | state->src_cur[0];
    state->src_cur += 2;
    state->num_bits = 16;
}

static unsigned lzssx_get_bit(lzssx *state)
{
    unsigned bit = state->bits & 1;
    state->bits >>= 1;
    state->num_bits -= 1;

    if (!state->num_bits)
    {
        lzssx_fetch_bits(state);
    }

    return bit;
}

static inline uint8_t lzssx_get_byte(lzssx *state)
{
    if (state->src_cur >= state->src_limit)
    {
        lzssx_error(state);
    }

    return *state->src_cur++;
}

static inline void lzssx_put_byte(lzssx *state, uint8_t value)
{
    if (state->dest_cur >= state->dest_limit)
    {
        lzssx_error(state);
    }

    *state->dest_cur++ = value;
}

void lzssx_decompress_loop(lzssx *state)
{
    // Prefetch block flags for the first time.
    lzssx_fetch_bits(state);

    while (state->src_cur < state->src_limit)
    {
        if (lzssx_get_bit(state))
        {
            // Literal byte.
            lzssx_put_byte(state, lzssx_get_byte(state));
        }
        else
        {
            // Back references.
            int distance;
            size_t counter;

            if (!lzssx_get_bit(state))
            {
                // This is a short reference (max. distance is -256). Here we
                // MUST calculate counter first because lzssx_get_bit may
                // read flags from input.
                counter = lzssx_get_bit(state);
                counter = (counter << 1 | lzssx_get_bit(state)) + 2;
                distance = (1 << 8) - lzssx_get_byte(state);
            }
            else
            {
                // Long(er) reference.
                int b1 = lzssx_get_byte(state);
                int b2 = lzssx_get_byte(state);

                counter = (b2 & 7) + 2;
                distance = (1 << 13) - ((b2 & 0xf8) << 5 | b1);

                // If counter bits (b2 & 7) were 0.
                if (counter == 2)
                {
                    counter = lzssx_get_byte(state);

                    if (counter == 0)
                    {
                        // We have found an end-of-stream indicator. Stop.
                        break;
                    }

                    if (counter == 1)
                    {
                        // Unknown, but it is used sometimes. A decompressor
                        // seems to just ignore this reference.
                        continue;
                    }

                    ++counter;
                }
            }

            assert(distance >= 1);

            // Ensure that the reference is not out of range.
            if (state->dest_cur - state->dest < distance)
            {
                lzssx_error(state);
            }

            while (counter--)
            {
                // Copy bytes from a sliding window.
                lzssx_put_byte(state, state->dest_cur[-distance]);
            }
        }
    }
}

size_t lzssx_decompress(uint8_t *restrict dest, size_t dest_size, uint8_t *restrict src, size_t src_size)
{
    lzssx state = { src, src + src_size, dest, dest, dest + dest_size };

    if (setjmp(state.jmp))
    {
        return SIZE_MAX;
    }

    lzssx_decompress_loop(&state);

    // There should be no trash left in the input buffer.
    assert(state.src_cur == state.src_limit);

    return (size_t) (state.dest_cur - state.dest);
}

static void unpack_demo(size_t index, uint8_t *data, size_t size)
{
    static uint8_t buffer[32768];
    char filename[128];

    sprintf(filename, "dem%03u.com", index);
    printf("Writing to %s...\n", filename);

    // Deobfuscate.
    for (size_t i = 0; i < size; ++i)
    {
        data[i] ^= 0xad;
    }

    size_t out_size = lzssx_decompress(buffer, sizeof(buffer), data, size);
    if (out_size == SIZE_MAX)
    {
        fprintf(stderr, "decompression error\n");
        return;
    }

    FILE * f = fopen(filename, "wb");
    if (!f)
    {
        perror("unable to open output file");
        return;
    }

    fwrite(buffer, 1, out_size, f);
    fclose(f);
}

static bool verify_magic(FILE *f)
{
    static const char banner[] = "\x02\x3f\x2d\x56" // Magic.
        "\r\n" // Banner...
        "Antiviral ToolKit Pro\r\n"
        " by Eugene Kaspersky \r\n"
        "(c)KAMI Corp., Russia 1992-1995.\r\n"
        "Programmers:\r\n"
        "Alexey N. de Mont de Rique,\r\n"
        "Eugene V. Kaspersky,\r\n"
        "Vadim V. Bogdanov.\r\n"
        "\r\n";
    uint8_t buffer[0x100];

    if (!fread(buffer, sizeof(banner) - 1, 1, f))
    {
        perror("cannot read buffer");
        return false;
    }

    if (memcmp(buffer, banner, sizeof(banner) - 1) != 0)
    {
        fprintf(stderr, "wrong magic/banner\n");
        return false;
    }

    assert(ftell(f) == 0xad);
    return true;
}

static void unpack_demos(FILE *f)
{
    static uint8_t buffer[32768];
    uint32_t size;

    for (size_t index = 1; ; index++)
    {

        if (!fread(&size, sizeof(size), 1, f))
        {
            if (!feof(f))
            {
                perror("cannot read size of demo");
            }

            break;
        }

        if (size > sizeof(buffer))
        {
            fprintf(stderr, "demo is too big (%u bytes)\n", size);
            break;
        }

        if (!fread(buffer, size, 1, f))
        {
            perror("cannot read demo");
            break;
        }

        unpack_demo(index, buffer, size);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s AVP.DEM\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        perror("unable to open file");
        return 1;
    }

    if (!verify_magic(f))
    {
        fprintf(stderr, "trying known offset...\n");
        fseek(f, 0xad, SEEK_SET);
    }

    unpack_demos(f);

    fclose(f);
    return 0;
}
