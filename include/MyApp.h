#pragma once
#include <wx/wx.h>
//defining double comparing methods
//defining myapp and myframe classes to override later
class MyApp : public  wxApp {
    public:
        //startup method 
        virtual bool OnInit();
};