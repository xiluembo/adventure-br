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

static int
expect_portuguese_translation(void)
{
#ifdef HAVE_GETTEXT
        const char *language = getenv("LANGUAGE");
        const char *locale = setlocale(LC_ALL, NULL);

        if (!language || strncmp(language, "pt_BR", 5) != 0)
                return 0;
        if (!locale)
                return 0;
        if (locale[0] == '\0')
                return 0;
        if (strncmp(locale, "pt", 2) == 0 || strncmp(locale, "PT", 2) == 0)
                return 1;
        if (strstr(locale, "Portuguese") || strstr(locale, "PORTUGUESE"))
                return 1;
        return 0;
#else
        return 0;
#endif
}

static int
buffer_contains(const unsigned char *data, size_t size, const char *needle)
{
        size_t i;
        size_t needle_len = strlen(needle);

        if (needle_len == 0)
                return 1;
        if (size < needle_len)
                return 0;
        for (i = 0; i + needle_len <= size; ++i) {
                if (memcmp(data + i, needle, needle_len) == 0)
                        return 1;
        }
        return 0;
}

static void
check_compiled_translation(const char *msgid, const char *expected)
{
        const char *dir = getenv("ADVENTURE_LOCALEDIR");
        char path[1024];
        FILE *fp;
        long raw_size;
        size_t size;
        unsigned char *buffer;
        size_t read;

        assert(dir != NULL);
        if (snprintf(path, sizeof(path), "%s/pt_BR/LC_MESSAGES/advent.mo", dir) >= (int)sizeof(path))
                abort();

        fp = fopen(path, "rb");
        assert(fp != NULL);
        if (fseek(fp, 0, SEEK_END) != 0)
                abort();
        raw_size = ftell(fp);
        if (raw_size < 0)
                abort();
        if (fseek(fp, 0, SEEK_SET) != 0)
                abort();
        size = (size_t)raw_size;
        buffer = malloc(size ? size : 1);
        assert(buffer != NULL);
        read = fread(buffer, 1, size, fp);
        fclose(fp);
        if (read != size)
                abort();

        if (!buffer_contains(buffer, size, msgid) ||
            !buffer_contains(buffer, size, expected)) {
                free(buffer);
                abort();
        }
        assert(buffer_contains(buffer, size, msgid));
        assert(buffer_contains(buffer, size, expected));
        free(buffer);
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

        const char *message = adventure_two_word_error();
#ifdef HAVE_GETTEXT
        if (expect_portuguese_translation()) {
                if (strcmp(message, "Só entendo comandos com duas palavras.") != 0)
                        abort();
                assert(strcmp(message, "Só entendo comandos com duas palavras.") == 0);
        } else if (strcmp(message, "Só entendo comandos com duas palavras.") == 0) {
                assert(strcmp(message, "Só entendo comandos com duas palavras.") == 0);
        } else {
                assert(strcmp(message, "Commands are limited to two significant words.") == 0);
        }
#else
        assert(strcmp(message, "Commands are limited to two significant words.") == 0);
#endif
}

static void
check_stopword_directions(void)
{
        char tokens[2][WORDSIZE];
        size_t count = 0;
        int status;

        status = adventure_parse_command("o", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 1);
        assert(strcmp(tokens[0], "W") == 0);

        status = adventure_parse_command("vai o", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GO") == 0);
        assert(strcmp(tokens[1], "W") == 0);

        status = adventure_parse_command("pegar o lanterna", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GET") == 0);
        assert(strcmp(tokens[1], "LAMP") == 0);

        status = adventure_parse_command("no", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 1);
        assert(strcmp(tokens[0], "NW") == 0);

        status = adventure_parse_command("ir no", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GO") == 0);
        assert(strcmp(tokens[1], "NW") == 0);

        status = adventure_parse_command("pegar no lanterna", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GET") == 0);
        assert(strcmp(tokens[1], "LAMP") == 0);

        status = adventure_parse_command("go e", tokens, &count);
        if (status != PARSE_OK)
                abort();
        assert(status == PARSE_OK);
        assert(count == 2);
        assert(strcmp(tokens[0], "GO") == 0);
        assert(strcmp(tokens[1], "E") == 0);
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

        check_compiled_translation(target, expected);
}

static void
check_unknown_flag_translation(void)
{
        const char *translated = _("unknown flag: %c\n");

        check_compiled_translation("unknown flag: %c\n", "opção desconhecida: %c\n");
#ifdef HAVE_GETTEXT
        if (expect_portuguese_translation()) {
                if (strcmp(translated, "opção desconhecida: %c\n") != 0)
                        abort();
                assert(strcmp(translated, "opção desconhecida: %c\n") == 0);
        } else if (strcmp(translated, "opção desconhecida: %c\n") == 0) {
                assert(strcmp(translated, "opção desconhecida: %c\n") == 0);
        } else {
                assert(strcmp(translated, "unknown flag: %c\n") == 0);
        }
#else
        assert(strcmp(translated, "unknown flag: %c\n") == 0);
#endif
}

int
main(void)
{
        configure_locale();
        check_single_word();
        check_two_word_with_stop();
        check_direction_synonym();
        check_too_many_words();
        check_stopword_directions();
        check_catalog_translation();
        check_unknown_flag_translation();
        return 0;
}
