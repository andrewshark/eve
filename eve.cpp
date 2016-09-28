#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

#include <malloc.h>
#include <io.h>
#include <windows.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#define open _wopen
#define close _close
#define read _read
#define write _write

#else
    
#ifdef __sun
#include <alloca.h>
#include <sys/filio.h>
#endif

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#endif

#include <foundation.h>

const int TAB_SIZE = 4;

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

struct Key
{
    KeyCode code = KEY_NONE;
    char_t ch = 0;
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
};

char_t* screen;
char_t* window;
const char_t* filename;

int top, left;
int width, height, screenHeight;

int position;
int line, column, preferredColumn;
int selection;

char_t* text;
int size, capacity;

char_t* buffer;
char_t* pattern;

int inputSize = 10;
#ifdef _WIN32
INPUT_RECORD* input;
#else
char_t* input;
#endif

int keysSize = inputSize;
Key* keys;

void setCharInputMode(bool charMode)
{
#ifndef _WIN32
    struct termios ta;
    tcgetattr(STDIN_FILENO, &ta);

    if (charMode)
    {
        ta.c_lflag &= ~(ECHO | ICANON);
        ta.c_cc[VTIME] = 0;
        ta.c_cc[VMIN] = 1;
    }
    else
        ta.c_lflag |= ECHO | ICANON;

    tcsetattr(STDIN_FILENO, TCSANOW, &ta);
#endif
}

void getConsoleSize(int& width, int& height)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    width = ws.ws_col;
    height = ws.ws_row;
#endif
}

void showCursor(bool show)
{
#ifndef _WIN32
    write(STDOUT_FILENO, show ? "\x1b[?25h" : "\x1b[?25l", 6);
#endif
}

void setCursorPosition(int line, int column)
{
#ifdef _WIN32
    COORD pos;
    pos.X = column - 1;
    pos.Y = line - 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
#else
    char_t cmd[30];
    sprintf(cmd, "\x1b[%d;%dH", line, column);
    write(STDOUT_FILENO, cmd, strlen(cmd));
#endif
}

void writeToConsole(int line, int column, const char_t* chars, int len)
{
#ifdef _WIN32
    DWORD written;
    COORD pos;
    pos.X = column - 1;
    pos.Y = line - 1;
    
    WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),  
        chars, len, pos, &written);
#else
    setCursorPosition(line, column);
    write(STDOUT_FILENO, chars, len);
#endif
}

void clearConsole()
{
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    
    COORD pos = { 0, 0 };
    DWORD size = csbi.dwSize.X * csbi.dwSize.Y, written;
    
    FillConsoleOutputCharacter(handle, ' ', size, pos, &written);
    FillConsoleOutputAttribute(handle, csbi.wAttributes, size, pos, &written);
    SetConsoleCursorPosition(handle, pos);
#else
    write(STDOUT_FILENO, "\x1b[2J", 4);
    setCursorPosition(1, 1);
#endif
}

#ifndef _WIN32

void readRegularKey(const char_t c, Key& key)
{
    key.ch = c;

    if (c == '\t')
        key.code = KEY_TAB;
    else if (c == '\n')
        key.code = KEY_ENTER;
    else if (c == 0x7f)
        key.code = KEY_BACKSPACE;
    else if (iscntrl(c))
        key.ctrl = true;
    else
        key.shift = isupper(c);
}

const char_t* readSpecialKey(const char_t* p, Key& key)
{
    ++p;
    bool read7e = true;

    if (*p == 0x31)
    {
        ++p;

        if (*p == 0x31)
            key.code = KEY_F1;
        else if (*p == 0x32)
            key.code = KEY_F2;
        else if (*p == 0x33)
            key.code = KEY_F3;
        else if (*p == 0x34)
            key.code = KEY_F4;
        else if (*p == 0x35)
            key.code = KEY_F5;
        else if (*p == 0x37)
            key.code = KEY_F6;
        else if (*p == 0x38)
            key.code = KEY_F7;
        else if (*p == 0x39)
            key.code = KEY_F8;
        else if (*p == 0x7e)
        {
            read7e = false;
            key.code = KEY_HOME;
        }
        else
            return p;
    }
    else if (*p == 0x32)
    {
        ++p;

        if (*p == 0x30)
            key.code = KEY_F9;
        else if (*p == 0x31)
            key.code = KEY_F10;
        else if (*p == 0x33)
            key.code = KEY_F11;
        else if (*p == 0x34)
            key.code = KEY_F12;
        else if (*p == 0x7e)
        {
            read7e = false;
            key.code = KEY_INSERT;
        }
        else
            return p;
    }
    else if (*p == 0x33)
        key.code = KEY_DELETE;
    else if (*p == 0x34)
        key.code = KEY_END;
    else if (*p == 0x35)
        key.code = KEY_PGUP;
    else if (*p == 0x36)
        key.code = KEY_PGDN;
    else if (*p == 0x41)
    {
        read7e = false;
        key.code = KEY_UP;
    }
    else if (*p == 0x42)
    {
        read7e = false;
        key.code = KEY_DOWN;
    }
    else if (*p == 0x43)
    {
        read7e = false;
        key.code = KEY_RIGHT;
    }
    else if (*p == 0x44)
    {
        read7e = false;
        key.code = KEY_LEFT;
    }
    else
        return p;

    ++p;
    if (read7e && *p == 0x7e)
        ++p;

    return p;
}

#endif

int readKeys()
{
    int numKeys = 0;

#ifdef _WIN32

    DWORD numInputRec = 0;
    ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), input, inputSize, &numInputRec);

    for (DWORD i = 0; i < numInputRec; ++i)
    {
        INPUT_RECORD& inputRec = input[i];

        if (inputRec.EventType == KEY_EVENT && inputRec.Event.KeyEvent.bKeyDown)
        {
            Key key;

            switch (inputRec.Event.KeyEvent.wVirtualKeyCode)
            {
            case VK_ESCAPE:
                key.code = KEY_ESC;
                break;
            case VK_TAB:
                key.code = KEY_TAB;
                key.ch = '\t';
                break;
            case VK_BACK:
                key.code = KEY_BACKSPACE;
                break;
            case VK_RETURN:
                key.code = KEY_ENTER;
                key.ch = '\n';
                break;
            case VK_UP:
                key.code = KEY_UP;
                break;
            case VK_DOWN:
                key.code = KEY_DOWN;
                break;
            case VK_LEFT:
                key.code = KEY_LEFT;
                break;
            case VK_RIGHT:
                key.code = KEY_RIGHT;
                break;
            case VK_INSERT:
                key.code = KEY_INSERT;
                break;
            case VK_DELETE:
                key.code = KEY_DELETE;
                break;
            case VK_HOME:
                key.code = KEY_HOME;
                break;
            case VK_END:
                key.code = KEY_END;
                break;
            case VK_PRIOR:
                key.code = KEY_PGUP;
                break;
            case VK_NEXT:
                key.code = KEY_PGDN;
                break;
            default:
                key.code = KEY_NONE;
                key.ch = inputRec.Event.KeyEvent.uChar.UnicodeChar;
                break;
            }

            DWORD controlKeys = inputRec.Event.KeyEvent.dwControlKeyState;
            key.ctrl = (controlKeys & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
            key.alt = (controlKeys & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
            key.shift = (controlKeys & SHIFT_PRESSED) != 0;

            if (numKeys == keysSize)
            {
                keysSize *= 2;
                keys = Memory::reallocate<Key>(keys, keysSize);
            }

            keys[numKeys++] = key;
        }
    }

    return numKeys;

#else

    int len = read(STDIN_FILENO, input, inputSize - 1);

    if (len > 0)
    {
        int remaining;
        ioctl(STDIN_FILENO, FIONREAD, &remaining);

        if (remaining > 0)
        {
            if (len + remaining + 1 > inputSize)
            {
                inputSize = len + remaining + 1;
                input = Memory::reallocate<char_t>(input, inputSize);
            }

            len += read(STDIN_FILENO, input + len, remaining);
        }

        input[len] = 0;
        const char_t* p = input;

        while (*p)
        {
            Key key;

            if (*p == 0x1b)
            {
                ++p;

                if (*p == 0x1b)
                {
                    ++p;
                    key.alt = true;

                    if (*p == 0x5b)
                    {
                        p = readSpecialKey(p, key);
                    }
                    else if (*p == 0x4f)
                    {
                        key.ctrl = true;
                        p = readSpecialKey(p, key);
                    }
                    else
                        continue;
                }
                else if (*p == 0x5b)
                {
                    p = readSpecialKey(p, key);
                }
                else if (*p == 0x4f)
                {
                    key.ctrl = true;
                    p = readSpecialKey(p, key);
                }
                else if (*p)
                {
                    key.alt = true;
                    readRegularKey(*p, key);
                    ++p;
                }
                else
                    key.code = KEY_ESC;
            }
            else
            {
                readRegularKey(*p, key);
                ++p;
            }

            if (numKeys == keysSize)
            {
                keysSize *= 2;
                keys = Memory::reallocate<Key>(keys, keysSize);
            }

            keys[numKeys++] = key;
        }
    }

#endif

    return numKeys;
}

bool isIdent(char_t c)
{
    return ISALNUM(c) || c == '_';
}

char_t* findChar(char_t* str, char_t c)
{
    while (*str && *str != c)
        ++str;

    return str;
}

char_t* findCharBack(char_t* start, char_t* str, char_t c)
{
    while (str > start)
        if (*--str == c)
            return str;

    return start;
}

char_t* wordForward(char_t* str)
{
    while (*str)
    {
        if (ISSPACE(*str) && !ISSPACE(*(str + 1)))
            return str + 1;
        ++str;
    }

    return str;
}

char_t* wordBack(char_t* start, char_t* str)
{
    if (str > start)
    {
        --str;
        while (str > start)
        {
            if (ISSPACE(*(str - 1)) && !ISSPACE(*str))
                return str;
            --str;
        }
    }

    return start;
}

char_t* findLine(char_t* str, int line)
{
    while (*str && line > 1)
    {
        if (*str++ == '\n')
            --line;
    }

    return str;
}

void insertChars(const char_t* chars, int pos, int len)
{
    int cap = size + len + 1;
    if (cap > capacity)
    {
        capacity = cap * 2;
        text = Memory::reallocate<char_t>(text, capacity);
    }

    STRMOVE(text + pos + len, text + pos, size - pos + 1);
    STRNCPY(text + pos, chars, len);
    size += len;
    selection = -1;
}

void deleteChars(int pos, int len)
{
    STRMOVE(text + pos, text + pos + len, size - pos - len + 1);
    size -= len;
    selection = -1;
}

void trimTrailingWhitespace()
{
    char_t* p = text;
    char_t* q = p;
    char_t* r = nullptr;

    while (true)
    {
        if (*p == ' ' || *p == '\t')
        {
            if (!r)
                r = q;
        }
        else if (*p == '\n' || !*p)
        {
            if (r)
            {
                q = r;
                r = nullptr;
            }
        }
        else
            r = nullptr;

        *q = *p;

        if (!*p)
            break;

        ++p; ++q;
    }

    size = STRLEN(text);
    selection = -1;
}

char_t* copyChars(const char_t* start, const char_t* end)
{
    int len = end - start;
    char_t* str = Memory::allocate<char_t>(len + 1);
    STRNCPY(str, start, len);
    str[len] = 0;

    return str;
}

void positionToLineColumn()
{
    char_t* p = text;
    char_t* q = text + position;
    line = 1, column = 1;

    while (*p && p < q)
    {
        if (*p == '\n')
        {
            ++line;
            column = 1;
        }
        else if (*p == '\t')
            column = ((column - 1) / TAB_SIZE + 1) * TAB_SIZE + 1;
        else
            ++column;

        ++p;
    }

    preferredColumn = column;
}

void lineColumnToPosition()
{
    char_t* p = text;
    int preferredLine = line;
    line = 1; column = 1;

    while (*p && line < preferredLine)
    {
        if (*p++ == '\n')
            ++line;
    }

    while (*p && *p != '\n' && column < preferredColumn)
    {
        if (*p == '\t')
            column = ((column - 1) / TAB_SIZE + 1) * TAB_SIZE + 1;
        else
            ++column;

        ++p;
    }

    position = p - text;
}

void updateScreen()
{
    if (line < top)
        top = line;
    else if (line >= top + height)
        top = line - height + 1;

    if (column < left)
        left = column;
    else if (column >= left + width)
        left = column - width + 1;

    char_t* p = findLine(text, top);
    char_t* q = window;
    int len = left + width - 1;

    for (int j = 1; j <= height; ++j)
    {
        for (int i = 1; i <= len; ++i)
        {
            char_t c;

            if (*p == '\t')
            {
                c = ' ';
                if (i == ((i - 1) / TAB_SIZE + 1) * TAB_SIZE)
                    ++p;
            }
            else if (*p && *p != '\n')
                c = *p++;
            else
                c = ' ';

            if (i >= left)
                *q++ = c;
        }

        if (*p == '\n')
            ++p;
        else
        {
            p = findChar(p, '\n');
            if (*p == '\n')
                ++p;
        }
    }

    STRSET(q, ' ', width);

    char_t lnCol[30];
    len = SPRINTF(lnCol, 30, STR("%d, %d"), line, column);
    if (len > 0 && len <= width)
        STRNCPY(q + width - len, lnCol, len);

    int i = 0, j = 0;
    bool matching = true;
    int nchars = width * screenHeight;
    
    showCursor(false);
    
    for (; j < nchars; ++j)
    {
        if (window[j] == screen[j])
        {
            if (!matching)
            {
                writeToConsole(i / width + 1, i % width + 1, window + i, j - i);
                i = j;
                matching = true;
            }
        }
        else
        {
            if (matching)
            {
                i = j;
                matching = false;
            }
        }
    }
    
    if (!matching)
        writeToConsole(i / width + 1, i % width + 1, window + i, j - i);
    
    setCursorPosition(line - top + 1, column - left + 1);
    showCursor(true);
    
    swap(window, screen);
}

bool readFile(const char_t* fileName)
{
#ifdef _WIN32
    struct _stat st;
    if (_wstat(fileName, &st) < 0)
        return false;
#else
    struct stat st;
    if (stat(fileName, &st) < 0)
        return false;
#endif

    int file = open(fileName, 
#ifdef _WIN32
    _O_RDONLY | _O_U16TEXT);
#else
    O_RDONLY);
#endif

    if (file < 0)
        return false;

    capacity = st.st_size / sizeof(char_t) + 1;
    text = Memory::allocate<char_t>(capacity);
    
    size = read(file, text, st.st_size) / sizeof(char_t);

    if (size >= 0)
    {
        text[size] = 0;
        close(file);
        return true;
    }
    else
    {
        Memory::deallocate(text);
        close(file);
        return false;
    }
}

bool writeFile(const char_t* fileName)
{
    int file = open(fileName, 
#ifdef _WIN32
        _O_WRONLY | _O_CREAT | _O_TRUNC | _O_U16TEXT,
        _S_IREAD | _S_IWRITE);
#else
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif

    if (file < 0)
        return false;

    if (write(file, text, sizeof(char_t) * size) >= 0)
    {
        close(file);
        return true;
    }
    else
    {
        close(file);
        return false;
    }
}

void openFile()
{
    if (!readFile(filename))
    {
        size = 0;
        capacity = 1;
        text = Memory::allocate<char_t>(capacity);
        *text = 0;
    }

    position = 0;
    line = 1; column = 1;
    preferredColumn = 1;
    selection = -1;
}

void saveFile()
{
    trimTrailingWhitespace();
    lineColumnToPosition();

    if (!writeFile(filename))
        throw Exception(STR("failed to save file"));
}

char_t* getCommand(const char_t* prompt)
{
    int start = STRLEN(prompt), end = start, cmdLine = screenHeight;
    char_t* cmd = (char_t*)alloca(sizeof(char_t) * width);
    
    STRSET(cmd, ' ', width);
    STRNCPY(cmd, prompt, start);
    writeToConsole(cmdLine, 1, cmd, width);
    setCursorPosition(cmdLine, end + 1);

    while (true)
    {
        int numKeys = readKeys();

        for (int i = 0; i < numKeys; ++i)
        {
            Key& key = keys[i];

            if (key.code == KEY_ENTER)
            {
                return copyChars(cmd + start, cmd + end);
            }
            else if (key.code == KEY_ESC)
            {
                return nullptr;
            }
            else if (key.code == KEY_BACKSPACE)
            {
                if (end > start)
                    cmd[--end] = ' ';
            }
            else
            {
                if (end < width)
                    cmd[end++] = key.ch;
            }
            
            writeToConsole(cmdLine, 1, cmd, width);
            setCursorPosition(cmdLine, end + 1);
        }
    }
}

bool deleteWordForward()
{
    int pos = wordForward(text + position) - text;
    if (pos > position)
    {
        deleteChars(position, pos - position);
        return true;
    }

    return false;
}

bool deleteWordBack()
{
    int pos = wordBack(text, text + position) - text;
    if (pos < position)
    {
        deleteChars(pos, position - pos);
        position = pos;
        positionToLineColumn();
        return true;
    }

    return false;
}

bool copyDeleteText(bool copy)
{
    char_t* p;
    char_t* q;

    if (selection < 0)
    {
        p = findCharBack(text, text + position, '\n');
        if (*p == '\n')
            ++p;

        q = findChar(text + position, '\n');
        if (*q == '\n')
            ++q;
    }
    else
    {
        if (selection < position)
        {
            p = text + selection;
            q = text + position;
        }
        else
        {
            p = text + position;
            q = text + selection;
        }

        selection = -1;
    }

    if (p < q)
    {
        Memory::deallocate(buffer);
        buffer = copyChars(p, q);

        if (!copy)
        {
            position = p - text;
            positionToLineColumn();
            deleteChars(position, q - p);
            return true;
        }
    }

    return false;
}

bool pasteText()
{
    if (buffer)
    {
        int len = STRLEN(buffer);
        insertChars(buffer, position, len);
        position += len;
        positionToLineColumn();
        return true;
    }

    return false;
}

void buildProject()
{
    setCharInputMode(false);
    clearConsole();

    saveFile();
    system("gmake");

    printf("Press ENTER to contiue...");
    getchar();

    setCharInputMode(true);
}

bool findNext()
{
    if (pattern)
    {
        char_t* p = text + position;
        char_t* q = STRSTR(p + 1, pattern);
        if (!q)
            q = STRSTR(text, pattern);

        if (q && q != p)
        {
            position = q - text;
            positionToLineColumn();
            return true;
        }
    }

    return false;
}

bool findWordAtCursor()
{
    char_t* p = text + position;
    Memory::deallocate(pattern);

    if (isIdent(*p))
    {
        while (p > text)
        {
            if (!isIdent(*(p - 1)))
                break;
            --p;
        }

        char_t* q = text + position + 1;
        while (*q)
        {
            if (!isIdent(*q))
                break;
            ++q;
        }

        pattern = copyChars(p, q);
    }
    else
        pattern = nullptr;

    return findNext();
}

void insertChar(char_t c)
{
    if (c == '\n') // new line
    {
        char_t* p = findCharBack(text, text + position, '\n');
        if (*p == '\n')
            ++p;

        char_t* q = p;
        while (*q == ' ' || *q == '\t')
            ++q;

        int len = q - p + 1;
        char_t* chars = (char_t*)alloca(sizeof(char_t) * len);

        chars[0] = '\n';
        STRNCPY(chars + 1, p, q - p);

        insertChars(chars, position, len);
        position += len;
    }
    else if (c == '\t') // tab
    {
        char_t chars[16];
        STRSET(chars, ' ', TAB_SIZE);
        insertChars(chars, position, TAB_SIZE);
        position += TAB_SIZE;
    }
    else if (c == 0x14) // real tab
        insertChars(STR("\t"), position++, 1);
    else
        insertChars(&c, position++, 1);

    positionToLineColumn();
}

bool processKey()
{
    bool update = false;
    int numKeys = readKeys();

    for (int i = 0; i < numKeys; ++i)
    {
        Key& key = keys[i];

        if (key.ctrl)
        {
            if (key.code == KEY_RIGHT)
            {
                position = wordForward(text + position) - text;
                positionToLineColumn();
                update = true;
            }
            else if (key.code == KEY_LEFT)
            {
                position = wordBack(text, text + position) - text;
                positionToLineColumn();
                update = true;
            }
            else if (key.code == KEY_TAB || key.ch == 0x14)
            {
                insertChar(0x14);
                update = true;
            }
        }
        else if (key.alt)
        {
            if (key.code == KEY_DELETE)
            {
                update = deleteWordForward();
            }
            else if (key.code == KEY_BACKSPACE)
            {
                update = deleteWordBack();
            }
            else if (key.code == KEY_HOME)
            {
                position = 0;
                positionToLineColumn();
                update = true;
            }
            else if (key.code == KEY_END)
            {
                position = size;
                positionToLineColumn();
                update = true;
            }
            else if (key.code == KEY_PGUP)
            {
                line -= height / 2;
                if (line < 1)
                    line = 1;

                lineColumnToPosition();
                update = true;
            }
            else if (key.code == KEY_PGDN)
            {
                line += height / 2;
                lineColumnToPosition();
                update = true;
            }
            else if (key.ch == 'q')
            {
                return false;
            }
            else if (key.ch == 's')
            {
                saveFile();
                update = true;
            }
            else if (key.ch == 'x')
            {
                saveFile();
                return false;
            }
            else if (key.ch == 'd')
            {
                update = copyDeleteText(false);
            }
            else if (key.ch == 'c')
            {
                update = copyDeleteText(true);
            }
            else if (key.ch == 'p')
            {
                update = pasteText();
            }
            else if (key.ch == 'b')
            {
                buildProject();
                update = true;
            }
            else if (key.ch == 'm')
            {
                selection = position;
            }
            else if (key.ch == 'f')
            {
                Memory::deallocate(pattern);
                pattern = getCommand(STR("find: "));
                findNext();
                update = true;
            }
            else if (key.ch == 'o')
            {
                update = findWordAtCursor();
            }
            else if (key.ch == 'n')
            {
                update = findNext();
            }
        }
        else if (key.code == KEY_BACKSPACE)
        {
            if (position > 0)
            {
                deleteChars(--position, 1);
                positionToLineColumn();
                update = true;
            }
        }
        else if (key.code == KEY_DELETE)
        {
            if (position < size)
            {
                deleteChars(position, 1);
                update = true;
            }
        }
        else if (key.code == KEY_UP)
        {
            if (line > 1)
            {
                --line;
                lineColumnToPosition();
                update = true;
            }
        }
        else if (key.code == KEY_DOWN)
        {
            ++line;
            lineColumnToPosition();
            update = true;
        }
        else if (key.code == KEY_RIGHT)
        {
            if (position < size)
            {
                ++position;
                positionToLineColumn();
                update = true;
            }
        }
        else if (key.code == KEY_LEFT)
        {
            if (position > 0)
            {
                --position;
                positionToLineColumn();
                update = true;
            }
        }
        else if (key.code == KEY_HOME)
        {
            char_t* p = findCharBack(text, text + position, '\n');
            if (*p == '\n')
                ++p;

            position = p - text;
            positionToLineColumn();
            update = true;
        }
        else if (key.code == KEY_END)
        {
            position = findChar(text + position, '\n') - text;
            positionToLineColumn();
            update = true;
        }
        else if (key.code == KEY_PGUP)
        {
            line -= height - 1;
            if (line < 1)
                line = 1;

            lineColumnToPosition();
            update = true;
        }
        else if (key.code == KEY_PGDN)
        {
            line += height - 1;
            lineColumnToPosition();
            update = true;
        }
        else if (key.ch == '\n' || key.ch == '\t' || ISPRINT(key.ch))
        {
            insertChar(key.ch);
            update = true;
        }
    }

    if (update)
        updateScreen();

    return true;
}

void editor()
{
    openFile();

    getConsoleSize(width, screenHeight);
    screen = Memory::allocate<char_t>(width * screenHeight);
    memset(screen, 0, sizeof(char_t) * width * screenHeight);
    window = Memory::allocate<char_t>(width * screenHeight);

    height = screenHeight - 1;
    top = 1; left = 1;

    updateScreen();
    setCharInputMode(true);

#ifdef _WIN32
    input = Memory::allocate<INPUT_RECORD>(inputSize);
#else
    input = Memory::allocate<char_t>(inputSize);
#endif
    keys = Memory::allocate<Key>(keysSize);

    while (processKey());

    setCharInputMode(false);
    clearConsole();

    Memory::deallocate(screen);
    Memory::deallocate(window);
    Memory::deallocate(text);
    Memory::deallocate(buffer);
    Memory::deallocate(pattern);
    Memory::deallocate(input);
    Memory::deallocate(keys);
}

int MAIN(int argc, const char_t** argv)
{
    if (argc != 2)
    {
        printf("usage: eve filename\n\n"
            "keys:\n"
            "arrows - move cursor\n"
            "ctrl+left/right - word back/forward\n"
            "home/end - start/end of line\n"
            "alt+home/end - start/end of file\n"
            "pgup/pgdn - page up/down\n"
            "alt+pgup/pgdn - half page up/down\n"
            "del - delete character at cursor position\n"
            "backspace - delete character to the left of cursor position\n"
            "alt+del - delete word at cursor position\n"
            "alt+backspace - delete word to the left of cursor position\n"
            "alt+M - mark selection start\n"
            "alt+D - delete line/selection\n"
            "alt+C - copy line/selection\n"
            "alt+P - paste line/selection\n"
            "alt+F - find\n"
            "alt+O - find word at cursor\n"
            "alt+N - find again\n"
            "alt+B - build with make\n"
            "alt+S - save\n"
            "alt+X - quit and save\n"
            "alt+Q - quit without saving\n\n");

        return 1;
    }

    try
    {
        filename = argv[1];
        editor();
    }
    catch (Exception& ex)
    {
        Console::writeLine(ex.message());
        return 1;
    }
    catch (...)
    {
        Console::writeLine(STR("unknown error"));
        return 1;
    }

    return 0;
}
