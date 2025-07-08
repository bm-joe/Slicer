
#include <wx/wxprec.h>
#include <wx/glcanvas.h> 
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif


// defining Application class as a child of wxApp class
class MyApp : public wxApp{
    //defining a virtual empty method called OnInit to override later
    public:
        virtual bool OnInit();
};

//defining application frame class as a child of wxFrame class
class MyFrame : public wxFrame{
    public: 
        //constructor
        MyFrame();
    private:
        //private methods for clicking buttons and stuff
        void OnHello(wxCommandEvent& helloEvent);
        void OnExit(wxCommandEvent& exitEvent);
        void OnAbout(wxCommandEvent& abortEvent);
};

//list indexes/IDs of menu options 
enum{
    myID_HELLO = 1
};

//calling macro to create main window 
wxIMPLEMENT_APP(MyApp);

//overloading oninit method of myapp to show the frame
bool MyApp::OnInit(){
    //creating new frame
    MyFrame *frame = new MyFrame();
    //showing the frame
    frame->Show(true);
    //returning true to indicate successful startup
    return(true);
}

//overloading myframe method in myframe class to add a window name
MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "hello world - window titele"){
        //creating new pointer to wxmenu object which contains the different menu dropdown options
        wxMenu *menuFile = new wxMenu;
        //appending new entry to menu for a hello option/button
        menuFile->Append(myID_HELLO, "&Hello...\tCtrl-H", "help string showqin in the status bar for this menu item ");
        //appending a line between menu options 
        menuFile->AppendSeparator();
        //appending a quit button to the menu options 
        menuFile->Append(wxID_EXIT);

        //creating new pointer to wxmenu option for the help menu 
        wxMenu *menuHelp = new wxMenu();
        //appending an about button to the help menu 
        menuHelp -> Append(wxID_ABOUT);

        //creating new menubar object (pointer )
        wxMenuBar *menuBar = new wxMenuBar;

        menuBar->Append(menuFile, "file");
        menuBar->Append(menuHelp, "help");

        //calling myFrame.SetMenuBar() method to set the current menu bar
        SetMenuBar(menuBar);

        //creating status bar to show the alt text when hovering over something and setting the default message  
        CreateStatusBar();
        SetStatusText("default message");

        //binding onhello method to the correct menu id
        Bind(wxEVT_MENU, &MyFrame::OnHello, this, myID_HELLO);
        //binding onabout to the wx about id
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
        //binding onexit to the wx exit id
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    }

//overloading onexit method of myframe outside of class definition
void MyFrame::OnExit(wxCommandEvent& event){
    //closing application
    Close(true);
}

//overloading onabout method of myframe
void MyFrame::OnAbout(wxCommandEvent& event){
    //creating a message box that tells user about the application
    wxMessageBox("This is an application by Luke Fadel", /*alt text:*/ "this application is gaming", wxCLOSE_BOX | wxICON_ERROR);

}
//overloading onHello method of myframe
void MyFrame::OnHello(wxCommandEvent& event){
    wxLogMessage("hai bro");
}
