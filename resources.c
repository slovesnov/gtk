#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.resources"), aligned (sizeof(void *) > 8 ? sizeof(void *) : 8)))
#else
# define SECTION
#endif

static const SECTION union { const guint8 data[1325]; const double alignment; void * const ptr;}  resources_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\254\000\000\000\000\000\000\050\005\000\000\000"
  "\000\000\000\000\003\000\000\000\003\000\000\000\004\000\000\000"
  "\004\000\000\000\375\175\155\077\004\000\000\000\254\000\000\000"
  "\014\000\166\000\270\000\000\000\004\005\000\000\324\265\002\000"
  "\377\377\377\377\004\005\000\000\001\000\114\000\010\005\000\000"
  "\014\005\000\000\375\337\223\021\003\000\000\000\014\005\000\000"
  "\010\000\114\000\024\005\000\000\030\005\000\000\302\257\211\013"
  "\001\000\000\000\030\005\000\000\004\000\114\000\034\005\000\000"
  "\040\005\000\000\023\047\024\311\002\000\000\000\040\005\000\000"
  "\006\000\114\000\050\005\000\000\054\005\000\000\141\160\160\055"
  "\151\143\157\156\056\160\156\147\074\004\000\000\000\000\000\000"
  "\211\120\116\107\015\012\032\012\000\000\000\015\111\110\104\122"
  "\000\000\000\040\000\000\000\040\010\006\000\000\000\163\172\172"
  "\364\000\000\000\011\160\110\131\163\000\000\016\304\000\000\016"
  "\304\001\225\053\016\033\000\000\003\356\111\104\101\124\130\205"
  "\255\327\173\210\125\125\024\006\360\337\014\163\052\112\042\042"
  "\042\044\103\164\212\244\110\243\024\213\212\315\041\053\312\312"
  "\062\054\214\262\014\213\054\063\022\222\050\023\051\062\215\002"
  "\021\211\240\242\007\045\231\212\224\105\311\264\073\106\105\204"
  "\364\064\061\011\315\220\030\206\220\020\221\141\216\014\375\261"
  "\317\255\333\170\147\356\275\123\037\014\314\075\373\261\276\275"
  "\366\267\036\273\303\050\220\027\345\004\334\206\133\061\210\365"
  "\330\020\103\366\153\273\173\165\264\141\364\064\314\301\355\230"
  "\216\316\041\123\006\361\025\336\302\306\030\262\276\377\114\040"
  "\057\312\023\061\253\062\172\045\216\153\221\357\121\364\110\236"
  "\331\022\103\166\250\145\002\171\121\166\041\257\214\336\214\061"
  "\055\032\035\016\107\260\125\362\314\107\061\144\003\015\011\344"
  "\105\071\005\167\140\056\316\150\262\351\036\074\217\235\325\357"
  "\051\130\212\263\232\254\073\210\215\170\023\237\307\220\045\002"
  "\171\121\136\205\217\133\070\115\015\263\143\310\066\327\177\310"
  "\213\162\041\326\265\261\307\214\030\262\236\172\017\114\303\042"
  "\334\202\023\232\054\356\303\033\370\011\135\230\214\073\161\162"
  "\223\165\003\330\200\065\061\144\073\240\043\057\312\163\061\101"
  "\272\237\301\112\355\363\161\077\306\267\161\242\221\360\033\136"
  "\302\313\061\144\175\225\316\156\300\057\235\010\370\000\373\362"
  "\242\174\002\135\061\144\253\061\021\327\343\103\051\304\332\305"
  "\040\266\341\046\114\214\041\173\006\235\225\215\175\330\204\231"
  "\235\222\013\111\002\172\012\373\363\242\174\007\227\141\153\014"
  "\331\165\070\033\253\361\107\013\206\017\342\005\114\212\041\273"
  "\032\133\060\055\057\312\365\330\137\331\070\263\232\333\065\064"
  "\231\220\142\175\016\266\343\307\112\134\175\061\144\113\061\116"
  "\212\224\057\033\254\373\032\167\143\134\014\331\022\034\310\213"
  "\162\076\276\305\027\122\346\074\046\217\164\344\105\371\040\326"
  "\066\071\325\041\274\206\027\143\310\166\103\136\224\347\143\236"
  "\224\047\136\251\211\052\057\312\361\170\100\322\321\251\115\366"
  "\135\336\221\027\345\103\130\323\144\142\015\265\173\135\253\022"
  "\155\145\224\224\051\027\341\132\377\134\153\063\054\157\165\042"
  "\174\207\037\244\324\174\015\166\346\105\071\103\012\255\117\244"
  "\144\324\066\032\151\240\021\376\304\345\061\144\363\060\125\022"
  "\343\371\122\050\115\037\306\370\100\203\157\243\046\320\033\103"
  "\166\030\142\310\366\140\131\335\330\120\141\035\306\215\070\176"
  "\310\274\266\011\354\306\256\352\377\356\274\050\307\326\215\155"
  "\033\141\217\305\061\144\357\305\220\301\333\243\045\260\027\027"
  "\342\074\351\024\135\130\127\145\060\070\175\230\075\372\207\030"
  "\155\126\234\206\045\260\063\206\254\277\072\305\323\122\214\317"
  "\302\366\274\050\237\224\252\131\043\364\127\177\265\310\130\122"
  "\067\266\027\257\112\041\335\220\300\056\111\345\160\161\355\264"
  "\025\211\367\253\357\227\142\005\272\207\041\160\012\036\315\213"
  "\362\002\274\216\231\325\367\035\230\034\103\166\217\124\266\217"
  "\041\360\073\056\222\252\332\263\030\213\107\352\346\235\064\214"
  "\301\106\130\205\357\245\352\130\303\342\232\210\053\133\177\243"
  "\166\247\275\061\144\065\327\055\223\072\241\225\171\121\116\302"
  "\317\130\330\006\201\241\030\220\172\305\032\146\324\017\326\074"
  "\160\116\136\224\143\040\206\354\250\244\362\116\334\205\225\232"
  "\327\371\221\320\045\171\124\136\224\001\367\326\215\015\326\010"
  "\214\301\252\112\070\064\317\341\355\240\023\237\346\105\271\111"
  "\352\272\352\363\106\177\027\066\143\201\344\346\356\274\050\017"
  "\110\225\353\377\104\267\143\205\273\013\233\073\143\310\016\110"
  "\061\277\100\112\257\363\265\236\041\107\203\136\251\333\232\034"
  "\103\266\367\137\155\171\365\016\170\130\012\225\126\356\375\076"
  "\251\001\171\267\205\271\207\245\106\345\271\272\210\150\374\060"
  "\251\372\302\307\245\153\031\351\061\322\012\201\001\051\001\255"
  "\210\041\353\035\072\330\354\145\064\136\152\241\346\152\174\055"
  "\043\021\030\224\336\000\313\252\002\326\020\055\275\015\253\314"
  "\266\122\152\066\132\041\320\203\307\152\135\322\110\150\371\161"
  "\132\021\271\102\312\164\323\207\041\260\243\062\334\323\352\236"
  "\155\251\075\206\354\063\134\042\265\332\337\110\205\345\210\324"
  "\055\315\306\324\166\214\303\137\041\134\073\154\037\341\174\136"
  "\000\000\000\000\111\105\116\104\256\102\140\202\000\000\050\165"
  "\165\141\171\051\057\000\000\000\003\000\000\000\145\170\141\155"
  "\160\154\145\057\004\000\000\000\143\157\155\057\002\000\000\000"
  "\155\171\141\160\160\057\000\000\000\000\000\000" };

static GStaticResource static_resource = { resources_resource_data.data, sizeof (resources_resource_data.data) - 1 /* nul terminator */, NULL, NULL, NULL };

G_MODULE_EXPORT
GResource *resources_get_resource (void);
GResource *resources_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_CONSTRUCTOR_H__
#define __G_CONSTRUCTOR_H__

/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a usable way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER)

/*
 * Only try to include gslist.h if not already included via glib.h,
 * so that items using gconstructor.h outside of GLib (such as
 * GResources) continue to build properly.
 */
#ifndef __G_LIB_H__
#include "gslist.h"
#endif

#include <stdlib.h>

#define G_HAS_CONSTRUCTORS 1

/* We do some weird things to avoid the constructors being optimized
 * away on VS2015 if WholeProgramOptimization is enabled. First we
 * make a reference to the array from the wrapper to make sure its
 * references. Then we use a pragma to make sure the wrapper function
 * symbol is always included at the link stage. Also, the symbols
 * need to be extern (but not dllexport), even though they are not
 * really used from another object file.
 */

/* We need to account for differences between the mangling of symbols
 * for x86 and x64/ARM/ARM64 programs, as symbols on x86 are prefixed
 * with an underscore but symbols on x64/ARM/ARM64 are not.
 */
#ifdef _M_IX86
#define G_MSVC_SYMBOL_PREFIX "_"
#else
#define G_MSVC_SYMBOL_PREFIX ""
#endif

#define G_DEFINE_CONSTRUCTOR(_func) G_MSVC_CTOR (_func, G_MSVC_SYMBOL_PREFIX)
#define G_DEFINE_DESTRUCTOR(_func) G_MSVC_DTOR (_func, G_MSVC_SYMBOL_PREFIX)

#define G_MSVC_CTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _wrapper(void);              \
  int _func ## _wrapper(void) { _func(); g_slist_find (NULL,  _array ## _func); return 0; } \
  __pragma(comment(linker,"/include:" _sym_prefix # _func "_wrapper")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _wrapper;

#define G_MSVC_DTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _constructor(void);              \
  int _func ## _constructor(void) { atexit (_func); g_slist_find (NULL,  _array ## _func); return 0; } \
   __pragma(comment(linker,"/include:" _sym_prefix # _func "_constructor")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */
#endif /* __G_CONSTRUCTOR_H__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resourcesresource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resourcesresource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resourcesresource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resourcesresource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resourcesresource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resourcesresource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
