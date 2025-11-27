#pragma once
#include <wx/wx.h>
#include "MyGLCanvas.h"
#include "STLHandler.h"
#include "Slicer.h"
#include "GaugeOverlay.h"
#include "PrintSettings.h"
#include "enums.h"
#include <fstream>
#include <sstream>
#include <thread>

class MyFrame : public wxFrame{
    public: 
        //constuructor 
        MyFrame() : wxFrame(NULL, wxID_ANY, "SussySlicer v0 - BETA" /*, wxDefaultPosition, wxSize(500, 500)*/)
        {
            canvas = new MyGLCanvas(this);
            slicer = new Slicer();
            gaugeOverlay = new GaugeOverlay(this);
            pSettings = new PrintSettings(slicer);
            pSettings->Move(this->GetScreenPosition());
            gaugeOverlay->Move(this->GetScreenPosition());
            canvas->setSlicer(slicer);

            STLManager = new STLHandler();
            // making menu bar
            wxMenuBar *menuBar = new wxMenuBar;

            wxMenu *menuFile = new wxMenu;
            menuFile->Append(ID_OPEN_FILE, "&Open STL File\tCtrl-O");
            menuFile->Append(ID_SLICE, "&Slice Currently Loaded Model\tCtrl-S");
            menuFile->Append(wxID_EXIT, "&Exit the application\tCtrl-Q");

            menuBar->Append(menuFile, "&File");

            wxMenu *menuSettings = new wxMenu;
            menuSettings->Append(ID_PRINT_SETTINGS, "&Print settings");
            menuSettings->Append(ID_APPLICATION_SETTINGS, "&Application settings");

            menuBar->Append(menuSettings, "&Settings");

            SetMenuBar(menuBar);

            slicingTimer = new wxTimer(this, ID_SLICING_PROGRESS_TIMER);
            // bindingn to event manager
            Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
            Bind(wxEVT_MENU, &MyFrame::OnOpenFile, this, ID_OPEN_FILE);

            Bind(wxEVT_MENU, &MyFrame::OnPrintSettings, this, ID_PRINT_SETTINGS);
            Bind(wxEVT_MENU, &MyFrame::OnApplicationSettings, this, ID_APPLICATION_SETTINGS);

            Bind(wxEVT_TIMER, &MyFrame::OnSlicing, this, ID_SLICING_PROGRESS_TIMER);
            Bind(wxEVT_MENU, &MyFrame::OnSlice, this, ID_SLICE);
            Bind(wxEVT_MOVE, &MyFrame::OnMove, this);
        };

    private:

        wxTimer* slicingTimer; 
        MyGLCanvas *canvas;
        STLHandler *STLManager;
        Slicer *slicer;
        GaugeOverlay *gaugeOverlay;
        PrintSettings *pSettings;
        wxString name;
        //methods
        void OnExit(wxCommandEvent& event){
            Close(true);
        };
        void OnOpenFile(wxCommandEvent& event){
            //if a model isn't loaded
            if(canvas->isModelLoaded()){
                canvas->renderingTriangles.clear();
                canvas->unLoadModel();
            }
            wxFileDialog openFileDialog(
                this,                                  // parent window
                "Open STL File",                       // dialog title
                "",                                    // default directory
                "",                                    // default filename
                "STL files (*.STL;*.stl)|*.STL;*.stl", // file filter
                wxFD_OPEN | wxFD_FILE_MUST_EXIST
            );

            if (openFileDialog.ShowModal() == wxID_OK) {
                wxString path = openFileDialog.GetPath();  // Full selected file path
                name = openFileDialog.GetFilename();
                if (name.size()> 4){
                    //removing file extension
                    name.erase(name.end() -4, name.end());
                }else{
                    name = "SussySlicer";
                }
                std::cout<<"name is "<<name<<std::endl;

                //parsing file
                std::ifstream inputFile(path.ToStdString());
                if(!inputFile.is_open()){
                    wxMessageBox("There was an error accessing the selected file", "File Error", wxCLOSE | wxICON_ERROR);
                    return;
                }
                //ascii stls 
                // new line character = 10
                STLManager->parseASCII(canvas->renderingTriangles, inputFile);
                //processing the stl output and switching it from z up to y up


                // std::cout << canvas -> triangles.at(1)[0] << ", " << canvas -> triangles.at(1)[1]<< ", " << canvas -> triangles.at(1)[2] << std::endl;
                canvas->loadModel();
                std::cout<<"model is loaded"<<std::endl;
            }
        };
        void OnSlice(wxCommandEvent& event){
            if (canvas->isModelLoaded()){
                gaugeOverlay->Show();
                Refresh(false);
                std::thread ts(&Slicer::slice, slicer, canvas->renderingTriangles, name);
                ts.detach();
                slicingTimer->Start(10);

            }else{
                wxMessageBox("There is no model loaded", "Slice Error", wxCLOSE | wxICON_ERROR);
            }
        };
        
        void OnMove(wxMoveEvent& event){
            // event.Skip();
            // if (gaugeOverlay->IsShown()){
                gaugeOverlay->Move(event.GetPosition());
            // }
        };

        void OnSlicing(wxTimerEvent& WXUNUSED(event)){
            gaugeOverlay->setValue(slicer->slicingProgress);

            if(slicer->doneSlicing){
                canvas->Refresh(false);
                slicingTimer->Stop();
                gaugeOverlay->Hide();

            }
        }

        void OnPrintSettings(wxCommandEvent& event){
            pSettings->Show();
        }

        void OnApplicationSettings(wxCommandEvent& event){

        }
};
