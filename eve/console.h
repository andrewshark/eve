#ifndef CONSOLE_INCLUDED
#define CONSOLE_INCLUDED

#include <foundation.h>

// KeyCode

enum KeyCode
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

// Key

struct Key
{
    KeyCode code;
    unichar_t ch;
    bool ctrl;
    bool alt;
    bool shift;

    Key() :
        code(KEY_NONE), ch(0), 
        ctrl(false), alt(false), shift(false)
    {
    }
};

// ConsoleMode

enum ConsoleMode
{
    CONSOLE_MODE_DEFAULT = 0,
    CONSOLE_MODE_NONCANONICAL = 1,
    CONSOLE_MODE_NOTBUFFERED = 2,
    CONSOLE_MODE_UNICODE = 4
};

// Console

class Console
{
public:
    static void setMode(int mode);

    static void write(const String& str);
    static void write(const char_t* chars);
    static void write(unichar_t ch, int len = 1);

    static void writeLine(const String& str);
    static void writeLine(const char_t* chars);
    static void writeLine(unichar_t ch, int len = 1);
    static void writeLine();

    static void write(int line, int column, const String& str);
    static void write(int line, int column, const char_t* chars);
    static void write(int line, int column, unichar_t ch, int len = 1);

    static void writeFormatted(const char_t* format, ...);
    static void writeFormatted(const char_t* format, va_list args);

    static void writeLineFormatted(const char_t* format, ...);
    static void writeLineFormatted(const char_t* format, va_list args);

    static unichar_t readChar()
    {
        return getChar();
    }

    static String readLine();

    static void getSize(int& width, int& height);
    static void clear();

    static void showCursor(bool show);
    static void setCursorPosition(int line, int column);

    static const Array<Key>& readKeys();

protected:
    static void write(const char_t* chars, int len);
    static void write(int line, int column, const char_t* chars, int len);

#ifdef PLATFORM_UNIX
    static const char_t* readRegularKey(const char_t* p, Key& key);
    static const char_t* readSpecialKey(const char_t* p, Key& key);
#endif

protected:
#ifdef PLATFORM_WINDOWS
    static Array<INPUT_RECORD> _input;
#else
    static Array<char> _input;
#endif

    static Array<Key> _keys;
};

#endif
