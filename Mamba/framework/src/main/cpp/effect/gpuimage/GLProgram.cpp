//
// Created by yongfali on 2016/2/23.
//

#include "include/GLProgram.h"
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <string>
#include "include/GPUImageCommon.h"
using namespace std;

namespace e
{
	GLProgram::GLProgram() 
        : isValid(false)
        , eglContext(0)
        , hProgram(0)
        , vShader(0)
        , fShader(0)
	{

	}

    GLProgram::GLProgram(const char* vShaderString, const char* fShaderString)
        : isValid(false)
        , eglContext(0)
        , hProgram(0)
        , vShader(0)
        , fShader(0)
    {
        Initialize(vShaderString, fShaderString);
    }

    GLProgram::~GLProgram(void)
    {
		if (eglContext != eglGetCurrentContext())
		{
			LOGD("GLProgram current context not equal local!");
			return;
		}

        if (vShader)
		{
			glDeleteShader(vShader);
            vShader = 0;
		}

        if (fShader)
		{
			glDeleteShader(fShader);
            fShader = 0;
		}

        if (hProgram)
		{
			glDeleteProgram(hProgram);
            hProgram = 0;
		}
    }

	bool GLProgram::Create(const char *vShaderString, const char *fShaderString, Ptr<GLProgram> & object)
	{
		object = new GLProgram();
        if (!object) return false;

		if (!object->Initialize(vShaderString, fShaderString))
		{
			object = 0;
			return false;
		}

		return true;
	}

    bool GLProgram::Initialize(const char* vShaderString, const char* fShaderString)
    {
        isValid = false;
		eglContext = eglGetCurrentContext();
        hProgram = glCreateProgram();

        if (!CompileShader(&vShader, GL_VERTEX_SHADER, vShaderString))
        {
            return false;
        }

        if (!CompileShader(&fShader, GL_FRAGMENT_SHADER, fShaderString))
        {
            return false;
        }

        glAttachShader(hProgram, vShader);
        glAttachShader(hProgram, fShader);
        return true;
    }

    bool GLProgram::CompileShader(GLuint* shader, GLenum type, const char* shaderString)
    {
        *shader = glCreateShader(type);
        glShaderSource(*shader, 1, &shaderString, NULL);
        glCompileShader(*shader);

        GLint status = -1;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE)
        {
            GLint length = 0;
            glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);

            if (length > 0)
            {
                GLchar* log = (GLchar*)malloc(length + 1);
                memset(log, 0, length + 1);
                glGetShaderInfoLog(*shader, length, &length, log);
                if (type == GL_VERTEX_SHADER){
                    logs[LOG_TYPE_VERT] = log;
                }else{
                    logs[LOG_TYPE_FRAG] = log;
                }
                free(log);
            }
        }

        return (status == GL_TRUE);
    }

    bool GLProgram::Link(void)
    {
        GLint status = 0;
        glLinkProgram(hProgram);
        glGetProgramiv(hProgram, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            return false;
        }

        if (vShader) {
            glDeleteShader(vShader);
            vShader = 0;
        }

        if (fShader) {
            glDeleteShader(fShader);
            fShader = 0;
        }

        isValid = true;
        return true;
    }

    void GLProgram::Use(void)
    {
        glUseProgram(hProgram);
    }

    void GLProgram::Validate(void)
    {
        GLint length;
        glValidateProgram(hProgram);
        glGetProgramiv(hProgram, GL_INFO_LOG_LENGTH, &length);
        if (length > 0){
            GLchar* log = (GLchar*)malloc(length + 1);
            memset(log, 0, length + 1);
            glGetProgramInfoLog(hProgram, length, &length, log);
            logs[LOG_TYPE_PROG] = log;
            free(log);
        }
    }

    GLuint GLProgram::GetProgram(void) const
    {
        return hProgram;
    }

    void GLProgram::AddAttribute(const char* attrName)
    {
        std::map<string, GLuint>::iterator it = attributes.find(string(attrName));
        if (it == attributes.end())
        {
            GLuint index = attributes.size();
            attributes.insert(pair<string, GLuint>(attrName, index));
            glBindAttribLocation(hProgram, index, attrName);
        }
    }

    GLuint GLProgram::GetAttributeIndex(const char* attrName)
    {
        std::map<string, GLuint>::iterator it = attributes.find(string(attrName));
        if (it != attributes.end()) {
            return it->second;
        } else {
            return (GLuint)(-1);
        }
    }

    bool GLProgram::IsValid(void) const
    {
        return isValid;
    }

    std::string GLProgram::GetShaderLog(int type) const
    {
        return logs[type % 3];
    }

    GLint GLProgram::GetUniformLocation(const char* attrName)
    {
        return glGetUniformLocation(hProgram, attrName);
    }

    void GLProgram::SetUniform1i(const char* attrName, GLint value)
    {   
        glUniform1i(GetUniformLocation(attrName), value);
    }

    void GLProgram::SetUniform1f(const char* attrName, GLfloat value)
    { 
       glUniform1f(GetUniformLocation(attrName), value);
    }
    void GLProgram::SetUniform2i(const char* attrName, GLint x, GLint y)
    {
        glUniform2i(GetUniformLocation(attrName), x, y);
    }
    void GLProgram::SetUniform2f(const char* attrName, GLfloat x, GLfloat y)
    {
        glUniform2f(GetUniformLocation(attrName), x, y);
    }
    void GLProgram::SetUniform3i(const char* attrName, GLint x, GLint y, GLint z)
    {
        glUniform3i(GetUniformLocation(attrName), x, y, z);
    }
    void GLProgram::SetUniform3f(const char* attrName, GLfloat x, GLfloat y, GLfloat z)
    {
        glUniform3f(GetUniformLocation(attrName), x, y, z);
    }
    void GLProgram::SetUniform4i(const char* attrName, GLint x, GLint y, GLint z, GLint w)
    {
        glUniform4i(GetUniformLocation(attrName), x, y, z, w);
    }
    void GLProgram::SetUniform4f(const char* attrName, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    {
        glUniform4f(GetUniformLocation(attrName), x, y, z, w);
    }
    void GLProgram::SetUniform1iv(const char* attrName, GLsizei count, const GLint* value)
    {
        glUniform1iv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform2iv(const char* attrName, GLsizei count, const GLint* value)
    {
        glUniform2iv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform3iv(const char* attrName, GLsizei count, const GLint* value)
    {
        glUniform3iv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform4iv(const char* attrName, GLsizei count, const GLint* value)
    {
        glUniform4iv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform1fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniform1fv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform2fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniform2fv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform3fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniform3fv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniform4fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniform4fv(GetUniformLocation(attrName), count, value);
    }
    void GLProgram::SetUniformMatrix2fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniformMatrix2fv(GetUniformLocation(attrName), count, GL_FALSE, value);
    }
    void GLProgram::SetUniformMatrix3fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniformMatrix3fv(GetUniformLocation(attrName), count, GL_FALSE, value);
    }
    void GLProgram::SetUniformMatrix4fv(const char* attrName, GLsizei count, const GLfloat* value)
    {
        glUniformMatrix4fv(GetUniformLocation(attrName), count, GL_FALSE, value);
    }
}