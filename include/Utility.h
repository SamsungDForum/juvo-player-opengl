#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>

#define LogGLErrors() logGLErrors(__FILE__, __LINE__)

class Utility {
  Utility() = delete;

public:
  static void logGLErrors(const char *filename, int line);
  static std::string getGLErrorString(int err);
};

#endif // _UTILITY_H_
