#include <assert.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "i18n.h"

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

static void
configure_locale(void)
{
        const char *dir;

        setlocale(LC_ALL, "");
        dir = getenv("ADVENTURE_LOCALEDIR");
        if (!dir)
                dir = ".";
        bindtextdomain("advent", dir);
        bind_textdomain_codeset("advent", "UTF-8");
        textdomain("advent");
}

static void
check_single_word(void)
{
        char tokens[2][WORDSIZE];
        size_t count = 0;
        int status = adventure_parse_command("inventário", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 1);
        assert(strcmp(tokens[0], "INVENTORY") == 0);
        assert(tokens[1][0] == '\0');
}

static void
check_two_word_with_stop(void)
{
        char tokens[2][WORDSIZE];
        size_t count = 0;
        int status = adventure_parse_command("pegar a lanterna", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GET") == 0);
        assert(strcmp(tokens[1], "LAMP") == 0);
}

static void
check_direction_synonym(void)
{
        char tokens[2][WORDSIZE];
        size_t count = 0;
        int status = adventure_parse_command("ir para norte", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GO") == 0);
        assert(strcmp(tokens[1], "NORTH") == 0);
}

static void
check_too_many_words(void)
{
        char tokens[2][WORDSIZE];
        size_t count = 0;
        int status = adventure_parse_command("pegar a lanterna agora", tokens, &count);
        if (status != PARSE_TOO_MANY_WORDS)
                abort();
        assert(status == PARSE_TOO_MANY_WORDS);
}

static void
check_catalog_translation(void)
{
        const char *po_path = PROJECT_SOURCE_DIR "/po/pt_BR.po";
        FILE *fp = fopen(po_path, "r");
        assert(fp != NULL);

        const char *target = "Commands are limited to two significant words.";
        const char *expected = "Só entendo comandos com duas palavras.";
        char line[1024];
        int found = 0;
        int matched = 0;

        while (fgets(line, sizeof(line), fp)) {
                if (!found) {
                        if (strncmp(line, "msgid \"", 7) == 0 && strstr(line, target)) {
                                found = 1;
                        }
                        continue;
                }
                if (strncmp(line, "msgstr \"", 8) == 0) {
                        char *start = strchr(line, '"');
                        char *end = start ? strrchr(start + 1, '"') : NULL;
                        assert(start && end && end > start);
                        *end = '\0';
                        matched = strcmp(start + 1, expected) == 0;
                        break;
                }
                if (strncmp(line, "msgid ", 6) == 0)
                        break;
        }

        fclose(fp);
        if (!found || !matched)
                abort();
        assert(found);
        assert(matched);
}

int
main(void)
{
        configure_locale();
        check_single_word();
        check_two_word_with_stop();
        check_direction_synonym();
        check_too_many_words();
        check_catalog_translation();
        return 0;
}
