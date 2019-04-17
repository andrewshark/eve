#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

#include <foundation.h>
#include <input.h>

// Application

class Application
{
public:
    Application(int argc, const char_t** argv, const char_t* title = STR("Application")) :
        _argc(argc), _argv(argv), _window(0), _title(title)
    {
        _application = this;
    }

    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();

    static void showMessage(const String& message);
    static void showMessage(const char_t* message);
    static void showErrorMessage(const String& message);
    static void showErrorMessage(const char_t* message);

protected:
    void createWindow(const char_t* title, int width = 0, int height = 0);
    void showWindow();
    void destroyWindow();

    static void registerClass();

    virtual bool onCreate()
    {
        return true;
    }

    virtual void onDestroy()
    {
    }

    virtual void onPaint()
    {
    }

    virtual void onInput(const Array<InputEvent>& inputEvents)
    {
    }

protected:
    int _argc;
    const char_t** _argv;
    uintptr_t _window;
    const char_t* _title;

    static const char_t* WINDOW_CLASS;
    static Application* _application;

#ifdef GUI_MODE
    static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
#endif
};

#endif
