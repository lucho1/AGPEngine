//
// Engine.h: This file contains the types and functions relative to the engine.
//
#pragma once

#include "Globals.h"
#include <glad/glad.h>

struct App
{
    // Loop
    bool isRunning;

    //std::vector<Texture>  textures;
    //std::vector<Program>  programs;

    // program indices
    //uint texturedGeometryProgramIdx;


    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    //GLuint embeddedVertices;
    //GLuint embeddedElements;
    //
    //// Location of the texture uniform in the textured quad shader
    //GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    //GLuint vao;
};