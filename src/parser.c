#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "i18n.h"

#ifndef WORDSIZE
#define WORDSIZE 20
#endif

#ifndef ADVENTURE_STRICT_2WORD_INPUT
#define ADVENTURE_STRICT_2WORD_INPUT 1
#endif

struct synonym {
        const char *term;
        const char *token;
};

static const char *const stop_words[] = {
        "A", "AS", "O", "OS", "UM", "UMA", "UNS", "UMAS",
        "DE", "DO", "DA", "DOS", "DAS",
        "NO", "NOS", "NA", "NAS",
        "AO", "AOS", "E",
        "PRA", "PRO", "PARA",
        "THE", "AN", "AT", "IN", "ON", "TO",
};

static const struct synonym synonyms[] = {
        { "ABRIR", "OPEN" },
        { "ANDAR", "GO" },
        { "APAGAR", "OFF" },
        { "DESLIGAR", "OFF" },
        { "ENTRAR", "IN" },
        { "EXAMINAR", "LOOK" },
        { "GUARDAR", "DROP" },
        { "INVENTARIO", "INVENTORY" },
        { "IR", "GO" },
        { "LARGAR", "DROP" },
        { "LIGAR", "ON" },
        { "LIGUE", "ON" },
        { "NORTE", "NORTH" },
        { "LESTE", "EAST" },
        { "OESTE", "WEST" },
        { "SUL", "SOUTH" },
        { "OLHAR", "LOOK" },
        { "PEGAR", "GET" },
        { "PEGUE", "GET" },
        { "PEGUEI", "GET" },
        { "PEGANDO", "GET" },
        { "LANTERNA", "LAMP" },
        { "LANTERNAS", "LAMP" },
        { "EXAMINA", "LOOK" },
        { "VER", "LOOK" },
        { "USAR", "USE" },
        { "USE", "USE" },
        { "VAI", "GO" },
        { "VAMOS", "GO" },
        { "VOLTA", "BACK" },
        { "VOLTE", "BACK" },
};

static const char too_many_words_msg[] = N_("Commands are limited to two significant words.");

static unsigned int decode_utf8(const char *input, size_t *consumed)
{
        unsigned char c = (unsigned char)input[0];

        if (!c) {
                *consumed = 0;
                return 0;
        }
        if (c < 0x80) {
                *consumed = 1;
                return c;
        }
        if ((c & 0xE0) == 0xC0) {
                unsigned char c1 = (unsigned char)input[1];
                if ((c1 & 0xC0) != 0x80) {
                        *consumed = 1;
                        return 0xFFFD;
                }
                *consumed = 2;
                return ((c & 0x1F) << 6) | (c1 & 0x3F);
        }
        if ((c & 0xF0) == 0xE0) {
                unsigned char c1 = (unsigned char)input[1];
                unsigned char c2 = (unsigned char)input[2];
                if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80)) {
                        *consumed = 1;
                        return 0xFFFD;
                }
                *consumed = 3;
                return ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
        }
        if ((c & 0xF8) == 0xF0) {
                unsigned char c1 = (unsigned char)input[1];
                unsigned char c2 = (unsigned char)input[2];
                unsigned char c3 = (unsigned char)input[3];
                if (((c1 & 0xC0) != 0x80) || ((c2 & 0xC0) != 0x80) ||
                    ((c3 & 0xC0) != 0x80)) {
                        *consumed = 1;
                        return 0xFFFD;
                }
                *consumed = 4;
                return ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) |
                       ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        }

        *consumed = 1;
        return 0xFFFD;
}

static int is_letter(unsigned int cp)
{
        if (cp < 0x80)
                return isalpha((unsigned char)cp);

        switch (cp) {
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x00C4:
        case 0x00C7:
        case 0x00C9: case 0x00CA: case 0x00CB:
        case 0x00CD: case 0x00CE: case 0x00CF:
        case 0x00D3: case 0x00D4: case 0x00D5: case 0x00D6:
        case 0x00DA: case 0x00DB: case 0x00DC:
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x00E4:
        case 0x00E7:
        case 0x00E9: case 0x00EA: case 0x00EB:
        case 0x00ED: case 0x00EE: case 0x00EF:
        case 0x00F3: case 0x00F4: case 0x00F5: case 0x00F6:
        case 0x00FA: case 0x00FB: case 0x00FC:
                return 1;
        default:
                return 0;
        }
}

static char normalize_letter(unsigned int cp)
{
        if (cp < 0x80) {
                if (isalpha((unsigned char)cp))
                        return (char)tolower((unsigned char)cp);
                if (isdigit((unsigned char)cp))
                        return (char)cp;
                return 0;
        }

        switch (cp) {
        case 0x00C7: case 0x00E7:
                return 'c';
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x00C4:
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x00E4:
                return 'a';
        case 0x00C9: case 0x00CA: case 0x00CB:
        case 0x00E9: case 0x00EA: case 0x00EB:
                return 'e';
        case 0x00CD: case 0x00CE: case 0x00CF:
        case 0x00ED: case 0x00EE: case 0x00EF:
                return 'i';
        case 0x00D3: case 0x00D4: case 0x00D5: case 0x00D6:
        case 0x00F3: case 0x00F4: case 0x00F5: case 0x00F6:
                return 'o';
        case 0x00DA: case 0x00DB: case 0x00DC:
        case 0x00FA: case 0x00FB: case 0x00FC:
                return 'u';
        default:
                return 0;
        }
}

static int is_stop_word(const char *word)
{
        size_t i;

        for (i = 0; i < sizeof(stop_words) / sizeof(stop_words[0]); ++i) {
                if (strcmp(stop_words[i], word) == 0)
                        return 1;
        }
        return 0;
}

static const char *lookup_synonym(const char *normalized)
{
        size_t i;

        for (i = 0; i < sizeof(synonyms) / sizeof(synonyms[0]); ++i) {
                if (strcmp(synonyms[i].term, normalized) == 0)
                        return synonyms[i].token;
        }
        return NULL;
}

static int add_token(char tokens[2][WORDSIZE], size_t *count, const char *token)
{
        if (*count >= 2) {
#if ADVENTURE_STRICT_2WORD_INPUT
                return 0;
#else
                return 1;
#endif
        }

        snprintf(tokens[*count], WORDSIZE, "%s", token);
        (*count)++;
        return 1;
}

int adventure_canonicalize_token(const char *input, char *output, size_t size)
{
        char normalized[WORDSIZE];
        size_t len = strlen(input);
        size_t i;

        if (len >= WORDSIZE)
                len = WORDSIZE - 1;

        for (i = 0; i < len; ++i)
                normalized[i] = toupper((unsigned char)input[i]);
        normalized[len] = '\0';

        if (!normalized[0])
                return 0;

        const char *mapped = lookup_synonym(normalized);
        if (!mapped)
                mapped = normalized;

        if (strlen(mapped) >= size)
                return 0;

        strcpy(output, mapped);
        return 1;
}

static int next_word(const char **cursor, char *output)
{
        const char *ptr = *cursor;
        size_t consumed;
        unsigned int cp;
        size_t len = 0;

        while (*ptr) {
                cp = decode_utf8(ptr, &consumed);
                if (consumed == 0)
                        break;
                if (isalpha((unsigned char)cp) || is_letter(cp) || isdigit((unsigned char)cp))
                        break;
                ptr += consumed;
        }

        if (!*ptr) {
                *cursor = ptr;
                return 0;
        }

        while (*ptr) {
                cp = decode_utf8(ptr, &consumed);
                if (consumed == 0)
                        break;
                if (!(isalpha((unsigned char)cp) || is_letter(cp) || isdigit((unsigned char)cp)))
                        break;
                if (len < WORDSIZE - 1) {
                        char ch = normalize_letter(cp);
                        if (!ch)
                                ch = (char)tolower((unsigned char)cp);
                        output[len++] = ch;
                }
                ptr += consumed;
        }

        output[len] = '\0';
        *cursor = ptr;
        if (!len)
                return next_word(cursor, output);
        return 1;
}

int adventure_parse_command(const char *line, char tokens[2][WORDSIZE], size_t *count)
{
        const char *cursor = line;
        char buffer[WORDSIZE];
        char canonical[WORDSIZE];
        size_t found = 0;

        tokens[0][0] = tokens[1][0] = '\0';

        while (next_word(&cursor, buffer)) {
                size_t i;

                for (i = 0; buffer[i]; ++i)
                        buffer[i] = (char)toupper((unsigned char)buffer[i]);

                if (is_stop_word(buffer))
                        continue;

                if (!adventure_canonicalize_token(buffer, canonical, sizeof(canonical)))
                        snprintf(canonical, sizeof(canonical), "%s", buffer);

                if (!add_token(tokens, &found, canonical))
                        return PARSE_TOO_MANY_WORDS;
        }

        if (count)
                *count = found;
        if (found == 0)
                tokens[0][0] = tokens[1][0] = '\0';
        else if (found == 1)
                tokens[1][0] = '\0';
        return PARSE_OK;
}

const char *adventure_two_word_error(void)
{
        return _(too_many_words_msg);
}
