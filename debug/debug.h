#ifndef DEBUG_H_
#define DEBUG_H_

typedef enum {
    DL_INFO = 0,
    DL_ERROR
} DebugLevel;

void printDebugString(const char * file, int line, const char * method, DebugLevel dl, const char* format, ...);


#define DEBUG_PRINT(LEVEL, FMT, ...)    printDebugString(__FILE__, __LINE__, __PRETTY_FUNCTION__, LEVEL, FMT, ##__VA_ARGS__)

#endif /* DEBUG_H_ */
