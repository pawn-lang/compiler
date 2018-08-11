/* prototypes for strlcpy() and strlcat() */

#include <stddef.h>

#if defined __WATCOMC__ && __WATCOMC__ >= 1240
  /* OpenWatcom introduced BSD "safe string functions" with version 1.4 */
  #define HAVE_SAFESTR
#endif

#if defined strlcpy && !defined HAVE_STRLCPY
  #define HAVE_STRLCPY
#endif
#if defined strlcat && !defined HAVE_STRLCAT
  #define HAVE_STRLCAT
#endif

#if defined HAVE_SAFESTR
  #define HAVE_STRLCPY
  #define HAVE_STRLCAT
#endif

#if !defined HAVE_STRLCPY

size_t
strlcpy(char *dst, const char *src, size_t siz);

#endif

#if !defined HAVE_STRLCAT

size_t
strlcat(char *dst, const char *src, size_t siz);

#endif
