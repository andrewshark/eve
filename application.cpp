#include <application.h>

#ifndef GUI_MODE
#include <console.h>
#endif

// Application

const char_t* Application::WINDOW_CLASS = STR("WINDOW_CLASS");
Application* Application::_application = NULL;

Application::~Application()
{
    if (_window)
    {
#ifdef GUI_MODE
        DestroyWindow(reinterpret_cast<HWND>(_window));
#else
        onDestroy();
        _window = 0;
#endif
    }
}

void Application::run()
{
    registerClass();
    createWindow(_title);
    showWindow();

#ifdef GUI_MODE
    while (_window)
    {
        MSG msg;

        if (GetMessage(&msg, NULL, 0, 0) != -1)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#else
    while (_window)
        onInput(Console::readInput());
#endif
}

#ifdef GUI_MODE

void Application::showMessage(const String& message)
{
    MessageBox(NULL, reinterpret_cast<LPCWSTR>(message.chars()),
               L"Message", MB_OK | MB_TASKMODAL);
}

void Application::showMessage(const char_t* message)
{
    MessageBox(NULL, reinterpret_cast<LPCWSTR>(message),
               L"Message", MB_OK | MB_TASKMODAL);
}

void Application::reportError(const String& message)
{
    MessageBox(NULL, reinterpret_cast<LPCWSTR>(message.chars()),
               L"Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

void Application::reportError(const char_t* message)
{
    MessageBox(NULL, reinterpret_cast<LPCWSTR>(message),
               L"Error", MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

#else

void Application::showMessage(const String& message)
{
    Console::writeLine(message);
}

void Application::showMessage(const char_t* message)
{
    Console::writeLine(message);
}

void Application::reportError(const String& message)
{
    Console::writeLine(message);
}

void Application::reportError(const char_t* message)
{
    Console::writeLine(message);
}

#endif

void Application::createWindow(const char_t* title, int width, int height)
{
    if (_window)
        throw Exception(STR("window already created"));

#ifdef GUI_MODE
    _window = reinterpret_cast<uintptr_t>(CreateWindow(reinterpret_cast<LPCWSTR>(WINDOW_CLASS),
        reinterpret_cast<LPCWSTR>(title), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width > 0 ? width : CW_USEDEFAULT, height > 0 ? height : CW_USEDEFAULT,
        NULL, NULL, GetModuleHandle(NULL), NULL));

    if (!_window)
        throw Exception(STR("failed to create window"));
#else
    _window = 1;
    onCreate();
#endif
}

void Application::showWindow()
{
    if (_window)
    {
#ifdef GUI_MODE
        ShowWindow(reinterpret_cast<HWND>(_window), SW_SHOWDEFAULT);
        UpdateWindow(reinterpret_cast<HWND>(_window));
#endif
    }
    else
        throw Exception(STR("window not created"));
}

void Application::destroyWindow()
{
    if (_window)
    {
#ifdef GUI_MODE
        DestroyWindow(reinterpret_cast<HWND>(_window));
#else
        onDestroy();
        _window = 0;
#endif
    }
    else
        throw Exception(STR("window not created"));
}

void Application::registerClass()
{
#ifdef GUI_MODE
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = reinterpret_cast<LPCWSTR>(WINDOW_CLASS);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wc);
#endif
}

#ifdef GUI_MODE

LRESULT CALLBACK Application::windowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    static Array<InputEvent> inputEvents;

    try
    {
        switch (message)
        {
        case WM_CREATE:
            return _application->onCreate() ? 0 : -1;

        case WM_DESTROY:
            _application->onDestroy();
            _application->_window = 0;
            return 0;

        case WM_PAINT:
            _application->onPaint();
            return 0;

        case WM_ERASEBKGND:
            return 0;

        case WM_KEYDOWN:
            {
                inputEvents.clear();

                KeyEvent keyEvent = { KEY_NONE };
                keyEvent.keyDown = true;

                switch (wParam)
                {
                case VK_ESCAPE:
                    keyEvent.key = KEY_ESC;
                    break;

                case VK_TAB:
                    keyEvent.key = KEY_TAB;
                    keyEvent.ch = '\t';
                    break;

                case VK_BACK:
                    keyEvent.key = KEY_BACKSPACE;
                    break;

                case VK_RETURN:
                    keyEvent.key = KEY_ENTER;
                    keyEvent.ch = '\n';
                    break;

                case VK_UP:
                    keyEvent.key = KEY_UP;
                    break;

                case VK_DOWN:
                    keyEvent.key = KEY_DOWN;
                    break;

                case VK_LEFT:
                    keyEvent.key = KEY_LEFT;
                    break;

                case VK_RIGHT:
                    keyEvent.key = KEY_RIGHT;
                    break;

                case VK_INSERT:
                    keyEvent.key = KEY_INSERT;
                    break;

                case VK_DELETE:
                    keyEvent.key = KEY_DELETE;
                    break;

                case VK_HOME:
                    keyEvent.key = KEY_HOME;
                    break;

                case VK_END:
                    keyEvent.key = KEY_END;
                    break;

                case VK_PRIOR:
                    keyEvent.key = KEY_PGUP;
                    break;

                case VK_NEXT:
                    keyEvent.key = KEY_PGDN;
                    break;

                case VK_F1:
                    keyEvent.key = KEY_F1;
                    break;

                case VK_F2:
                    keyEvent.key = KEY_F2;
                    break;

                case VK_F3:
                    keyEvent.key = KEY_F3;
                    break;

                case VK_F4:
                    keyEvent.key = KEY_F4;
                    break;

                case VK_F5:
                    keyEvent.key = KEY_F5;
                    break;

                case VK_F6:
                    keyEvent.key = KEY_F6;
                    break;

                case VK_F7:
                    keyEvent.key = KEY_F7;
                    break;

                case VK_F8:
                    keyEvent.key = KEY_F8;
                    break;

                case VK_F9:
                    keyEvent.key = KEY_F9;
                    break;

                case VK_F10:
                    keyEvent.key = KEY_F10;
                    break;

                case VK_F11:
                    keyEvent.key = KEY_F11;
                    break;

                case VK_F12:
                    keyEvent.key = KEY_F12;
                    break;
                }

                if (keyEvent.key != KEY_NONE)
                {
                    keyEvent.ctrl = GetKeyState(VK_CONTROL);
                    keyEvent.alt = GetKeyState(VK_MENU);
                    keyEvent.shift = GetKeyState(VK_SHIFT);

                    inputEvents.addLast(keyEvent);
                    _application->onInput(inputEvents);

                    return 0;
                }
            }
            break;

        case WM_CHAR:
            {
                inputEvents.clear();

                KeyEvent keyEvent = { KEY_NONE };
                keyEvent.keyDown = true;

                if (wParam > 0)
                {
                    if (wParam < 0x20)
                        keyEvent.ch = CONTROL_KEYS[wParam];
                    else
                        keyEvent.ch = wParam;

                    keyEvent.ctrl = GetKeyState(VK_CONTROL);
                    keyEvent.alt = GetKeyState(VK_MENU);
                    keyEvent.shift = GetKeyState(VK_SHIFT);

                    inputEvents.addLast(keyEvent);
                    _application->onInput(inputEvents);

                    return 0;
                }
            }
            break;
        }
    }
    catch (Exception& ex)
    {
        Application::reportError(ex.message());
    }
    catch (...)
    {
        Application::reportError(STR("unknown error"));
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

#endif
