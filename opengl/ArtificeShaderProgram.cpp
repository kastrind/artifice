/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redestributed without written permission.*/
//Version: 002

#include "ArtificeShaderProgram.h"

bool ArtificeShaderProgram::loadProgram()
{
	//generate program
	mProgramID = glCreateProgram();

	//load vertex shader
	GLuint vertexShader = loadShaderFromFile( "shaders/ArtificeShader.glvs", GL_VERTEX_SHADER );

	//check for errors
	if( vertexShader == 0 )
	{
	glDeleteProgram( mProgramID );
	mProgramID = 0;
	return false;
	}

	//attach vertex shader to program
	glAttachShader( mProgramID, vertexShader );


	//create fragment shader
	GLuint fragmentShader = loadShaderFromFile( "shaders/ArtificeShader.glfs", GL_FRAGMENT_SHADER );

	//check for errors
	if( fragmentShader == 0 )
	{
		glDeleteShader( vertexShader );
		glDeleteProgram( mProgramID );
		mProgramID = 0;
		return false;
	}

	//attach fragment shader to program
	glAttachShader( mProgramID, fragmentShader );

	//link program
	glLinkProgram( mProgramID );

	//check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv( mProgramID, GL_LINK_STATUS, &programSuccess );
	if( programSuccess != GL_TRUE )
	{
		printf( "Error linking program %d!\n", mProgramID );
		printProgramLog( mProgramID );
		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );
		glDeleteProgram( mProgramID );
		mProgramID = 0;
		return false;
	}

	//clean up excess shader references
	glDeleteShader( vertexShader );
	glDeleteShader( fragmentShader );
	
	return true;
}
