#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <string>
#include <cassert>

#define logGLErrors() __logGLErrors__(__FILE__, __LINE__)

void initEGLFunctions();
void setCurrentEGLContext();
void assertCurrentEGLContext();

class Utility {
private:
  Utility() {}
public:
  static void __logGLErrors__(const char *filename, int line);
  static std::string getGLErrorString(int err);
};

template<typename T> struct Size;
template<typename T> struct Position;

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

  Position<T> operator+(Position<T> const& rhs) const { return { x + rhs.x, y + rhs.y }; }
  Position<T>& operator+=(Position<T> const& rhs) { x += rhs.x; y += rhs.y; return *this; }
  Position<T> operator+(T const& rhs) const { return { x + rhs, y + rhs }; }
  Position<T>& operator+=(T const& rhs) { x += rhs; y += rhs; return *this; }
  Position<T> operator-(Position<T> const& rhs) const { return { x - rhs.x, y - rhs.y }; }
  Position<T>& operator-=(Position<T> const& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
  Position<T> operator-(T const& rhs) const { return { x - rhs, y - rhs }; }
  Position<T>& operator-=(T const& rhs) { x -= rhs; y -= rhs; return *this; }
  Position<T> operator*(Position<T> const& rhs) const { return { x * rhs.x, y * rhs.y }; }
  Position<T>& operator*=(Position<T> const& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
  Position<T> operator*(T const& rhs) const { return { x * rhs, y * rhs }; }
  Position<T>& operator*=(T const& rhs) { x *= rhs; y *= rhs; return *this; }
  Position<T> operator/(Position<T> const& rhs) const { return { x / rhs.x, y / rhs.y }; }
  Position<T>& operator/=(Position<T> const& rhs) { x /= rhs.x; y /= rhs.y; return *this; }
  Position<T> operator/(T const& rhs) const { return { x / rhs, y / rhs }; }
  Position<T>& operator/=(T const& rhs) { x /= rhs; y /= rhs; return *this; }

  Position<T> operator+(Size<T> const& rhs) const { return { x + rhs.width, y + rhs.height }; }
  Position<T>& operator+=(Size<T> const& rhs) { x += rhs.width; y += rhs.height; return *this; }
  Position<T> operator-(Size<T> const& rhs) const { return { x - rhs.width, y - rhs.height }; }
  Position<T>& operator-=(Size<T> const& rhs) { x -= rhs.width; y -= rhs.height; return *this; }
  Position<T> operator*(Size<T> const& rhs) const { return { x * rhs.width, y * rhs.height }; }
  Position<T>& operator*=(Size<T> const& rhs) { x *= rhs.width; y *= rhs.height; return *this; }
  Position<T> operator/(Size<T> const& rhs) const { return { x / rhs.width, y / rhs.height }; }
  Position<T>& operator/=(Size<T> const& rhs) { x /= rhs.width; y /= rhs.height; return *this; }
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

  template<typename Integer, std::enable_if_t<std::is_integral<Integer>::value, int> = 0>
  bool operator==(const Size<Integer> &other) const {
    return width == other.width && height == other.height;
  }

  template<typename Floating, std::enable_if_t<std::is_floating_point<Floating>::value, int> = 0>
  bool operator==(const Size<Floating> &other) const {
    return abs(width - other.width) <= epsilon && abs(height - other.height) <= epsilon;
  }

  Size<T> operator+(Size<T> const& rhs) const { return { width + rhs.width, height + rhs.height }; }
  Size<T>& operator+=(Size<T> const& rhs) { width += rhs.width; height += rhs.height; return *this; }
  Size<T> operator+(T const& rhs) const { return { width + rhs, height + rhs }; }
  Size<T>& operator+=(T const& rhs) { width += rhs; height += rhs; return *this; }
  Size<T> operator-(Size<T> const& rhs) const { return { width - rhs.width, height - rhs.height }; }
  Size<T>& operator-=(Size<T> const& rhs) { width -= rhs.width; height -= rhs.height; return *this; }
  Size<T> operator-(T const& rhs) const { return { width - rhs, height - rhs }; }
  Size<T>& operator-=(T const& rhs) { width -= rhs; height -= rhs; return *this; }
  Size<T> operator*(Size<T> const& rhs) const { return { width * rhs.width, height * rhs.height }; }
  Size<T>& operator*=(Size<T> const& rhs) { width *= rhs.width; height *= rhs.height; return *this; }
  Size<T> operator*(T const& rhs) const { return { width * rhs, height * rhs }; }
  Size<T>& operator*=(T const& rhs) { width *= rhs; height *= rhs; return *this; }
  Size<T> operator/(Size<T> const& rhs) const { return { width / rhs.width, height / rhs.height }; }
  Size<T>& operator/=(Size<T> const& rhs) { width /= rhs.width; height /= rhs.height; return *this; }
  Size<T> operator/(T const& rhs) const { return { width / rhs, height / rhs }; }
  Size<T>& operator/=(T const& rhs) { width /= rhs; height /= rhs; return *this; }

  Size<T> operator+(Position<T> const& rhs) const { return { width + rhs.x, height + rhs.y }; }
  Size<T>& operator+=(Position<T> const& rhs) { width += rhs.x; height += rhs.y; return *this; }
  Size<T> operator-(Position<T> const& rhs) const { return { width - rhs.x, height - rhs.y }; }
  Size<T>& operator-=(Position<T> const& rhs) { width -= rhs.x; height -= rhs.y; return *this; }
  Size<T> operator*(Position<T> const& rhs) const { return { width * rhs.x, height * rhs.y }; }
  Size<T>& operator*=(Position<T> const& rhs) { width *= rhs.x; height *= rhs.y; return *this; }
  Size<T> operator/(Position<T> const& rhs) const { return { width / rhs.x, height / rhs.y }; }
  Size<T>& operator/=(Position<T> const& rhs) { width /= rhs.x; height /= rhs.y; return *this; }
};

#endif // _UTILITY_H_
