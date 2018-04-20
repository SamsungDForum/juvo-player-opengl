#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <vector>
#include <string>
#include <map>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

#include "Text.h"

class Options {
  private:
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
        std::map<int, Suboption> subopt;
    };

    std::map<int, Option> options;
    int activeOptionId;
    int activeSuboptionId;
    int selectedOptionId;
    int selectedSuboptionId;

    GLuint programObject = GL_INVALID_VALUE;
    GLuint posLoc = GL_INVALID_VALUE;
    GLuint colLoc = GL_INVALID_VALUE;
    GLuint opaLoc = GL_INVALID_VALUE;
    GLuint fraLoc = GL_INVALID_VALUE;

    initialize();
    checkShaderCompileError(GLuint shader);
    void renderRectangle(std::pair<int, int> position, std::pair<int, int> size, std::pair<int, int> viewport, std::vector<float> color, float opacity, std::string name, int frame, const Text &text);

  public:
    Options();

    bool addOption(int id, std::string name);
    bool addSuboption(int parentId, int id, std::string name);
    bool updateSelection(int activeOptionId, int activeSuboptionId, int selectedOptionId, int selectedSuboptionId);
    void clearOptions();
    void render(std::pair<int, int> position, std::pair<int, int> viewport, float opacity, const Text &text);
    int getMaxTextLength() { return maxTextLength; }
};

#endif // _OPTIONS_H_
