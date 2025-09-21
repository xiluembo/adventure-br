#ifndef ADVENTURE_I18N_H
#define ADVENTURE_I18N_H

#include <libintl.h>

#ifndef _
#define _(String) gettext(String)
#endif

#ifndef N_
#define N_(String) String
#endif

#endif /* ADVENTURE_I18N_H */
