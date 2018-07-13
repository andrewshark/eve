#ifndef CONSOLE_INCLUDED
#define CONSOLE_INCLUDED

#include <foundation.h>

// console I/O support

void printLine(const char_t* str);
void print(const char_t* format, ...);
void printArgs(const char_t* format, va_list args);

// InputEventType

enum InputEventType
{
    INPUT_EVENT_TYPE_KEY,
    INPUT_EVENT_TYPE_MOUSE,
    INPUT_EVENT_TYPE_WINDOW
};

// Key

enum Key
{
    KEY_NONE,
    KEY_ESC,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PGUP,
    KEY_PGDN,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12
};

// KeyEvent

struct KeyEvent
{
    Key key;
    unichar_t ch;
    bool keyDown;
    bool ctrl, alt, shift;
};

// MouseButton

enum MouseButton
{
    MOUSE_BUTTON_NONE,
    MOUSE_BUTTON_PRIMARY,
    MOUSE_BUTTON_SECONDARY,
    MOUSE_BUTTON_WHEEL,
    MOUSE_BUTTON_WHEEL_UP,
    MOUSE_BUTTON_WHEEL_DOWN
};

// MouseEvent

struct MouseEvent
{
    MouseButton button;
    bool buttonDown;
    int x, y;
    bool ctrl, alt, shift;
};

// WindowEvent

struct WindowEvent
{
    int width, height;
};

// InputEvent

struct InputEvent
{
    InputEventType eventType;

    union
    {
        KeyEvent keyEvent;
        MouseEvent mouseEvent;
        WindowEvent windowEvent;
    } event;

    InputEvent(KeyEvent keyEvent) :
        eventType(INPUT_EVENT_TYPE_KEY)
    {
        event.keyEvent = keyEvent;
    }

    InputEvent(MouseEvent mouseEvent) :
        eventType(INPUT_EVENT_TYPE_MOUSE)
    {
        event.mouseEvent = mouseEvent;
    }

    InputEvent(WindowEvent windowEvent) :
        eventType(INPUT_EVENT_TYPE_WINDOW)
    {
        event.windowEvent = windowEvent;
    }
};

// ConsoleColor

enum ConsoleColor
{
    CONSOLE_COLOR_DEFAULT = 39,
    CONSOLE_COLOR_BLACK = 30,
    CONSOLE_COLOR_RED = 31,
    CONSOLE_COLOR_GREEN = 32,
    CONSOLE_COLOR_YELLOW = 33,
    CONSOLE_COLOR_BLUE = 34,
    CONSOLE_COLOR_MAGENTA = 35,
    CONSOLE_COLOR_CYAN = 36,
    CONSOLE_COLOR_WHITE = 37,
    CONSOLE_COLOR_BRIGHT_BLACK = 90,
    CONSOLE_COLOR_BRIGHT_RED = 91,
    CONSOLE_COLOR_BRIGHT_GREEN = 92,
    CONSOLE_COLOR_BRIGHT_YELLOW = 93,
    CONSOLE_COLOR_BRIGHT_BLUE = 94,
    CONSOLE_COLOR_BRIGHT_MAGENTA = 95,
    CONSOLE_COLOR_BRIGHT_CYAN = 96,
    CONSOLE_COLOR_BRIGHT_WHITE = 97
};

// Console

class Console
{
public:
    static void setLineMode(bool lineMode);
    static void setColor(ConsoleColor foreground = CONSOLE_COLOR_DEFAULT,
        ConsoleColor background = CONSOLE_COLOR_DEFAULT);

    static void write(const String& str);
    static void write(const char_t* chars, int len = -1);
    static void write(unichar_t ch, int n = 1);

    static void writeLine(const String& str);
    static void writeLine(const char_t* chars, int len = -1);
    static void writeLine(unichar_t ch, int n = 1);
    static void writeLine();

    static void write(int line, int column, const String& str);
    static void write(int line, int column, const char_t* chars, int len = -1);
    static void write(int line, int column, unichar_t ch, int n = 1);

    static void writeFormatted(const char_t* format, ...);
    static void writeFormattedArgs(const char_t* format, va_list args);

    static void writeLineFormatted(const char_t* format, ...);
    static void writeLineFormattedArgs(const char_t* format, va_list args);

    static unichar_t readChar();
    static String readLine();

    static void getSize(int& width, int& height);
    static void clear();

    static void showCursor(bool show);
    static void setCursorPosition(int line, int column);

    static const Array<InputEvent>& readInput();

protected:
#ifdef PLATFORM_WINDOWS
    static Buffer<INPUT_RECORD> _inputRecords;
    static WORD _defaultForeground, _defaultBackground;
#else
    static Array<char> _inputChars;
#endif

    static Array<InputEvent> _inputEvents;

    struct Constructor
    {
        Constructor();
        ~Constructor();
    };

    static Constructor constructor;
};

#endif
