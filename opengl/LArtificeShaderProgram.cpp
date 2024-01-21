/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redestributed without written permission.*/
//Version: 002

#include "LArtificeShaderProgram.h"

bool LArtificeShaderProgram::loadProgram()
{
	//Generate program
	mProgramID = glCreateProgram();

	//Load vertex shader
	GLuint vertexShader = loadShaderFromFile( "shaders/LArtificeShader.glvs", GL_VERTEX_SHADER );

    //Check for errors
    if( vertexShader == 0 )
    {
        glDeleteProgram( mProgramID );
        mProgramID = 0;
        return false;
    }

	//Attach vertex shader to program
	glAttachShader( mProgramID, vertexShader );


	//Create fragment shader
	GLuint fragmentShader = loadShaderFromFile( "shaders/LArtificeShader.glfs", GL_FRAGMENT_SHADER );

    //Check for errors
    if( fragmentShader == 0 )
    {
        glDeleteShader( vertexShader );
        glDeleteProgram( mProgramID );
        mProgramID = 0;
        return false;
    }

	//Attach fragment shader to program
	glAttachShader( mProgramID, fragmentShader );

	//Link program
    glLinkProgram( mProgramID );

	//Check for errors
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

	//Clean up excess shader references
    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );
	
	return true;
}
