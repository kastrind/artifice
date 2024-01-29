/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/
//Version: 002

#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string>

class ShaderProgram
{
public:
	ShaderProgram();
	/*
	Pre Condition:
		-None
	Post Condition:
		-Initializes variables
	Side Effects:
		-None
	*/

	virtual ~ShaderProgram();
	/*
	Pre Condition:
		-None
	Post Condition:
		-Frees shader program
	Side Effects:
		-None
	*/

	virtual bool loadProgram() = 0;
	/*
	Pre Condition:
		-A valid OpenGL context
	Post Condition:
		-Loads shader program
	Side Effects:
		-None
	*/

	virtual void freeProgram();
	/*
	Pre Condition:
		-None
	Post Condition:
		-Frees shader program if it exists
	Side Effects:
		-None
	*/

	bool bind();
	/*
	Pre Condition:
		-A loaded shader program
	Post Condition:
		-Sets this program as the current shader program
		-Reports to console if there was an error
	Side Effects:
		-None
	*/

	void unbind();
	/*
	Pre Condition:
		-None
	Post Condition:
		-Sets default shader program as current program
	Side Effects:
		-None
	*/

	GLuint getProgramID();
	/*
	Pre Condition:
		-None
	Post Condition:
		-Returns program ID
	Side Effects:
		-None
	*/

	// Utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const
	{         
		glUniform1i(glGetUniformLocation(mProgramID, name.c_str()), (int)value); 
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const
	{ 
		glUniform1i(glGetUniformLocation(mProgramID, name.c_str()), value); 
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const
	{ 
		glUniform1f(glGetUniformLocation(mProgramID, name.c_str()), value); 
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{ 
		glUniform2fv(glGetUniformLocation(mProgramID, name.c_str()), 1, &value[0]); 
	}
	void setVec2(const std::string &name, float x, float y) const
	{ 
		glUniform2f(glGetUniformLocation(mProgramID, name.c_str()), x, y); 
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{ 
		glUniform3fv(glGetUniformLocation(mProgramID, name.c_str()), 1, &value[0]); 
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{ 
		glUniform3f(glGetUniformLocation(mProgramID, name.c_str()), x, y, z); 
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{ 
		glUniform4fv(glGetUniformLocation(mProgramID, name.c_str()), 1, &value[0]); 
	}
	void setVec4(const std::string &name, float x, float y, float z, float w) const
	{ 
		glUniform4f(glGetUniformLocation(mProgramID, name.c_str()), x, y, z, w); 
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(mProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(mProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(mProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

protected:
	void printProgramLog( GLuint program );
	/*
	Pre Condition:
		-None
	Post Condition:
		-Prints program log
		-Reports error is GLuint ID is not a shader program
	Side Effects:
		-None
	*/

	void printShaderLog( GLuint shader );
	/*
	Pre Condition:
		-None
	Post Condition:
		-Prints shader log
		-Reports error is GLuint ID is not a shader
	Side Effects:
		-None
	*/

	GLuint loadShaderFromFile( std::string path, GLenum shaderType );
	/*
	Pre Condition:
		-None
	Post Condition:
		-Returns the ID of a compiled shader of the specified type from the specified file
		-Reports error to console if file could not be found or compiled
	Side Effects:
		-None
	*/

	//Program ID
	GLuint mProgramID;
};

#endif
