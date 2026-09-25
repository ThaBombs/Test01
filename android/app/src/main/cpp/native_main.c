#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <time.h>

#include "port_runtime.h"
#include "port_gba_timing.h"
#include "port_gba_flash.h"
#include "port_game_engine.h"

#define LOG_TAG "PokeemeraldNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static char sNativeCrashPath[512];
static uintptr_t sNativeLibraryBase;

static size_t AppendLiteral(char *dst, size_t pos, size_t cap, const char *src)
{
    while (*src != '\0' && pos + 1 < cap)
        dst[pos++] = *src++;
    return pos;
}

static size_t AppendHex32(char *dst, size_t pos, size_t cap, uintptr_t value)
{
    static const char hex[] = "0123456789ABCDEF";
    const uint32_t v = (uint32_t)value;
    for (int shift = 28; shift >= 0 && pos + 1 < cap; shift -= 4)
        dst[pos++] = hex[(v >> shift) & 0xFu];
    return pos;
}

static size_t AppendUnsigned(char *dst, size_t pos, size_t cap, unsigned value)
{
    char reverse[12];
    size_t count = 0;
    do
    {
        reverse[count++] = (char)('0' + value % 10u);
        value /= 10u;
    } while (value != 0 && count < sizeof(reverse));

    while (count != 0 && pos + 1 < cap)
        dst[pos++] = reverse[--count];
    return pos;
}

static void NativeCrashHandler(int signalNumber, siginfo_t *info, void *context)
{
    uintptr_t pc = 0;
    uintptr_t lr = 0;

#if defined(__aarch64__)
    ucontext_t *uc = (ucontext_t *)context;
    if (uc != NULL)
    {
        pc = (uintptr_t)uc->uc_mcontext.pc;
        lr = (uintptr_t)uc->uc_mcontext.regs[30];
    }
#elif defined(__arm__)
    ucontext_t *uc = (ucontext_t *)context;
    if (uc != NULL)
    {
        pc = (uintptr_t)uc->uc_mcontext.arm_pc;
        lr = (uintptr_t)uc->uc_mcontext.arm_lr;
    }
#elif defined(__x86_64__)
    ucontext_t *uc = (ucontext_t *)context;
    if (uc != NULL)
    {
        pc = (uintptr_t)uc->uc_mcontext.gregs[REG_RIP];
    }
#endif

    char message[96];
    size_t pos = 0;
    pos = AppendLiteral(message, pos, sizeof(message), "X");
    pos = AppendUnsigned(message, pos, sizeof(message), (unsigned)signalNumber);
    pos = AppendLiteral(message, pos, sizeof(message), " P");
    pos = AppendHex32(message, pos, sizeof(message),
                      pc >= sNativeLibraryBase ? pc - sNativeLibraryBase : pc);
    pos = AppendLiteral(message, pos, sizeof(message), " L");
    pos = AppendHex32(message, pos, sizeof(message),
                      lr >= sNativeLibraryBase ? lr - sNativeLibraryBase : lr);
    pos = AppendLiteral(message, pos, sizeof(message), " F");
    pos = AppendHex32(message, pos, sizeof(message),
                      info != NULL ? (uintptr_t)info->si_addr : 0u);
    if (pos + 1 < sizeof(message))
        message[pos++] = '\n';
    message[pos] = '\0';

    if (sNativeCrashPath[0] != '\0')
    {
        const int fd = open(sNativeCrashPath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd >= 0)
        {
            (void)write(fd, message, pos);
            close(fd);
        }
    }

    signal(signalNumber, SIG_DFL);
    raise(signalNumber);
}

static void InstallNativeCrashHandler(const char *storagePath)
{
    if (storagePath != NULL && storagePath[0] != '\0')
        snprintf(sNativeCrashPath, sizeof(sNativeCrashPath),
                 "%s/battle_init_stage.txt", storagePath);

    Dl_info info;
    memset(&info, 0, sizeof(info));
    if (dladdr((const void *)&android_main, &info) != 0 && info.dli_fbase != NULL)
        sNativeLibraryBase = (uintptr_t)info.dli_fbase;

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = NativeCrashHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;

    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGBUS, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGILL, &action, NULL);
    sigaction(SIGFPE, &action, NULL);
}

struct AndroidEngine
{
    struct android_app *app;
    struct PortInputState input;
    uint32_t keyboardButtons;
    uint32_t touchButtons;
    uint32_t latchedButtons;
    uint32_t previousHeldButtons;
    int surfaceWidth;
    int surfaceHeight;
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
    case AKEYCODE_BUTTON_R2:
    case AKEYCODE_F:
        return PORT_BUTTON_FAST_FORWARD;
    default:
        return 0;
    }
}

static void RefreshCombinedButtons(struct AndroidEngine *engine)
{
    const uint32_t heldButtons = engine->keyboardButtons | engine->touchButtons;
    engine->latchedButtons |= heldButtons & ~engine->previousHeldButtons;
    engine->previousHeldButtons = heldButtons;
    engine->input.buttons = heldButtons | engine->latchedButtons;
}

static void ConsumeLatchedButtons(struct AndroidEngine *engine)
{
    engine->latchedButtons = 0;
    engine->input.buttons = engine->keyboardButtons | engine->touchButtons;
}

static int32_t HandleInput(struct android_app *app, AInputEvent *event)
{
    struct AndroidEngine *engine = (struct AndroidEngine *)app->userData;
    const int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        const int32_t rawAction = AMotionEvent_getAction(event);
        const int32_t action = rawAction & AMOTION_EVENT_ACTION_MASK;

        if (PortRuntime_IsTouchLayoutEditing())
        {
            if (action == AMOTION_EVENT_ACTION_CANCEL)
            {
                PortRuntime_TouchEditorPointer(0.0f, 0.0f, false,
                                               engine->surfaceWidth, engine->surfaceHeight);
            }
            else
            {
                const size_t pointerCount = AMotionEvent_getPointerCount(event);
                size_t index = 0;
                if (action == AMOTION_EVENT_ACTION_POINTER_UP || action == AMOTION_EVENT_ACTION_UP)
                {
                    index = (size_t)((rawAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                        >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
                    if (index >= pointerCount)
                        index = 0;
                }

                const float x = AMotionEvent_getX(event, index);
                const float y = AMotionEvent_getY(event, index);
                const bool down =
                    action != AMOTION_EVENT_ACTION_UP
                    && action != AMOTION_EVENT_ACTION_POINTER_UP;
                PortRuntime_TouchEditorPointer(
                    x, y, down, engine->surfaceWidth, engine->surfaceHeight);
            }

            engine->touchButtons = 0;
            engine->input.pointerDown = false;
            RefreshCombinedButtons(engine);
            return 1;
        }

        if (action == AMOTION_EVENT_ACTION_CANCEL)
        {
            engine->touchButtons = 0;
            engine->input.pointerDown = false;
            RefreshCombinedButtons(engine);
            return 1;
        }

        const size_t pointerCount = AMotionEvent_getPointerCount(event);
        const size_t liftedIndex =
            (size_t)((rawAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);

        uint32_t touchButtons = 0;
        bool havePointer = false;

        for (size_t i = 0; i < pointerCount; ++i)
        {
            if ((action == AMOTION_EVENT_ACTION_UP
                 || action == AMOTION_EVENT_ACTION_POINTER_UP)
                && i == liftedIndex)
            {
                continue;
            }

            const float x = AMotionEvent_getX(event, i);
            const float y = AMotionEvent_getY(event, i);
            touchButtons |= PortRuntime_ButtonsForTouch(
                x, y, engine->surfaceWidth, engine->surfaceHeight);

            if (!havePointer)
            {
                engine->input.pointerX = x;
                engine->input.pointerY = y;
                havePointer = true;
            }
        }

        engine->touchButtons = touchButtons;
        engine->input.pointerDown = havePointer;
        RefreshCombinedButtons(engine);
        return 1;
    }

    if (type == AINPUT_EVENT_TYPE_KEY)
    {
        if (PortRuntime_IsTouchLayoutEditing()
         && AKeyEvent_getKeyCode(event) == AKEYCODE_BACK)
        {
            if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
                PortRuntime_EndTouchLayoutEdit();
            engine->touchButtons = 0;
            RefreshCombinedButtons(engine);
            return 1;
        }

        const uint32_t button = ButtonForKeyCode(AKeyEvent_getKeyCode(event));
        if (button == 0)
            return 0;

        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            engine->keyboardButtons |= button;
        else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP)
            engine->keyboardButtons &= ~button;

        RefreshCombinedButtons(engine);
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
            engine->surfaceWidth = ANativeWindow_getWidth(app->window);
            engine->surfaceHeight = ANativeWindow_getHeight(app->window);
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
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
        (void)PortGbaFlash_Flush();
        break;
    case APP_CMD_LOST_FOCUS:
        engine->input = (struct PortInputState){0};
        engine->keyboardButtons = 0;
        engine->touchButtons = 0;
        engine->latchedButtons = 0;
        engine->previousHeldButtons = 0;
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

    engine->surfaceWidth = buffer.width;
    engine->surfaceHeight = buffer.height;

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
        .keyboardButtons = 0,
        .touchButtons = 0,
        .latchedButtons = 0,
        .previousHeldButtons = 0,
        .surfaceWidth = 0,
        .surfaceHeight = 0,
        .animating = 0,
        .lastFrameNanos = 0,
    };

    app->userData = &engine;
    app->onAppCmd = HandleCommand;
    app->onInputEvent = HandleInput;

    PortRuntime_Init();
    PortRuntime_SetStoragePath(app->activity->internalDataPath);
    InstallNativeCrashHandler(app->activity->internalDataPath);
    PortGbaFlash_Init(app->activity->internalDataPath);
    LOGI("Native runtime started; no GBA ROM or emulator core is embedded.");
    const bool engineReady = PortRuntime_IsGbaHostReady();
    LOGI("GBA host-memory compatibility layer: %s",
         engineReady ? "ready" : "SELF-TEST FAILED");

    if (engineReady)
    {
        Game_Init();
        LOGI("Emerald Game_Init completed; Android now owns the outer frame loop.");
    }

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
                (void)PortGbaFlash_Flush();
                LOGI("Native runtime stopped after %llu frames.",
                     (unsigned long long)PortRuntime_GetFrameCount());
                return;
            }

            if (engine.animating)
                break;
        }

        if (!engine.animating)
            continue;

        PortGbaTiming_WaitForNextFrame();

        const int64_t now = MonotonicNanos();
        double deltaSeconds = 1.0 / 60.0;
        if (engine.lastFrameNanos != 0)
            deltaSeconds = (double)(now - engine.lastFrameNanos) / 1000000000.0;
        engine.lastFrameNanos = now;

        PortRuntime_Step(&engine.input, deltaSeconds);

        int simulatedFrames = 1;
        if (PortRuntime_IsFastForwardEnabled())
            simulatedFrames = PortRuntime_GetFastForwardMultiplier();

        // Fast-forward advances additional Emerald frames inside one Android
        // presentation frame. Each simulated frame still receives a complete
        // visible/VBlank cycle, so timers, movement and tasks stay internally
        // consistent while rendering remains capped to the device cadence.
        for (int i = 0; i < simulatedFrames; ++i)
        {
            PortGbaTiming_BeginVisibleFrame();

            if (engineReady)
                Game_RunFrame();

            PortGbaTiming_EnterVBlank();

            if (engineReady)
                Game_VBlank();

            PortGbaTiming_LeaveVBlank();
        }

        // Preserve a quick tap through the simulated frame batch, then return
        // to the physical held state for the next Android presentation frame.
        ConsumeLatchedButtons(&engine);

        RenderFrame(&engine);
    }
}
