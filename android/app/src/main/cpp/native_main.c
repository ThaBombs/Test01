#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <stdint.h>
#include <time.h>

#include "port_runtime.h"
#include "port_gba_timing.h"

#define LOG_TAG "PokeemeraldNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct AndroidEngine
{
    struct android_app *app;
    struct PortInputState input;
    int animating;
    int64_t lastFrameNanos;
};

static int64_t MonotonicNanos(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static uint32_t ButtonForKeyCode(int32_t keyCode)
{
    switch (keyCode)
    {
    case AKEYCODE_BUTTON_A:
    case AKEYCODE_Z:
        return PORT_BUTTON_A;
    case AKEYCODE_BUTTON_B:
    case AKEYCODE_X:
        return PORT_BUTTON_B;
    case AKEYCODE_BUTTON_SELECT:
    case AKEYCODE_BACK:
        return PORT_BUTTON_SELECT;
    case AKEYCODE_BUTTON_START:
    case AKEYCODE_ENTER:
        return PORT_BUTTON_START;
    case AKEYCODE_DPAD_RIGHT:
        return PORT_BUTTON_RIGHT;
    case AKEYCODE_DPAD_LEFT:
        return PORT_BUTTON_LEFT;
    case AKEYCODE_DPAD_UP:
        return PORT_BUTTON_UP;
    case AKEYCODE_DPAD_DOWN:
        return PORT_BUTTON_DOWN;
    case AKEYCODE_BUTTON_R1:
        return PORT_BUTTON_R;
    case AKEYCODE_BUTTON_L1:
        return PORT_BUTTON_L;
    default:
        return 0;
    }
}

static int32_t HandleInput(struct android_app *app, AInputEvent *event)
{
    struct AndroidEngine *engine = (struct AndroidEngine *)app->userData;
    const int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        const int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
        engine->input.pointerX = AMotionEvent_getX(event, 0);
        engine->input.pointerY = AMotionEvent_getY(event, 0);
        engine->input.pointerDown =
            action != AMOTION_EVENT_ACTION_UP
            && action != AMOTION_EVENT_ACTION_CANCEL;
        return 1;
    }

    if (type == AINPUT_EVENT_TYPE_KEY)
    {
        const uint32_t button = ButtonForKeyCode(AKeyEvent_getKeyCode(event));
        if (button == 0)
            return 0;

        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            engine->input.buttons |= button;
        else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP)
            engine->input.buttons &= ~button;

        return 1;
    }

    return 0;
}

static void HandleCommand(struct android_app *app, int32_t command)
{
    struct AndroidEngine *engine = (struct AndroidEngine *)app->userData;

    switch (command)
    {
    case APP_CMD_INIT_WINDOW:
        if (app->window != NULL)
        {
            ANativeWindow_setBuffersGeometry(app->window, 0, 0, WINDOW_FORMAT_RGBX_8888);
            engine->animating = 1;
            engine->lastFrameNanos = MonotonicNanos();
            LOGI("Native Android surface initialized: %dx%d",
                 ANativeWindow_getWidth(app->window),
                 ANativeWindow_getHeight(app->window));
        }
        break;
    case APP_CMD_TERM_WINDOW:
        engine->animating = 0;
        break;
    case APP_CMD_LOST_FOCUS:
        engine->input = (struct PortInputState){0};
        break;
    default:
        break;
    }
}

static void RenderFrame(struct AndroidEngine *engine)
{
    if (engine->app->window == NULL)
        return;

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) != 0)
    {
        LOGE("ANativeWindow_lock failed");
        return;
    }

    PortRuntime_Render(
        (uint32_t *)buffer.bits,
        buffer.width,
        buffer.height,
        buffer.stride);

    ANativeWindow_unlockAndPost(engine->app->window);
}

void android_main(struct android_app *app)
{
    app_dummy();

    struct AndroidEngine engine = {
        .app = app,
        .input = {0},
        .animating = 0,
        .lastFrameNanos = 0,
    };

    app->userData = &engine;
    app->onAppCmd = HandleCommand;
    app->onInputEvent = HandleInput;

    PortRuntime_Init();
    LOGI("Native runtime started; no GBA ROM or emulator core is embedded.");
    LOGI("GBA host-memory compatibility layer: %s",
         PortRuntime_IsGbaHostReady() ? "ready" : "SELF-TEST FAILED");

    while (1)
    {
        int events;
        struct android_poll_source *source;

        while (ALooper_pollOnce(engine.animating ? 0 : -1, NULL, &events, (void **)&source) >= 0)
        {
            if (source != NULL)
                source->process(app, source);

            if (app->destroyRequested != 0)
            {
                LOGI("Native runtime stopped after %llu frames.",
                     (unsigned long long)PortRuntime_GetFrameCount());
                return;
            }

            if (engine.animating)
                break;
        }

        if (!engine.animating)
            continue;

        const int64_t now = MonotonicNanos();
        double deltaSeconds = 1.0 / 60.0;
        if (engine.lastFrameNanos != 0)
            deltaSeconds = (double)(now - engine.lastFrameNanos) / 1000000000.0;
        engine.lastFrameNanos = now;

        PortGbaTiming_WaitForNextFrame();
        PortGbaTiming_BeginVisibleFrame();

        PortRuntime_Step(&engine.input, deltaSeconds);
        RenderFrame(&engine);

        PortGbaTiming_EnterVBlank();
        PortGbaTiming_LeaveVBlank();
    }
}
