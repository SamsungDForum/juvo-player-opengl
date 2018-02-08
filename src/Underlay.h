#ifndef _UNDERLAY_H_
#define _UNDERLAY_H_

#include <chrono>
#include <cmath>

#ifndef _INCLUDE_GLES_
#define _INCLUDE_GLES_
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif // _INCLUDE_GLES_

class Underlay {
private:
  GLuint programObject;
  std::chrono::time_point<std::chrono::high_resolution_clock> time;
  bool initialize();

public:
  Underlay();
  ~Underlay();
  void render();
};

Underlay::Underlay()
{
  initialize();
}

Underlay::~Underlay() {
}

bool Underlay::initialize() {
  const GLchar* vShaderTexStr =  
    "attribute vec4 a_position;     \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "}                              \n";
 
  const GLchar* fShaderTexStr =  
    "precision highp float;       \n"
    "uniform float u_time;          \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   vec2 res = vec2(1920.0, 1080.0);  \n"
    "   vec2 uv = gl_FragCoord.xy / res; \n"
    "   vec3 col = 0.5 + 0.5 * cos(vec3(u_time) + uv.xyx + vec3(0,2,4)); \n"
    "   gl_FragColor = vec4(col, u_opacity);   \n"
    "}                              \n";

  const GLchar* fShaderTexStr1 =  
    "precision highp float;       \n"
    "uniform float u_time;          \n"
    "uniform float u_opacity;       \n"
    "void main()                    \n"
    "{                              \n"
    "   vec3 col = vec3(0.0);       \n"
    "   int AA = 2;                 \n"
    "   vec2 res = vec2(1920.0, 1080.0); \n"
    "   for(int m = 0; m < AA; m++)   \n"
    "     for(int n = 0; n < AA; n++) \n"
    "     {                         \n"
    "       vec2 p = -1.0 + 2.0 * (gl_FragCoord.xy + vec2(float(m), float(n)) / float(AA)) / res.xy; \n"
    "       p.x *= res.x / res.y;   \n"
    "       float zoo = 1.0 / 250.0;\n"
    "       zoo = 1.0 / (400.0 - 150.0 * sin(0.15 * u_time - 0.3)); \n"
    "       vec2 cc = vec2(-0.533516, 0.526141) + p * zoo; \n"
    "       vec2 t2c = vec2(-0.5, 2.0); \n"
    "       t2c += 0.5 * vec2(cos(0.13 * (u_time - 10.0)), sin(0.13 * (u_time - 10.0))); \n"
    "                               \n"
    "       vec2 z  = vec2(0.0);    \n"
    "       vec2 dz = vec2(0.0);    \n"
    "       float trap1 = 0.0;      \n"
    "       float trap2 = 1e20;     \n" // ???
    "       float co2 = 0.0;        \n"
    "       for(int i = 0; i < 150; i++) \n"
    "       {                       \n"
    "         if(dot(z, z) > 1024.0)\n"
    "           break;              \n"
    "         dz = 2.0 * vec2(z.x * dz.x - z.y * dz.y, z.x * dz.y + z.y * dz.x) + vec2(1.0, 0.0); \n"
    "         z = cc + vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y); \n"
    "         float d1 = abs(dot(z - vec2(0.0, 1.0), vec2(0.707)));  \n"
    "         float ff = step(d1, 1.0);                              \n"
    "         co2 += ff;            \n"
    "         trap1 += ff * d1;     \n"
    "         trap2 = min(trap2, dot(z - t2c, z - t2c)); \n"
    "       }                       \n"
    "       float d = sqrt(dot(z, z) / dot(dz, dz)) * log(dot(z, z)); \n"
    "       float c1 = pow(clamp(2.00 * d / zoo, 0.0, 1.0), 0.5);     \n"
    "       float c2 = pow(clamp(1.5 * trap1 / co2, 0.0, 1.0), 2.0);  \n"
    "       float c3 = pow(clamp(0.4 * trap2, 0.0, 1.0), 0.25);       \n"
    "       vec3 col1 = 0.5 + 0.5 * sin(3.0 + 4.0 * c2 + vec3(0.0, 0.5, 1.0)); \n"
    "       vec3 col2 = 0.5 + 0.5 * sin(4.1 + 2.0 * c3 + vec3(1.0, 0.5, 0.0)); \n"
    "       col += 2.0 * sqrt(c1 * col1 * col2);                      \n"
    "     }                        \n"
    "     col /= float(AA * AA);   \n"
    "   gl_FragColor = vec4(col, 1.0); \n"
    "}                              \n";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vShaderTexStr, NULL);
  glCompileShader(vertexShader);

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fShaderTexStr1, NULL);
  glCompileShader(fragmentShader);

  programObject = glCreateProgram();
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);
  glLinkProgram(programObject);

  //glBindAttribLocation(programObject, 0, "a_position");
  //glViewport(0, 0, viewport.first, viewport.second);

  time = std::chrono::high_resolution_clock::now();

  return true;
}

void Underlay::render() {
  std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float, std::milli> timespan = now - time;

  float down = -1.0f;
  float top = 1.0f;
  float left = -1.0f;
  float right = 1.0f;
  GLfloat vVertices[] = { left,   top,  0.0f,
                          left,   down, 0.0f,
                          right,  down, 0.0f,
                          right,  top,  0.0f
  };
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 }; // TODO: Move it to VBO... Or somewhene in GPU's memory.

  glUseProgram(programObject);

  //int factor = 16;

  //glViewport(0, 0, 1920 / factor, 1080 / factor);

  GLint posLoc = glGetAttribLocation(programObject, "a_position");
  glEnableVertexAttribArray(posLoc);
  glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, vVertices);

  GLfloat t = static_cast<float>(timespan.count()) / 1000.0f;
  GLuint timeLoc = glGetUniformLocation(programObject, "u_time"); // TODO: Store the location somewhere.
  glUniform1f(timeLoc, t);

  GLuint opacityLoc = glGetUniformLocation(programObject, "u_opacity"); // TODO: Store the location somewhere.
  glUniform1f(opacityLoc, 1.0f);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  //glViewport(0, 0, 1920, 1080);

  glUseProgram(0);
}

#endif // _UNDERLAY_H_
