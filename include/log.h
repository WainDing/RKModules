#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define log(tag,fmt,...)\
    do {\
        printf("%s:", tag);\
        printf(fmt,##__VA_ARGS__);\
        printf("\n");\
    } while (0)

#endif // __LOG_H__