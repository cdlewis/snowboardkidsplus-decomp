#pragma once

/* Psy-Q 4.2 STDARG.H walks the MIPS stack argument slots. */
typedef void *va_list;

#define PSX_VA_SLOT_SIZE(type) \
    (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(args, last) ((args) = (char *)&(last) + PSX_VA_SLOT_SIZE(last))
#define va_arg(args, type) \
    ((args) = (char *)(args) + PSX_VA_SLOT_SIZE(type), \
     *(type *)((char *)(args) - PSX_VA_SLOT_SIZE(type)))
#define va_end(args) ((args) = (void *)0)
