#ifndef _MODALWINDOW_H_
#define _MODALWINDOW_H_

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include <string>
#include <utility>

#include "Text.h"
#include "log.h"

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
    Text &text;
    TextParams title;
    TextParams body;
    int lineWidth;

    std::pair<int, int> viewport;
    WindowParams window;

    WindowParams buttonWindow;
    TextParams buttonText;
  };

  void initialize();
  void checkShaderCompileError(GLuint shader);
  void renderContent(Params &params);
  void renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport);
  void renderTitle(Params &params);
  void renderBody(Params &params);
  void renderButton(Params &params);
  void calculateElementsPositions(Params &params);
  std::pair<int, int> getTextSize(Text &text, std::string s, int lineWidth, int fontHeight, int fontId, std::pair<int, int> viewport);

public:
  ModalWindow();
  ~ModalWindow();
  void show(std::string title, std::string body, std::string button);
  void hide();
  void render(Text &text, std::pair<int, int> viewport, int fontId);
  bool isVisible();
};

#endif // _MODALWINDOW_H_

