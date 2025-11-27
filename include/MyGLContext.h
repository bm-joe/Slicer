#pragma once

#include "wx/glcanvas.h"
class MyGLContext : public wxGLContext {
    public:
        MyGLContext(wxGLCanvas *canvas) : wxGLContext(canvas)
        {
            SetCurrent(*canvas);
            // set up the parameters we want to use
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_TEXTURE_2D);
            // glEnable(GL_COLOR_MATERIAL);
            // add slightly more light, the default lighting is rather dark
            GLfloat ambient[] = {0.5, 0.5, 0.5, 0.5};
            glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

            glEnable(GL_NORMALIZE);

            // Smooth shading
            glShadeModel(GL_SMOOTH);

            // set viewing projection
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-0.5, 0.5, -0.5, 0.5, 1, 3);
        };

    private:

};