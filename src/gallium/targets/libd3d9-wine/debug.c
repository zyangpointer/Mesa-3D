/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "pipe/p_compiler.h"

#include "debug.h"

#include <stdio.h>
#include <stdarg.h>

enum level
{
    LEVEL_SILENT = 0,
    LEVEL_DEBUG,
    LEVEL_VERBOSE
};

static enum level
get_level( void )
{
    static boolean first = TRUE;
    static enum level lvl;

    if (first) {
        const char *env;
        first = FALSE;

        env = getenv("LIBD3D9_DEBUG");

        lvl = LEVEL_SILENT;
        if (env) {
            if (strcmp(env, "debug") == 0) {
                lvl = LEVEL_DEBUG;
            } else if (strcmp(env, "verbose") == 0) {
                lvl = LEVEL_VERBOSE;
            }
        }
    }

    return lvl;
}


static INLINE void
dbg( const char *prepend,
     const char *fmt,
     va_list args )
{
    fprintf(stderr, "%s", prepend);
    vfprintf(stderr, fmt, args);
}

void
_ERROR( const char *fmt,
        ... )
{
    va_list ap;

    if (get_level() >= LEVEL_DEBUG) {
        va_start(ap, fmt);
        dbg("libd3d9-wine ERROR: ", fmt, ap);
        va_end(ap);
    }
}

void
_WARNING( const char *fmt,
          ... )
{
    va_list ap;

    if (get_level() >= LEVEL_DEBUG) {
        va_start(ap, fmt);
        dbg("libd3d9-wine WARNING: ", fmt, ap);
        va_end(ap);
    }
}

void
_MESSAGE( const char *fmt,
          ... )
{
    va_list ap;

    if (get_level() >= LEVEL_VERBOSE) {
        va_start(ap, fmt);
        dbg("libd3d9-wine MESSAGE: ", fmt, ap);
        va_end(ap);
    }
}