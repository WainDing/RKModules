#include <stdio.h>

#define log(fmt,...)\
    do {\
        printf(fmt,##__VA_ARGS__);\
        printf("\n");\
    } while (0)

int main(int argc, char **argv) {
    log("video_encode2save_mp4");

    return 0;
}