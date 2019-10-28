#ifndef _MODALWINDOW_H_
#define _MODALWINDOW_H_

#include <string>
#include <utility>
#include <vector>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

class ModalWindow {
private:
  GLuint programObject;
  GLuint posALoc;
  GLuint posLoc;
  GLuint sizLoc;
  GLuint colLoc;
  GLuint opaLoc;

  bool visible;
  std::string title;
  std::string body;
  std::string button;

  class TextParams {
  public:
    int fontId;
    int fontSize;
    std::string text;
    std::pair<int, int> position;
    std::pair<int, int> size;
    std::vector<float> color;
  };

  class WindowParams {
  public:
    std::pair<int, int> position;
    std::pair<int, int> size;
    std::pair<int, int> margin;
  };

  class Params {
  public:
    TextParams title;
    TextParams body;
    int lineWidth;

    WindowParams window;

    WindowParams buttonWindow;
    TextParams buttonText;
  };

  std::pair<int, int> position;
  std::pair<int, int> size;
  Params params;

  void initialize();
  void renderContent();
  void renderRectangle(std::pair<int, int> position, std::pair<int, int> size);
  void renderTitle();
  void renderBody();
  void renderButton();
  void calculateElementsPositions();
  std::pair<int, int> getTextSize(std::string s, int lineWidth, int fontHeight, int fontId);
  void calculateParams();

public:
  ModalWindow();
  ~ModalWindow();
  void show(std::string title, std::string body, std::string button);
  void hide();
  void render(int fontId);
  bool isVisible();
};

#endif // _MODALWINDOW_H_

