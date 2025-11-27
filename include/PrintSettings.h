#pragma once
#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>
class PrintSettings : public wxFrame{
    public:
        //constructor
        PrintSettings(Slicer *s): wxFrame(NULL, wxID_ANY, "print settings", wxDefaultPosition, wxDefaultSize){
            //making menu options with a property grid
            slicer = s;
            pg = new wxPropertyGrid(
                this,              // parent
                wxID_ANY,          // id
                wxDefaultPosition, // position
                wxDefaultSize,     // size
                // Here are just some of the supported window styles
                wxPG_AUTO_SORT |                // Automatic sorting after items added
                wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
                // Default style
                wxPG_DEFAULT_STYLE
            );
            pg->SetSize(GetSize());
            layerHeightProperty = new wxFloatProperty("Layer Height (mm)", wxPG_LABEL, 0.2);
            infillWidthProperty = new wxFloatProperty("Infill Width (mm)", wxPG_LABEL, 0.5);
            filamentDiameterProperty = new wxFloatProperty("Filament Diameter (mm)", wxPG_LABEL, 1.75);
            pg->Append( layerHeightProperty );
            pg->Append( infillWidthProperty );
            pg->Append( filamentDiameterProperty );

            Bind(wxEVT_PG_CHANGED, &OnPGChanged, this);
        }
        void OnPGChanged(wxPropertyGridEvent& event){
            if (event.GetProperty() == layerHeightProperty){
                //layer height
                slicer->layerHeight = glm::clamp(layerHeightProperty->GetValue().GetDouble(), 0.1, 10.0);
                layerHeightProperty->SetValue(slicer->layerHeight);
            }else if (event.GetProperty() == filamentDiameterProperty){
                //filament diameter
                slicer->filamentDiameter = glm::clamp(filamentDiameterProperty->GetValue().GetDouble(), 0.1, 10.0);
                filamentDiameterProperty->SetValue(slicer->filamentDiameter);
            }else if(event.GetProperty() == infillWidthProperty){
                //infill width
                slicer->infillWidth = glm::clamp(infillWidthProperty->GetValue().GetDouble(), 0.1, 10.0);
                infillWidthProperty->SetValue(slicer->infillWidth);

            }
        }
    private:
        wxPropertyGrid* pg;
        wxFloatProperty* layerHeightProperty;
        wxFloatProperty* infillWidthProperty;
        wxFloatProperty* filamentDiameterProperty;
        Slicer* slicer;
        //layer hieght
        // infilil width
        //filmanet diameter
};
