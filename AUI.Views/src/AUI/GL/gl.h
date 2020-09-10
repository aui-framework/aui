#pragma once

#ifdef ANDROID
    #include <GLES3/gl3.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif