//
// Created by yongfali on 2016/2/23.
//

#ifndef E_GLPROGRAM_H
#define E_GLPROGRAM_H
#include "GPUImageCommon.h"

namespace e
{
    class GLProgram : public RefObject
    {
    public:
		GLProgram();
        GLProgram(const char* vShaderString, const char* fShaderString);
        virtual ~GLProgram(void);

		static bool Create(const char* vShaderString
			, const char* fShaderString
			, Ptr<GLProgram> & object);
    public:
        bool Link(void);
        void Use(void);
        void Validate(void);

        bool IsValid(void) const;
        //OpenGL 部分设备上add attribute要在link之前处理
        void AddAttribute(const char* attrName);

        GLuint GetProgram(void) const;
        GLuint GetAttributeIndex(const char* attrName);
        GLint  GetUniformLocation(const char* attrName);

        enum{LOG_TYPE_PROG, LOG_TYPE_VERT, LOG_TYPE_FRAG};
        std::string GetShaderLog(int type) const;

		void SetUniform1i(const char* attrName, GLint value);
        void SetUniform1f(const char* attrName, GLfloat value);

        void SetUniform2i(const char* attrName, GLint x, GLint y);
        void SetUniform2f(const char* attrName, GLfloat x, GLfloat y);

        void SetUniform3i(const char* attrName, GLint x, GLint y, GLint z);
        void SetUniform3f(const char* attrName, GLfloat x, GLfloat y, GLfloat z);

        void SetUniform4i(const char* attrName, GLint x, GLint y, GLint z, GLint w);
        void SetUniform4f(const char* attrName, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

        void SetUniform1iv(const char* attrName, GLsizei count, const GLint* value);
        void SetUniform2iv(const char* attrName, GLsizei count, const GLint* value);
        void SetUniform3iv(const char* attrName, GLsizei count, const GLint* value);
        void SetUniform4iv(const char* attrName, GLsizei count, const GLint* value);

		void SetUniform1fv(const char* attrName, GLsizei count, const GLfloat* value);
        void SetUniform2fv(const char* attrName, GLsizei count, const GLfloat* value);
        void SetUniform3fv(const char* attrName, GLsizei count, const GLfloat* value);
        void SetUniform4fv(const char* attrName, GLsizei count, const GLfloat* value);

		void SetUniformMatrix2fv(const char* attrName, GLsizei count, const GLfloat* value);
		void SetUniformMatrix3fv(const char* attrName, GLsizei count, const GLfloat* value);
        void SetUniformMatrix4fv(const char* attrName, GLsizei count, const GLfloat* value);
    private:
        bool Initialize(const char* vertexShader, const char* fragmentShader);
        bool CompileShader(GLuint* shader, GLenum type, const char* shaderString);
    private:
        bool isValid;
		EGLContext eglContext;
		GLuint hProgram;
        GLuint vShader;
        GLuint fShader;
        
        std::string logs[3];
        std::map<std::string, GLuint> attributes;      
    };

    typedef Ptr<GLProgram> GLProgramPtr;
}
#endif
