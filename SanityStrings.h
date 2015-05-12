#ifndef SANITY_STRINGS_H
#define SANITY_STRINGS_H

#include <BeBuild.h>
#ifdef B_ZETA_VERSION
#include <locale/Locale.h>
//#include <FindDirectory.h>
//B_BEOS_ETC_DIRECTORY, 
#define sanity_locale_init() \
{ \
	be_locale.LoadLanguageFile("/etc/Language/Dictionaries/SysApps/Sanity"); \
}
#else
#define _T(s) (s)
#define sanity_locale_init()
#endif

#endif // ifdef SANITY_STRINGS_H
