/*  SSNES - A Super Nintendo Entertainment System (SNES) Emulator frontend for libsnes.
 *  Copyright (C) 2010-2011 - Hans-Kristian Arntzen
 *
 *  Some code herein may be based on code found in BSNES.
 * 
 *  SSNES is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  SSNES is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with SSNES.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SSNES_MSVC_COMPAT_H
#define __SSNES_MSVC_COMPAT_H

#ifdef _MSC_VER

#undef UNICODE // Do not bother with UNICODE at this time.
#include <stddef.h>
#include <math.h>

// Python headers defines ssize_t and sets HAVE_SSIZE_T. Cannot duplicate these efforts.
#ifndef HAVE_SSIZE_T
typedef int ssize_t;
#endif

#define snprintf _snprintf
#define strcasecmp _stricmp

// Disable some of the annoying warnings.
#pragma warning(disable : 4800)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4146)
#pragma warning(disable : 4267)

static inline float roundf(float in)
{
   return in >= 0.0f ? floorf(in + 0.5f) : ceilf(in - 0.5f);
}

#endif
#endif
