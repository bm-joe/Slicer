
#pragma once
#include <wx/wx.h>
class GaugeOverlay : public wxFrame{
    public:
        GaugeOverlay(wxWindow* parent) : wxFrame(parent, wxID_ANY, "progress", wxDefaultPosition, wxDefaultSize, wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP){
            gauge = new wxGauge(this, wxID_ANY, 100);
            SetClientSize(gauge->GetBestSize());
        }
    
        void setValue(int v){
            gauge->SetValue(v);
        }

    private:
        wxGauge *gauge;
};
