#ifndef EDITOR_INCLUDED
#define EDITOR_INCLUDED

#include <foundation.h>
#include <console.h>
#include <file.h>

// ScreenCell

struct ScreenCell
{
    unichar_t ch;
    short color;

    ScreenCell() :
        ch(0), color(39)
    {
    }

    friend bool operator==(const ScreenCell& left, const ScreenCell& right)
    {
        return left.ch == right.ch && left.color == right.color;
    }
};

// TokenType

enum TokenType
{
    TOKEN_TYPE_NONE,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_IDENT,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_TYPE,
    TOKEN_TYPE_SINGLELINE_COMMENT,
    TOKEN_TYPE_MULTILINE_COMMENT,
    TOKEN_TYPE_PREPROCESSOR
};

// Document

class Document
{
public:
    Document();

    const String& text() const
    {
        return _text;
    }

    int position() const
    {
        return _position;
    }

    bool modified() const
    {
        return _modified;
    }

    void filename(const String& filename)
    {
        ASSERT(!filename.empty());
        _filename = filename;
    }

    const String& filename() const
    {
        return _filename;
    }

    TextEncoding encoding() const
    {
        return _encoding;
    }

    bool crlf() const
    {
        return _crLf;
    }

    int line() const
    {
        return _line;
    }

    int column() const
    {
        return _column;
    }

    int top() const
    {
        return _top;
    }

    int left() const
    {
        return _left;
    }

    int x() const
    {
        return _x;
    }

    int y() const
    {
        return _y;
    }

    int width() const
    {
        return _width;
    }

    int height() const
    {
        return _height;
    }

    int selection() const
    {
        return _selection;
    }

    bool moveForward();
    bool moveBack();

    bool moveWordForward();
    bool moveWordBack();

    bool moveCharsForward();
    bool moveCharsBack();

    bool moveToStart();
    bool moveToEnd();

    bool moveToLineStart();
    bool moveToLineEnd();

    bool moveLines(int lines);
    bool moveToLine(int line);
    bool moveToLineColumn(int line, int column);

    void insertNewLine();
    void insertChar(unichar_t ch, bool afterIdent = false);

    bool deleteCharForward();
    bool deleteCharBack();

    bool deleteWordForward();
    bool deleteWordBack();

    bool deleteCharsForward();
    bool deleteCharsBack();

    void indentLines();
    void unindentLines();
    void commentLines();
    void uncommentLines();

    void markSelection();
    String copyDeleteText(bool copy);
    void pasteText(const String& text);
    bool toggleSelectionStart();

    String currentWord() const;
    String autocompletePrefix() const;
    void completeWord(const char_t* suffix);

    bool find(const String& searchStr, bool caseSesitive, bool next);
    bool replace(const String& searchStr, const String& replaceStr, bool caseSesitive);
    bool replaceAll(const String& searchStr, const String& replaceStr, bool caseSesitive);

    void open(const String& filename);
    void save();
    void clear();

    void setDimensions(int x, int y, int width, int height);
    void draw(int screenWidth, Buffer<ScreenCell>& screen, bool unicodeLimit16);

protected:
    void setPositionLineColumn(int pos);
    void positionToLineColumn(int startPos, int startLine, int startColumn,
        int newPos, int& line, int& column);

    void lineColumnToPosition(int startPos, int startLine, int startColumn,
        int newLine, int newColumn, int& pos, int& line, int& column);

    int findLineStart(int pos) const;
    int findLineEnd(int pos) const;
    int findNextLine(int pos) const;
    int findPreviousLine(int pos) const;

    int findWordForward(int pos) const;
    int findWordBack(int pos) const;
    int findCharsForward(int pos) const;
    int findCharsBack(int pos) const;

    int findPosition(int pos, const String& searchStr, bool caseSesitive, bool next) const;

    void changeLines(int(Document::* lineOp)(int));

    int indentLine(int pos);
    int unindentLine(int pos);
    int commentLine(int pos);
    int uncommentLine(int pos);

    void trimTrailingWhitespace();

    void populateKeywords();
    void highlightChar(int p, unichar_t ch);

protected:
    String _text;
    int _position;
    bool _modified;

    String _filename;
    TextEncoding _encoding;
    bool _bom;
    bool _crLf;

    int _line, _column;
    int _preferredColumn;

    int _top, _left;
    int _topPosition;

    int _x, _y;
    int _width, _height;

    int _selection;
    bool _selectionMode;

    String _indent;

    Set<String> _keywords;
    Set<String> _types;
    Set<String> _preprocessor;

    bool _enableHighlighting;
    bool _brightBackground;
    TokenType _tokenType;
    int _charsRemaining;
    String _word;
    unichar_t _quote, _prevCh;

    static const int _brightBackgroundColors[9];
    static const int _darkBackgroundColors[9];
};

// RecentLocation

struct RecentLocation
{
    ListNode<Document>* document;
    int line;

    RecentLocation(ListNode<Document>* document, int line) :
        document(document), line(line)
    {
    }
};

// AutocompleteSuggestion

struct AutocompleteSuggestion
{
    String word;
    int rank;

    AutocompleteSuggestion(const String& word, int rank) :
        word(word), rank(rank)
    {
    }

    friend void swap(AutocompleteSuggestion& left, AutocompleteSuggestion& right)
    {
        swap(left.word, right.word);
        swap(left.rank, right.rank);
    }

    friend bool operator<(const AutocompleteSuggestion& left, const AutocompleteSuggestion& right)
    {
        if (left.rank > right.rank)
            return true;
        else if (left.rank == right.rank)
            return left.word > right.word;
        else
            return false;
    }
};

// Editor

class Editor
{
public:
    Editor();
    ~Editor();

    void newDocument(const String& filename);
    void openDocument(const String& filename);
    void saveDocument();
    void saveAllDocuments();
    void closeDocument();

    void run();

protected:
    void setDimensions();
    void updateScreen(bool redrawAll);
    bool addCharToOutput(int p, int& color);
    void updateStatusLine();

    bool processInput();
    void showCommandLine();
    void buildProject();

    bool processCommand(const String& command);

    void updateRecentLocations();
    bool moveToNextRecentLocation();
    bool moveToPrevRecentLocation();

    void findUniqueWords();
    void prepareSuggestions(const String& prefix);
    bool completeWord(int next);

    void copyToClipboard(const String& text);
    void pasteFromClipboard(String& text);

protected:
    List<Document> _documents;
    ListNode<Document> _commandLine;
    ListNode<Document>* _document;
    ListNode<Document>* _lastDocument;

    Array<InputEvent> _inputEvents;
    bool _recordingMacro;
    Array<InputEvent> _macro;

    int _width, _height;
    bool _unicodeLimit16;

    Buffer<ScreenCell> _screen;
    Buffer<ScreenCell> _prevScreen;
    String _output;

    String _status, _message;

    String _buffer;
    String _searchStr, _replaceStr;
    bool _caseSesitive;

    List<RecentLocation> _recentLocations;
    ListNode<RecentLocation>* _recentLocation;

    Map<String, int> _uniqueWords;
    Array<AutocompleteSuggestion> _suggestions;
    int _currentSuggestion;
};

#endif
