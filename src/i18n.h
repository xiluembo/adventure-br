#ifndef ADVENTURE_I18N_H
#define ADVENTURE_I18N_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_GETTEXT) && HAVE_GETTEXT
#include <libintl.h>
#else
static inline const char *adventure_gettext(const char *msgid)
{
        return msgid;
}

static inline const char *adventure_dgettext(const char *domain, const char *msgid)
{
        (void)domain;
        return msgid;
}

static inline char *adventure_bindtextdomain(const char *domainname, const char *dirname)
{
        (void)domainname;
        return (char *)(dirname ? dirname : "");
}

static inline char *adventure_bind_textdomain_codeset(const char *domainname, const char *codeset)
{
        (void)domainname;
        return (char *)(codeset ? codeset : "");
}

static inline char *adventure_textdomain(const char *domainname)
{
        return (char *)(domainname ? domainname : "");
}

#define gettext(String) adventure_gettext(String)
#define dgettext(Domain, String) adventure_dgettext(Domain, String)
#define bindtextdomain(Domain, Dir) adventure_bindtextdomain(Domain, Dir)
#define bind_textdomain_codeset(Domain, Codeset) adventure_bind_textdomain_codeset(Domain, Codeset)
#define textdomain(Domain) adventure_textdomain(Domain)
#endif

#ifndef _
#define _(String) gettext(String)
#endif

#ifndef N_
#define N_(String) String
#endif

#endif /* ADVENTURE_I18N_H */
