#include "ShaderProgram.h"
#include <cstdio>
#include <cstring>

#define MAX_ERROR_LENGTH 512

ShaderProgram::ShaderProgram()
{
    m_programId = glCreateProgram();
    m_error = new char[MAX_ERROR_LENGTH];
    memset(m_error, 0, MAX_ERROR_LENGTH);
}

ShaderProgram::~ShaderProgram()
{
    delete[] m_error;
    m_shaders.clear();
    glDeleteProgram(m_programId);
}

bool ShaderProgram::load(GLenum shaderType, const char* file)
{
    // Do not allow shader duplicates
    if (m_shaders.find(shaderType) != m_shaders.end()) {
        sprintf(m_error, "Shader of type %i already loaded.", shaderType);
        return false;
    }

    // Load source from file, NOTE: CRLF will go kablooey, require LF for now
    FILE* f = fopen(file, "r");
    if (!f) {
        sprintf(m_error, "Could not open shader file %s.", file);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char* source = new char[size + 1];
    fread(source, sizeof(char), size, f);
    source[size] = 0;
    fclose(f);

    // Compile source
    GLuint id = glCreateShader(shaderType);
    glShaderSource(id, 1, (const GLchar**)&source, nullptr);
    glCompileShader(id);
    delete[] source;

    // Check for compiler errors
    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(id, MAX_ERROR_LENGTH, nullptr, m_error);
        glDeleteShader(id);
        return false;
    }
    m_shaders[shaderType] = id;
    return true;
}

bool ShaderProgram::link()
{
    for (const auto& it : m_shaders)
        glAttachShader(m_programId, it.second);

    glLinkProgram(m_programId);
    GLint status;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        glGetProgramInfoLog(m_programId, MAX_ERROR_LENGTH, nullptr, m_error);
        return false;
    }

    // Free redundant resources and flag shaders for deletion (will happen when we delete our program)
    for (const auto& it : m_shaders) {
        glDetachShader(m_programId, it.second);
        glDeleteShader(it.second);
    }
    return true;
}

void ShaderProgram::use() const
{
    glUseProgram(m_programId);
}
