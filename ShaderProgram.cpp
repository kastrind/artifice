/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redestributed without written permission.*/
//Version: 002

#include "ShaderProgram.h"
#include <fstream>

ShaderProgram::ShaderProgram()
{
	mProgramID = NULL;
}

ShaderProgram::~ShaderProgram()
{
	//free program if it exists
	freeProgram();
}

void ShaderProgram::freeProgram()
{
	//delete program
	glDeleteProgram( mProgramID );
}

bool ShaderProgram::bind()
{
	//use shader
	glUseProgram( mProgramID );

	//check for error
	GLenum error = glGetError();
	if( error != GL_NO_ERROR )
	{
		printf( "Error binding shader with id %d! %s\n", mProgramID, glewGetErrorString( error ) );
		printProgramLog( mProgramID );
		return false;
	}

	return true;
}

void ShaderProgram::unbind()
{
	//use default program
	glUseProgram( NULL );
}

GLuint ShaderProgram::getProgramID()
{
	return mProgramID;
}

void ShaderProgram::printProgramLog( GLuint program )
{
	//make sure name is shader
	if( glIsProgram( program ) )
	{
		//program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

		//allocate string
		char* infoLog = new char[ maxLength ];

		//get info log
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//print Log
			printf( "%s\n", infoLog );
		}

		//deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a program\n", program );
	}
}

void ShaderProgram::printShaderLog( GLuint shader )
{
	//make sure name is shader
	if( glIsShader( shader ) )
	{
		//shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

		//allocate string
		char* infoLog = new char[ maxLength ];

		//get info log
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//print Log
			printf( "%s\n", infoLog );
		}

		//deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a shader\n", shader );
	}
}

GLuint ShaderProgram::loadShaderFromFile( std::string path, GLenum shaderType )
{
	//open file
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile( path.c_str() );

	//source file loaded
	if( sourceFile )
	{
		//get shader source
		shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), std::istreambuf_iterator< char >() );

		//create shader ID
		shaderID = glCreateShader( shaderType );

		//set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource( shaderID, 1, (const GLchar**)&shaderSource, NULL );

		//compile shader source
		glCompileShader( shaderID );

		//check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv( shaderID, GL_COMPILE_STATUS, &shaderCompiled );
		if( shaderCompiled != GL_TRUE )
		{
			printf( "Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource );
			printShaderLog( shaderID );
			glDeleteShader( shaderID );
			shaderID = 0;
		}
	}
	else
	{
		printf( "Unable to open file %s\n", path.c_str() );
	}

	return shaderID;
}
