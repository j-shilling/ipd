#ifndef __ASPRINTF_H__
#define __ASPRINTF_H__

#include "config.h"

#ifdef HAVE_ASPRINTF

int asprintf (char **strp, const char *fmt, ...);

#else

static int
asprintf (char **strp, const char *fmt, ...)
{
  va_list args, tmp;
  va_start (args, fmt);
  va_copy (tmp, args);

  int size = vsnprintf (NULL, 0, fmt, tmp);
  va_end (tmp);

  if (size < 0)
    return size;

  *strp = malloc (size + 1);

  if (*strp == NULL)
    return -1;

  size = vsprintf (*str, fmt, args);
  va_end (args);

  return size;
}

#endif

#endif /* __ASPRINTF_H__ */
