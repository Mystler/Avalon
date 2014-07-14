#ifndef _ShaderProgram_h_
#define _ShaderProgram_h_

#include <GL/glew.h>
#include <map>

class ShaderProgram {
public:
    ShaderProgram();
    virtual ~ShaderProgram();

    /** Load and compile a shader from the specified file location.
     *  Shader types can be GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, ...
     *  Returns true if compilation was successfull.
     */
    bool load(GLenum shaderType, const char* file);

    /** Link all our loaded shaders into a shader program.
     *  Returns true if everything went well.
     */
    bool link();

    /** Use this program in the active render state. */
    void use() const;

    /** Find the location of the specified attribute. */
    GLint attribLocation(const char* attribute) const { return glGetAttribLocation(m_programId, attribute); }

    /** Find the location of the specified uniform. */
    GLint uniformLocation(const char* uniform) const { return glGetUniformLocation(m_programId, uniform); }

    /** In case load() or link() failed, this returns the error message. */
    const char* errorMsg() const { return m_error; }

protected:
    GLuint m_programId;
    std::map<GLenum, GLuint> m_shaders;
    char* m_error;
};

#endif
