#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

#include <foundation.h>
#include <graphics.h>

// Window

class Window
{
public:
    Window(const char_t* windowClass);
    virtual ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    HWND handle() const
    {
        return _handle;
    }

    void create(const char_t* title, int width, int height);
    void show();

protected:
    virtual void onCreate();
    virtual void onDestroy();
    virtual void onPaint();
    virtual void onResize(int width, int height);

private:
    static LRESULT CALLBACK windowProc(
        HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
    const char_t* _windowClass;
    HWND _handle;

    Unique<Graphics> _graphics;

    static Map<HWND, Window*> _windows;
};

#endif
