#include "res.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

#define POSIX 1
#define WINDOWS 2
#define NEWLINE_MODES_LF 4
#define NEWLINE_MODES_CRLF 8

#define NEWLINE_MODE NEWLINE_MODES_LF

#ifndef PLATFORM
#define PLATFORM WINDOWS
#endif

#if PLATFORM == POSIX
    #include <unistd.h>
    #define ISATTY(fd) isatty(fd)
#elif PLATFORM == WINDOWS
    #include <io.h>
    #define ISATTY(fd) _isatty(fd)
#endif

/* | Sequence | Hexadecimal |
 * | -------- | ----------- |
 * | CR       | 0x0a        |
 * | LF       | 0x0d        |
 */


/*********************
 * --- Constants --- *
 *********************/

const char * HELP_SEQUENCES[] = {
    "--HELP",
    "-H",
    "/?"
};

const char * HELP_MSG[] = {
    "Usage: diosay <string>",
    "",
    "Examples:",
    "  diosay what the",
    "  dir | diosay",
    "  type file.txt | diosay"
};

const int MIN_LENGTH = 40;
const int MAX_LENGTH = 80;
const int INDENT_LENGTH = 4;


/**********************
 * --- Prototypes --- *
 **********************/

int StrCmp(const char * first, const char * secnd);

bool IsHelpSequence(const char * str);

template <typename T>
const T & max(const T & first, const T & secnd);

int len(const char * str);

bool PipelineInput();

std::istream & Getline(std::istream & in, std::string & line);

template <typename T>
int GetTrimmedLength(const T & str, int len);

template <typename T>
void PrintTextWrapped(std::ostream & out, const T & buf, int len, int indent, int max);

std::ostream & Padding(std::ostream & stream, int len);

template <typename T>
void PrintPrettyMessage(std::ostream & out, const T & content, int width);

void PrintResource(std::ostream & out, unsigned char res[], unsigned int size, int indent);


/**********************
 * --- Main Entry --- *
 **********************/

int main(int argc, char ** argv)
{
    SetConsoleOutputCP(CP_UTF8);
    std::string buf;
    std::ostringstream temp;

    int text_length;
    int length = MIN_LENGTH;

    if (PipelineInput())
    {
        while (Getline(std::cin, buf))
        {
            text_length = GetTrimmedLength(buf, (int) buf.length());
            length = max(length, text_length);
            PrintTextWrapped(temp, buf, text_length, INDENT_LENGTH, MAX_LENGTH);
        }
    }

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (IsHelpSequence((const char *) argv[i]))
            {
                temp.str("");

                for (auto help_line : HELP_MSG)
                {
                    text_length = GetTrimmedLength(help_line, len(help_line));
                    length = max(length, text_length);
                    PrintTextWrapped(temp, help_line, text_length, INDENT_LENGTH, MAX_LENGTH);
                }

                break;
            }

            text_length = GetTrimmedLength(argv[i], len(argv[i]));
            length = max(length, text_length);
            PrintTextWrapped(temp, argv[i], text_length, INDENT_LENGTH, MAX_LENGTH);
        }
    }

    bool empty = true;
    buf = temp.str();
    empty = buf.length() == 0;

    if (!empty)
        PrintPrettyMessage(std::cout, buf, length);

    PrintResource(std::cout, res::txt, res::txt_len, length - 31);
    return 0;
}


/***********************
 * --- Definitions --- *
 ***********************/

int StrCmp(const char * first, const char * secnd)
{
    int i = 0;
    int temp = 0;
    int determinant = (first[i] == '\0') + 2 * (secnd[i] == '\0');

    for (; determinant == 0 && temp == 0; ++i)
    {
        temp = (int) std::toupper(first[i]) - (int) std::toupper(secnd[i]);
        determinant = (first[i] == '\0') + 2 * (secnd[i] == '\0');
    }

    switch (determinant)
    {
        case 1: return 0 - (int) secnd[i];
        case 2: return (int) first[i];
        default: return temp;
    }
}

bool IsHelpSequence(const char * str)
{
    for (auto seq : HELP_SEQUENCES)
        if (!StrCmp(str, seq))
            return true;

    return false;
}

template <typename T>
const T & max(const T & first, const T & secnd)
{
    return (first > secnd) ? first : secnd;
}

int len(const char * str)
{
    int i = 0;
    while (str[i] != '\0')
        i = i + 1;
    return i;
}

bool PipelineInput()
{
    return !ISATTY(_fileno(stdin));
}

std::istream & Getline(std::istream & in, std::string & line)
{
    std::ostringstream buf;
    char temp;

    while (in.get(temp) && temp != '\n')
        buf << temp;

    line = buf.str();
    return in;
}

template <typename T>
int GetTrimmedLength(const T & str, int len)
{
    int i = len - 1;
    while (i > 0 && isspace(str[i]))
        i = i - 1;
    return i + 1;
}

template <typename T>
void PrintTextWrapped(std::ostream & out, const T & buf, int len, int indent, int max)
{
    if (len == 0)
    {
        out << std::setw(indent) << out.fill() << '\n';
    }
    else
    {
        std::ostringstream line;
        int count = 0;

        for (int i = 0; i < len; ++i)
        {
            if (count == max)
            {
                out << std::setw(indent) << out.fill() << line.str() << '\n';
                line.str("");
                count = 0;
            }

            line << buf[i];
            count = count + 1;
        }

        std::string temp = line.str();

        if (temp.length() > 0)
        {
            out << std::setw(indent) << out.fill() << temp << '\n';
        }
    }
}

std::ostream & Padding(std::ostream & stream, int len)
{
    stream << std::setw(len) << (len <= 0 ? "" : " ");
    return stream;
}

template <typename T>
void PrintPrettyMessage(std::ostream & out, const T & content, int width)
{
    out << std::setfill('_')
        << "     " << std::setw(width) << "_" << '\n';
    out << std::setfill(' ')
        << "  .' " << std::setw(width) << " " << " '.\n";
    out << " /          You were expecting" << std::setw(width - 25) << " " << "   \\\n\n";

    out << content << '\n';

    out << " \\      ";
    Padding(out, width - 32);
    out << "... BUT IT WAS ME! DIO!!!       /\n";

    out << std::setfill('_')
        << "  '. " << std::setw(width - 4) << "_" << "   _ .'\n";
    out << std::setfill(' ')
        << " " << std::setw(width) << " " << "\\ |\n";
    out << " " << std::setw(width + 1) << " " << "\\'\n";
}

void PrintResource(std::ostream & out, unsigned char res[], unsigned int size, int indent)
{
    int index = 0;
    bool indent_here = true;

    while (index < size)
    {
        if (indent_here)
        {
           Padding(out, indent);
           indent_here = false;
        }

#if NEWLINE_MODE == NEWLINE_MODES_LF
        if (res[index] != 0x0d)
#endif
            out << res[index];

        indent_here = res[index] == '\n';
        index = index + 1;
    }
}
