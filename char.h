#ifndef CHAR_H
#define CHAR_H

#ifdef WIDE_CHAR
#define CHAR wchar_t
#define _S(str) L##str
#define COUT std::wcout
#else
#define CHAR char
#define _S(str) str
#define COUT std::cout
#endif
#endif
