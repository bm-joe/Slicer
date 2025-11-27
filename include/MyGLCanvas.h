#pragma once
#include "wx/glcanvas.h"
#include "MyGLContext.h"
#include "enums.h"
#include <glm/glm.hpp>                      
#include <wx/wx.h>
#include "Slicer.h"
#include <glm/gtc/type_ptr.hpp>
class MyGLCanvas : public wxGLCanvas {
    public:
    //construcotr
        MyGLCanvas(wxWindow *parent) : wxGLCanvas(
                                           parent,
                                           []
                                           {
                                               wxGLAttributes attrs;
                                               attrs.Defaults();
                                               attrs.PlatformDefaults();
                                               attrs.EndList();
                                               return attrs;
                                           }(),
                                           wxID_ANY,
                                           wxDefaultPosition,
                                           wxDefaultSize,
                                           wxFULL_REPAINT_ON_RESIZE)
        {
            auto *context = new MyGLContext(this);
            holdTimer = new wxTimer(this, ID_VIEW_TIMER);
            moveTimer = new wxTimer(this, ID_MOVE_VIEW_TIMER);
            ComputeGrid();
            Bind(wxEVT_PAINT, &MyGLCanvas::OnPaint, this);
            Bind(wxEVT_MOUSEWHEEL, &MyGLCanvas::OnScroll, this);
            Bind(wxEVT_KEY_DOWN, &MyGLCanvas::OnKeyDown, this);
            Bind(wxEVT_RIGHT_DOWN, &MyGLCanvas::OnRightDown, this);
            Bind(wxEVT_RIGHT_UP, &MyGLCanvas::OnRightUp, this);
            Bind(wxEVT_TIMER, &MyGLCanvas::OnRightHolding, this, ID_VIEW_TIMER);
            Bind(wxEVT_LEFT_DOWN, &MyGLCanvas::OnLeftDown, this);
            Bind(wxEVT_LEFT_UP, &MyGLCanvas::OnLeftUp, this);
            Bind(wxEVT_TIMER, &MyGLCanvas::OnLeftHolding, this, ID_MOVE_VIEW_TIMER);
        };

        std::vector<glm::dvec3> renderingTriangles;        

        void loadModel(){
            loadedModel = true;
            Refresh(false);
        }
        void unLoadModel(){
            loadedModel = false;
        }
        bool isModelLoaded(){
            return loadedModel;
        }
        void setSlicer(Slicer *s){
            slicer = s;
        }
        
    private:

        const GLfloat greenColour[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat redColour[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        const GLfloat blueColour[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        const GLfloat purpleColour[4] = {0.5f, 0.0f, 1.0f, 1.0f};
        const GLfloat yellowColour[4] = {1.0f, 1.0f, 0.0f, 1.0f};
        const GLfloat whiteColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        const GLfloat pinkColour[4] = {1.0f, 0.0f, 0.96f, 1.0f};
        const GLfloat greyColour[4] = {0.9f, 0.9f, 0.9f, 1.0f};
        const GLfloat blackColour[4] = {1.0f, 1.0f, 1.0f, 1.0f};

        const std::array<const GLfloat*[4], 7> colours = {greenColour, redColour, blueColour, purpleColour, yellowColour, pinkColour, blackColour};

        bool loadedModel = false;

        //making grid array
        static const int h = 16;
        static const int l = 16;
        float gridScale = 100;
        GLfloat gridPoints[(h+l)*2 + 4][2];

        //TEMP
        unsigned int ti =0;

        //resizing varibales
        const float fovY = 45.0f;
        float aspect;
        const float nearVar = 0.1f;
        const float farVar = 10000000000.0f;
        float top;
        float bottom;
        float right;
        float left;

        const float sensitivity = 0.1f;
        const float moveSensitivity = 0.001f;
        const float scrollSensitivity = 0.3f;

        float cameraDistance = 2000.0f;

        bool holdingR = false;
        bool holdingM = false;

        //old camera method
        // float OGXDelta;
        // float OGYDelta;
        // float TempXDelta;
        // float TempYDelta;

        //new camera method
        float lx = 0.0f;
        float ly = 0.0f;
        float dx;
        float dy;

        float mlx = 0.0f;
        float mly = 0.0f;
        float mdx;
        float mdy;

        
        const glm::vec3 axisY = glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::vec3 axisX = glm::vec3(1.0f, 0.0f, 0.0f);

        float pitch = 0.0f;
        float yaw = 0.0f;

        glm::vec3 cameraPos;
        glm::vec3 target = glm::vec3(0,0,0);

        //camear method 2
        // glm::quat yawQuat;
        // glm::quat pitchQuat;
        // glm::quat cameraQuat;

        wxTimer* holdTimer; 
        wxTimer* moveTimer; 

        Slicer *slicer;
        
        void ComputeGrid(){
            //making grid
            for ( int i = 0 ; i < l+1; i++){
                gridPoints[i*2][0] = (i - (l/2.0f)) * gridScale;
                gridPoints[i*2][1] = (h/2.0f) * gridScale;
                gridPoints[i*2 +1][0] = (i - (l/2.0f)) * gridScale;
                gridPoints[i*2 +1][1] = (-h/2.0f) * gridScale;
            }
            for ( int i = 0 ; i < h + 1 ;i++){
                gridPoints[i*2 + (l+1)*2][0] = (-l/2.0f) * gridScale;
                gridPoints[i*2+ (l+1)*2][1] = (i - (h/2.0f)) * gridScale;
                gridPoints[i*2 +1+ (l+1)*2][0] = (l/2.0f) * gridScale;
                gridPoints[i*2 +1+ (l+1)*2][1] = (i - (h/2.0f)) * gridScale;
            }
        };



        //event methods
        void OnPaint(wxPaintEvent &WXUNUSED(event))
        {
            wxPaintDC dc(this);
            // resizing opengl renderable space. this method is called when the window is resized or has other updates
            const wxSize ClientSize = GetClientSize() * GetContentScaleFactor();

            glMatrixMode(GL_PROJECTION);
            // aspect = ClientSize.x / ClientSize.y;
            aspect = static_cast<float>(ClientSize.x) / static_cast<float>(ClientSize.y);

            top = nearVar * tan(glm::radians(fovY) / 2.0f);
            bottom = -top;
            right = top * aspect;
            left = -right;

            glViewport(0, 0, ClientSize.x, ClientSize.y);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glLoadIdentity();
            glFrustum(left, right, bottom, top, nearVar, farVar);

            cameraPos.x = target.x + cameraDistance * cos(glm::radians(pitch)) * sin(glm::radians(-yaw));
            cameraPos.z = target.z + cameraDistance * cos(glm::radians(pitch)) * cos(glm::radians(-yaw));
            cameraPos.y = target.y + cameraDistance * sin(glm::radians(pitch));
            // std::cout<<yaw<< ", " << pitch << std::endl;
            glm::mat4 cameraMatrix = glm::lookAt(cameraPos, target, glm::vec3(0, 1, 0));
            glMultMatrixf(glm::value_ptr(cameraMatrix));

            // glDisable(GL_CULL_FACE);

            // //positive x indicator
            // glBegin(GL_QUADS);
            //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
            //     glNormal3f( 0.0f, 0.0f, 1.0f);
            //     //bottom left corner
            //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
            //     //bottom right corner
            //     glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
            //     //top right corner
            //     glTexCoord2f(1, 1); glVertex3f(1.0f,1.0f, 0.0f);
            //     //top left corner
            //     glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
            // glEnd();

            // //positive y indicator
            // glBegin(GL_QUADS);
            //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greenColour);
            //     glNormal3f( 0.0f, 1.0f, 0.0f);
            //     //bottom left corner
            //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
            //     //bottom right corner
            //     glTexCoord2f(1, 0); glVertex3f(1.0f, 0.0f, 0.0f);
            //     //top right corner
            //     glTexCoord2f(1, 1); glVertex3f(1.0f,0.0f, 1.0f);
            //     //top left corner
            //     glTexCoord2f(0, 1); glVertex3f( 0.0f, 0.0f, 1.0f);
            // glEnd();

            // //positive z indicator
            // glBegin(GL_QUADS);
            //     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
            //     glNormal3f( 0.0f, 0.0f, 1.0f);
            //     //bottom left corner
            //     glTexCoord2f(0, 0); glVertex3f( 0.0f, 0.0f, 0.0f);
            //     //bottom right corner
            //     glTexCoord2f(1, 0); glVertex3f(0.0f, 0.0f, 1.0f);
            //     //top right corner
            //     glTexCoord2f(1, 1); glVertex3f(0.0f,1.0f, 1.0f);
            //     //top left corner
            //     glTexCoord2f(0, 1); glVertex3f( 0.0f,1.0f, 0.0f);
            // glEnd();

            // glEnable(GL_CULL_FACE);

            // rendering modlels
            if (isModelLoaded())
            {
                glBegin(GL_TRIANGLES);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, purpleColour);
                for (int i = 0; i < renderingTriangles.size() / 4; i++)
                {
                    glNormal3f(renderingTriangles.at(i * 4).x, renderingTriangles.at(i * 4).y, renderingTriangles.at(i * 4).z);
                    // bottom left corner
                    glVertex3d(renderingTriangles.at(i * 4 + 1).x, renderingTriangles.at(i * 4 + 1).y, renderingTriangles.at(i * 4 + 1).z);
                    glVertex3d(renderingTriangles.at(i * 4 + 2).x, renderingTriangles.at(i * 4 + 2).y, renderingTriangles.at(i * 4 + 2).z);
                    glVertex3d(renderingTriangles.at(i * 4 + 3).x, renderingTriangles.at(i * 4 + 3).y, renderingTriangles.at(i * 4 + 3).z);
                }
                glEnd();
            }

            glLineWidth(1.0f);
            glPointSize(5.0f);

            if (slicer->doneToolpath)
            {
                // load points
                double z = 0.0;
                glDisable(GL_DEPTH_TEST);
                glBegin(GL_LINES);

                // drawing segments
                //         //             glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
                //         // for (int i = 0; i < slicer->segments.size(); i++){
                //         //     z = (static_cast<double>(i) * slicer-> layerHeight) + (slicer->layerHeight/2.0);
                //         //     for (int j = 0; j < slicer->segments.at(i).size(); j++){
                //         //         glVertex3d(slicer->segments.at(i).at(j).at(0).x, z, slicer->segments.at(i).at(j).at(0).y);
                //         //         glVertex3d(slicer->segments.at(i).at(j).at(1).x, z, slicer->segments.at(i).at(j).at(1).y);
                //         //     }
                //         // }

                // drawing polygons
                //  z = (static_cast<double>(ti+1) * slicer-> layerHeight) ;//+ (slicer->layerHeight/2.0);
                //  for (std::shared_ptr<polygon> &p : slicer->polygons.at(ti)){
                //      if (p.get()->parent == nullptr ){
                //          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  greenColour);
                //      }else{
                //          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueColour);
                //      }
                //      for (int j = 0; j < p.get()->perimeter.size(); j++){

                //         glVertex3d(p.get()->perimeter.at(j)[0].x, z,p.get()->perimeter.at(j)[0].y);
                //         glVertex3d(p.get()->perimeter.at(j)[1].x, z,p.get()->perimeter.at(j)[1].y);
                //     }
                // }
                // drawing toolpath (first layer)

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
                // for (int j = 0; j < slicer->toolpath.size(); j++){

                z = (static_cast<double>(ti + 1) * slicer->layerHeight);
                for (int i = 0; i < slicer->toolpath[ti].size(); i++)
                {
                    // the second point of the segment is a jump point
                    if ((i + 1 < slicer->toolpath[ti].size() && !slicer->toolpath[ti][i + 1].jumpPoint) || i == 0)
                    {
                        glVertex3d(slicer->toolpath[ti][i].point.x, z, slicer->toolpath[ti][i].point.y);
                        glVertex3d(slicer->toolpath[ti][i + 1].point.x, z, slicer->toolpath[ti][i + 1].point.y);
                    }
                }
                // }
                glEnd();
                //         //debugging thing
                //     glLineWidth(2.0f);
                //     glBegin(GL_LINES);

                //         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColour);
                // glVertex3d(slicer->sTris.at(ti).verticies[0].x, slicer->sTris.at(ti).verticies[0].y, slicer->sTris.at(ti).verticies[0].z);
                //         glVertex3d(slicer->sTris.at(ti).verticies[1].x, slicer->sTris.at(ti).verticies[1].y, slicer->sTris.at(ti).verticies[1].z);
                //         glVertex3d(slicer->sTris.at(ti).verticies[1].x, slicer->sTris.at(ti).verticies[1].y, slicer->sTris.at(ti).verticies[1].z);
                //         glVertex3d(slicer->sTris.at(ti).verticies[2].x, slicer->sTris.at(ti).verticies[2].y, slicer->sTris.at(ti).verticies[2].z);
                //         glVertex3d(slicer->sTris.at(ti).verticies[2].x, slicer->sTris.at(ti).verticies[2].y, slicer->sTris.at(ti).verticies[2].z);
                //         glVertex3d(slicer->sTris.at(ti).verticies[0].x, slicer->sTris.at(ti).verticies[0].y, slicer->sTris.at(ti).verticies[0].z);
                //     glEnd();
                //     glEnable(GL_DEPTH_TEST);
            }

            glLineWidth(1.0f);
            glBegin(GL_LINES);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greyColour);

            glNormal3f(0.0f, 1.0f, 0.0f);
            // OPTIMIZE THIS
            for (int i = 0; i < sizeof(gridPoints) / sizeof(gridPoints[0]); i++)
            {
                glVertex3f(gridPoints[i][0], 0.0f, gridPoints[i][1]);
            }

            glEnd();
            glFlush();
            SwapBuffers();

            // std::cout << "WINDOW REFRESHING: " << ClientSize.x << ", " << ClientSize.y << std::endl;
        };

        void OnLeftDown(wxMouseEvent &event)
        {
            moveTimer->Start(8);
        };
        void OnLeftUp(wxMouseEvent &event)
        {
            moveTimer->Stop();
            holdingM = false;

            mlx, mdy = 0.0f;
        };
        void OnLeftHolding(wxTimerEvent &WXUNUSED(event))
        {
            wxPoint mousePos = ScreenToClient(::wxGetMousePosition());
            if (holdingM)
            {
                mdx = mousePos.x - mlx;
                mdy = mousePos.y - mly;
            }
            else
            {
                mlx = mousePos.x;
                mly = mousePos.y;
                mdx, mdy = 0.0f;

                holdingM = true;
            }

            target.y += mdy * moveSensitivity * cameraDistance * ((89.0f - abs(pitch)) / 89.0f);

            // looking down
            target.z += mdy * moveSensitivity * cameraDistance * -cos(glm::radians(yaw)) * (abs(pitch) / 89.0f);
            target.x += mdy * moveSensitivity * cameraDistance * sin(glm::radians(yaw)) * (abs(pitch) / 89.0f);

            // looking sideways
            target.x += mdx * moveSensitivity * cameraDistance * -cos(glm::radians(yaw));
            target.z += mdx * moveSensitivity * cameraDistance * -sin(glm::radians(yaw));

            mlx = mousePos.x;
            mly = mousePos.y;
            Refresh(false);
        };

        void OnKeyDown(wxKeyEvent &event)
        {
            switch (event.GetKeyCode())
            {
            case WXK_ESCAPE:
                pitch = 0.0f;
                yaw = 0.0f;
                target = glm::vec3(0, 0, 0);

                // old rotation modes
                //  OGXDelta = 0.0f;
                //  TempYDelta = 0.0f;
                //   TempXDelta = 0.0f;
                //   OGYDelta = 0.0f;

                // cameraQuat = glm::quat();
                // yawQuat = glm::quat();
                // pitchQuat = glm::quat();
                // std::cout << "RESET: " << TempXDelta << ", " << TempYDelta << std::endl;
                Refresh(false);
                break;
            case WXK_UP:
                // gridScale += 0.1;
                // ComputeGrid();

                if (ti < slicer->toolpath.size() - 1)
                {
                    ti++;
                }

                Refresh(false);
                break;
            case WXK_DOWN:
                // ComputeGrid();
                // gridScale -= 0.1;

                if (ti > 0)
                {
                    ti--;
                }

                Refresh(false);
                break;
            // p
            //  case 'P':
            //  case 'p':
            //      std::cout<<"\n\n\n"<<std::endl;
            //      for (int v = 0; v < 3; v++){
            //          std::cout<< slicer->sTris.at(ti).verticies[v].x<< ", "<<slicer->sTris.at(ti).verticies[v].y<<", "<< slicer->sTris.at(ti).verticies[v].z<<std::endl;
            //      }
            //      break;
            default:
                event.Skip();
                break;
            }
        };

        void OnRightDown(wxMouseEvent &event)
        {
            // starting the timer to call the onholding method every 8 ms
            holdTimer->Start(8);
            // std::cout << "RIGHT MOUSE PRESSED: " << startingRPos.x << ", " << startingRPos.y << std::endl;
        };
        void OnRightUp(wxMouseEvent &event)
        {
            holdTimer->Stop();
            holdingR = false;

            // resetting deltas
            lx = 0.0f;
            ly = 0.0f;

            // std::cout << "RIGHT MOUSE RELEASED: " << OGXDelta << ", " << OGYDelta << std::endl;
        };

        void OnRightHolding(wxTimerEvent &WXUNUSED(event))
        {
            wxPoint mousePos = ScreenToClient(::wxGetMousePosition());

            // new looking method
            if (holdingR)
            {
                dx = mousePos.x - lx;
                dy = mousePos.y - ly;
            }
            else
            {
                lx = mousePos.x;
                ly = mousePos.y;
                dx, dy = 0.0f;
                holdingR = true;
            }

            pitch += dy * sensitivity;
            yaw += dx * sensitivity;
            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            lx = mousePos.x;
            ly = mousePos.y;
            Refresh(false);
        };

        void OnScroll(wxMouseEvent &event)
        {
            cameraDistance += event.GetWheelRotation() * scrollSensitivity;
            cameraDistance = glm::clamp(cameraDistance, 20.0f, 100000.0f);
            Refresh(false);
        };
};

