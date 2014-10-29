#ifndef __GLProgram_h
#define __GLProgram_h

//[]------------------------------------------------------------------------[]
//|                                                                          |
//|                          GVSG Graphics Library                           |
//|                               Version 1.0                                |
//|                                                                          |
//|              Copyright® 2012-2014, Paulo Aristarco Pagliosa              |
//|              All Rights Reserved.                                        |
//|                                                                          |
//[]------------------------------------------------------------------------[]
//
// OVERVIEW: GLProgram.h
// ========
// Class definitions for GLSL shader and program.

#include "GL/glew.h"
#include <GL/gl.h>
#include <stdio.h>
#include <string>
#include "Graphics/Color.h"
#include "DsMath"

using namespace std;
using namespace Ds;

namespace GLSL
{ // begin namespace GLSL

//
// Init GLSL
//
extern void init();

enum ShaderSource
{
  FILE,
  STRING
};


//////////////////////////////////////////////////////////
//
// Program: GLSL program class
// =======
class Program
{
public:
  enum State
  {
    CREATED,
    MODIFIED,
    BUILT,
    IN_USE
  };

  // Constructor
  Program(const char*);

  // Destructor
  ~Program();

  // Cast to handle type
  operator GLuint() const
  {
    return handle;
  }

  // Get program name
  const char* getName() const
  {
    return name.c_str();
  }

  // Get program state
  GLuint getState() const
  {
    return state;
  }

  // Add shader
  void addShader(GLenum, ShaderSource, const char*);

  // Use/disuse program
  void use();
  void disuse();

  // Get uniform variable location
  GLint getUniformLocation(const char*) const;

  // Set uniform variable
  static void setUniform(GLint, float);
  static void setUniform(GLint, float, float);
  static void setUniform(GLint, float, float, float);
  static void setUniform(GLint, float, float, float, float);
  static void setUniform(GLint, const vec3f&);
  static void setUniform(GLint, const vec4f&);
  static void setUniform(GLint, const mat4f&);
  static void setUniform(GLint, const Color&);

  void setUniform(const char*, float);
  void setUniform(const char*, float, float);
  void setUniform(const char*, float, float, float);
  void setUniform(const char*, float, float, float, float);
  void setUniform(const char*, const vec3f&);
  void setUniform(const char*, const vec4f&);
  void setUniform(const char*, const mat4f&);
  void setUniform(const char*, const Color&);

  // Get atributte location
  GLint getAttributeLocation(const char*) const;

  // Get the current program
  static Program* getCurrent()
  {
    return current;
  }

  // Set the current program
  static void setCurrent(Program* c)
  {
    if (c != current)
    {
      if (c == 0)
        current->disuse();
      else
        c->use();
    }
  }

protected:
  // Link program
  void link();

private:
  static Program* current;

  GLuint handle;
  string name;
  GLuint state;

  // Check if program is in use
  void checkInUse() const;

}; // Program


//////////////////////////////////////////////////////////
//
// Program inline implementtaion
// =======
inline void
Program::setUniform(GLint loc, float f0)
{
  glUniform1f(loc, f0);
}

inline void
Program::setUniform(GLint loc, float f0, float f1)
{
  glUniform2f(loc, f0, f1);
}

inline void
Program::setUniform(GLint loc, float f0, float f1, float f2)
{
  glUniform3f(loc, f0, f1, f2);
}

inline void
Program::setUniform(GLint loc, float f0, float f1, float f2, float f3)
{
  glUniform4f(loc, f0, f1, f2, f3);
}

inline void
Program::setUniform(GLint loc, const vec3f& v)
{
  glUniform3fv(loc, 1, &v[0]);
}

inline void
Program::setUniform(GLint loc, const vec4f& v)
{
  glUniform4fv(loc, 1, &v[0]);
}

inline void
Program::setUniform(GLint loc, const mat4f& v)
{
  glUniformMatrix4fv(loc, 1, GL_FALSE, v.data());
}

inline void
Program::setUniform(GLint loc, const Color& c)
{
  glUniform4fv(loc, 1, &c[0]);
}

inline void
Program::setUniform(const char* name, float f0)
{
  setUniform(getUniformLocation(name), f0);
}

inline void
Program::setUniform(const char* name, float f0, float f1)
{
  setUniform(getUniformLocation(name), f0, f1);
}

inline void
Program::setUniform(const char* name, float f0, float f1, float f2)
{
  setUniform(getUniformLocation(name), f0, f1, f2);
}

inline void
Program::setUniform(const char* name, float f0, float f1, float f2, float f3)
{
  setUniform(getUniformLocation(name), f0, f1, f2, f3);
}

inline void
Program::setUniform(const char* name, const vec3f& v)
{
  setUniform(getUniformLocation(name), v);
}

inline void
Program::setUniform(const char* name, const vec4f& v)
{
  setUniform(getUniformLocation(name), v);
}

inline void
Program::setUniform(const char* name, const mat4f& v)
{
  setUniform(getUniformLocation(name), v);
}

inline void
Program::setUniform(const char* name, const Color& c)
{
  setUniform(getUniformLocation(name), c);
}

} // end namespace GLSL

#endif // __GLSL_h
