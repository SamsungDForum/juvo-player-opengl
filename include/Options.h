#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <vector>
#include <string>
#include <map>
#include <functional>

#include "log.h"

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

class Options {
  private:
    std::pair<int, int> optionRectangleSize;
    std::pair<int, int> suboptionRectangleSize;
    std::pair<int, int> position;
    std::pair<int, int> margin;
    int frameWidth;
    std::vector<float> optionColor;
    std::vector<float> selectedOptionColor;
    std::vector<float> activeOptionColor;
    std::vector<float> suboptionColor;
    std::vector<float> selectedSuboptionColor;
    std::vector<float> activeSuboptionColor;
    std::vector<float> frameColor;

    const int maxTextLength;

    class Suboption {
      public:
        int id;
        int parentId;
        std::string name;
    };

    class Option {
      public:
        int id;
        std::string name;
        std::map<int, Suboption, std::less<int>> subopt;
    };

    std::map<int, Option> options;
    int activeOptionId;      // currently active option
    int activeSuboptionId;   // currently active suboption
    int selectedOptionId;    // option selected at the moment in the menu
    int selectedSuboptionId; // suboption selected at the moment in the menu
    float opacity;
    bool show;

    GLuint programObject = GL_INVALID_VALUE;
    GLuint positionALoc  = GL_INVALID_VALUE;
    GLuint positionLoc   = GL_INVALID_VALUE;
    GLuint sizeLoc       = GL_INVALID_VALUE;
    GLuint colorLoc      = GL_INVALID_VALUE;
    GLuint opacityLoc    = GL_INVALID_VALUE;
    GLuint frameWidthLoc = GL_INVALID_VALUE;
    GLuint frameColorLoc = GL_INVALID_VALUE;

    void initialize();
    void render(std::pair<int, int> position, std::pair<int, int> optionRectangleSize, std::pair<int, int> suboptionRectangleSize, float opacity);
    void renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::vector<float> color, float opacity, std::string name, int frameWidth, std::vector<float> frameColor, bool submenuSelected = false);

  public:
    Options();

    bool addOption(int id, std::string name);
    bool addSuboption(int parentId, int id, std::string name);
    bool updateSelection(bool show, int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
    void clearOptions();
    void render();
    void renderIcon();
    int getMaxTextLength() { return maxTextLength; }
    float getOpacity() { return opacity; }
    void setOpacity(float opacity) { this->opacity = opacity; }
};

#endif // _OPTIONS_H_
