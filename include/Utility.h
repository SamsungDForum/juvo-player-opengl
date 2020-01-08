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

template<typename T> struct Position {
  T x, y;
  T epsilon = static_cast<T>(1e-9);

  Position<T>() : x{0}, y{0} {
  }

  Position<T>(T x, T y) : x{x}, y{y} {
  }

  template<typename U> Position<T>(const Position<U> &other) {
    x = other.x;
    y = other.y;
  }

  template<typename U> Position<T>& operator=(const Position<U> &other) {
    if((const Position<T>*) &other != this) {
      x = other.x;
      y = other.y;
    }
    return *this;
  }

  ~Position<T>() {
  }

  template<typename U> Position<T>(const Position<U> &&other) {
    x = other.x;
    y = other.y;
  }

  template<typename U> Position<T>& operator=(const Position<U> &&other) {
    if((const Position<T>*) &other != this) {
      x = other.x;
      y = other.y;
    }
    return *this;
  }

  bool operator==(const Position<T> &other) const {
    return abs(x - other.x) <= epsilon && abs(y - other.y) <= epsilon;
  }
};

template<typename T> struct Size {
  T width, height;
  T epsilon = static_cast<T>(1e-9);

  Size<T>() : width{0}, height{0} {
  }

  Size<T>(T width, T height) : width{width}, height{height} {
  }

  template<typename U> Size<T>(const Size<U> &other) {
    width = other.width;
    height = other.height;
  }

  template<typename U> Size<T>& operator=(const Size<U> &other) {
    if((const Size<T>*) &other != this) {
      width = other.width;
      height = other.height;
    }
    return *this;
  }

  ~Size<T>() {
  }

  template<typename U> Size<T>(const Size<U> &&other) {
    width = other.width;
    height = other.height;
  }

  template<typename U> Size<T>& operator=(const Size<U> &&other) {
    if((const Size<T>*) &other != this) {
      width = other.width;
      height = other.height;
    }
    return *this;
  }

  bool operator==(const Size<T> &other) const {
    return abs(width - other.width) <= epsilon && abs(height - other.height) <= epsilon;
  }
};

#endif // _UTILITY_H_
