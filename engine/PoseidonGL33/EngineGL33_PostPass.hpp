#pragma once

// Small GL helpers the post stages share (EngineGL33_PostFX.cpp and the stage
// files EngineGL33_Post<Name>.cpp). Every fullscreen stage binds its inputs to
// kPostUnits (EngineGL33_Lighting.hpp), draws the fullscreen triangle of the
// post vertex shader into one target and sets its uniforms by name.

#include <glad/gl.h>

namespace Poseidon::gl33post
{

inline void BindTex(int unit, GLuint tex)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
}

inline void DrawTo(GLuint fbo, int vw, int vh)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, vw, vh);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

inline void SetInt(GLuint prog, const char* name, int v)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform1i(loc, v);
}

inline void SetFloat(GLuint prog, const char* name, float v)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform1f(loc, v);
}

inline void Set2(GLuint prog, const char* name, float a, float b)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform2f(loc, a, b);
}

inline void Set4(GLuint prog, const char* name, float a, float b, float c, float d)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform4f(loc, a, b, c, d);
}

} // namespace Poseidon::gl33post
