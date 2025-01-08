#include <iostream>
#include "wrench.h"
#include <emscripten/emscripten.h>
#include "stdint.h"

int main()
{
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

int size = 0;

EXTERN EMSCRIPTEN_KEEPALIVE int getSize() {
    return size;
}


EXTERN EMSCRIPTEN_KEEPALIVE uint8_t* compile(char* p) {


    std::cout << "Hello, World2!" << std::endl;


    size_t length = strlen(p);

    unsigned char* outBytes = nullptr;
    int outLen = 0;
    const int err = wr_compile(p, length, &outBytes, &outLen); // compile it
    size = outLen;
    //return bytes
    return outBytes;
}