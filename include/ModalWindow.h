#ifndef _MODALWINDOW_H_
#define _MODALWINDOW_H_

#include <string>
#include <utility>
#include <vector>

#include "GLES.h"
#include "Utility.h"

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
    Position<int> position;
    Size<int> size;
    std::vector<float> color;
  };

  class WindowParams {
  public:
    Position<int> position;
    Size<int> size;
    Size<int> margin;
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

  Position<int> position;
  Size<int> size;
  Params params;

  void initialize();
  void renderContent();
  void renderRectangle(Position<int> position, Size<int> size);
  void renderTitle();
  void renderBody();
  void renderButton();
  void calculateElementsPositions();
  void calculateParams();

public:
  ModalWindow();
  ~ModalWindow();
  void show(std::string title, std::string body, std::string button);
  void hide();
  void render();
  bool isVisible();
};

#endif // _MODALWINDOW_H_

