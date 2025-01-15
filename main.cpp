#include <iostream>
#include "wrench.h"
#include <emscripten/emscripten.h>
#include "stdint.h"
#include "stdio.h"
#include "MatrixManager.h"
#include "ControlManager.h"
#include "WrenchWrapper.h"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif


int size = 0;
unsigned char* outBytes = nullptr;
uint32_t* pixels = nullptr;
WRState* w = nullptr;
WRContext* wc = nullptr;
MatrixManager *mm = nullptr;
ControlManager *cm = nullptr;
int main()
{
    pixels = new uint32_t[144];
    for (int i = 0; i < 144; i++)
    {
        pixels[i] = 0;
    }

    mm = new MatrixManager(pixels, false);
    cm = new ControlManager([]()
    {

    });
    return 0;
}




EXTERN EMSCRIPTEN_KEEPALIVE int getSize() {
    return size;
}


EXTERN EMSCRIPTEN_KEEPALIVE uint8_t* compile(char* p) {



    size_t length = strlen(p);

    int outLen = 0;
    const int err = wr_compile(p, length, &outBytes, &outLen); // compile it
    size = outLen;
    //return bytes
    return outBytes;
}

EXTERN EMSCRIPTEN_KEEPALIVE uint32_t* get_leds()
{

    return pixels;
}

EXTERN EMSCRIPTEN_KEEPALIVE void init()
{
    w = wr_newState();
    wr_loadMathLib(w);
    wr_loadStringLib(w);
    wr_loadContainerLib(w);
    wrench_wrapper::register_wrench_functions(w,new ControlElements{cm,mm});
    wc = wr_run(w, outBytes, size);
    wr_setAllocatedMemoryGCHint(w,1000);
    mm->set_tps(30);
    cm->__internal_set_animation(nullptr);
    mm->clear();
    WRValue* result = wr_callFunction(wc, "init");
    if (!result)
    {
        throw std::runtime_error("Error calling function");
    }
}

EXTERN EMSCRIPTEN_KEEPALIVE void destroy()
{
    wr_destroyState(w);
    w = nullptr;
}

EXTERN EMSCRIPTEN_KEEPALIVE void draw()
{
    WRValue* result = wr_callFunction(wc, "draw");

    if (cm->is_animation_running())
    {
        long long start = cm->__internal_get_animation_start();
        float duration = cm->__internal_get_animation_duration();
        const auto p1 = std::chrono::system_clock::now();

        long long time_running = std::chrono::duration_cast<std::chrono::milliseconds>(
                   p1.time_since_epoch()).count() -start;

        bool result = cm->__internal_get_animation()->
                          run((static_cast<float>(time_running) / duration), mm);

        if (result && (static_cast<float>(time_running) / (duration + cm->
            __interal_get_animation_keep_time())) > 1)
        {
            delete cm->__internal_get_animation();
            cm->__internal_set_animation(nullptr);
        }
    }

    if (!result)
    {
        throw std::runtime_error("Error calling function");
    }
}

EXTERN EMSCRIPTEN_KEEPALIVE void sendEvent(int id)
{
    WRValue val;
    wr_makeInt(&val, id);
    wr_callFunction(wc, "on_event", &val, 1);
}

EXTERN EMSCRIPTEN_KEEPALIVE void game_loop()
{
    WRValue* result = wr_callFunction(wc, "game_loop");
    if (!result)
    {
        throw std::runtime_error("Error calling function");
    }
}

EXTERN EMSCRIPTEN_KEEPALIVE float get_tps()
{
    return mm->get_current_tps();
}

EXTERN EMSCRIPTEN_KEEPALIVE uint8_t get_controls()
{
    return cm->get_controls();
}

EXTERN EMSCRIPTEN_KEEPALIVE const uint8_t* get_status()
{
    return reinterpret_cast<const uint8_t*>(cm->get_status_pointer()->data());
}

EXTERN EMSCRIPTEN_KEEPALIVE uint8_t get_status_length()
{
       return strlen(cm->get_status().c_str());
}
