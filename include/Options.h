#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <vector>
#include <string>
#include <map>
#include <functional>

#include "GLES.h"
#include "Utility.h"

class Options {
  private:
    Size<int> optionRectangleSize;
    Size<int> suboptionRectangleSize;
    Position<int> position;
    Size<int> margin;
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
    void render(Position<int> position, Size<int> optionRectangleSize, Size<int> suboptionRectangleSize, float opacity);
    void renderRectangle(Position<int> position, Size<int> size, std::vector<float> color, float opacity, std::string name, int frameWidth, std::vector<float> frameColor, bool submenuSelected = false);

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
