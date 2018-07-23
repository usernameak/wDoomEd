#pragma once

#include <wx/wx.h>
#include <wx/gbsizer.h>
#include <wx/valnum.h>

#include "map_editor.h"
#include "validator_cb_bit.h"
#include "validator_combo.h"
#include <initializer_list>

#include <vector>

template <typename T> struct WDEdBitCheckboxProperties {
    wxString name;
    T bitmask;
};

template <typename T> struct WDEdNumericComboboxProperties {
	T number;
	wxString name;
};

class WDEdPropertiesDialog : public wxDialog {
private:
    DECLARE_EVENT_TABLE()
    wxBoxSizer *rootSizer;
    wxGridBagSizer *sizer;
    wxPanel *panel;
public:
    WDEdPropertiesDialog(wxWindow *parent);
    void ProcessButton(wxCommandEvent &event);

    WDEdPropertiesDialog *AddNoBorderGroup() {
        panel = new wxPanel(this);
        sizer = new wxGridBagSizer(10, 10);
        panel->SetSizer(sizer);
        rootSizer->Add(panel, wxSizerFlags(0).Expand().Border(wxALL));
        return this;
    }

    WDEdPropertiesDialog *AddGroup(wxString name) {
        wxPanel *outerPanel = new wxPanel(this);
        wxStaticBoxSizer *sbs;
        outerPanel->SetSizer(sbs = new wxStaticBoxSizer(new wxStaticBox(outerPanel, wxID_ANY, name), wxVERTICAL));
        panel = new wxPanel(outerPanel);
        sbs->Add(panel, wxSizerFlags(0).Expand().Border(wxALL));
        sizer = new wxGridBagSizer(10, 10);
        panel->SetSizer(sizer);
        rootSizer->Add(outerPanel, wxSizerFlags(0).Expand().Border(wxALL));
        return this;
    }

    template <typename T> WDEdPropertiesDialog *AddNumberField(wxString name, T *value) {
        sizer->Add(new wxStaticText(panel, wxID_ANY, name), wxGBPosition(sizer->GetRows(), 0), wxDefaultSpan, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
        sizer->Add(new wxTextCtrl(panel,
                                  wxID_ANY,
                                  wxString::Format("%d", *value),
                                  wxDefaultPosition, wxDefaultSize,
                                  0L,
                                  wxIntegerValidator<T>(value)
                                 ), wxGBPosition(sizer->GetRows() - 1, 1), wxDefaultSpan, wxEXPAND);
        if(!sizer->IsColGrowable(1)) sizer->AddGrowableCol(1, 1);
        return this;
    }

    template <typename T> WDEdPropertiesDialog *AddBitCheckbox(wxString name, T *value, T bitmask) {
        sizer->Add(new wxCheckBox(panel,
                                  wxID_ANY,
                                  name,
                                  wxDefaultPosition, wxDefaultSize,
                                  0L,
                                  WDEdCBBitValidator<T>(value, bitmask)
                                 ), wxGBPosition(sizer->GetRows(), 0), wxGBSpan(1, 2), wxEXPAND);
        return this;
    }

    template <typename T> WDEdPropertiesDialog *AddBitCheckboxGroup(wxString name, T *value, std::initializer_list<WDEdBitCheckboxProperties<T> > props) {
        AddGroup(name);
        int i = 0;
        int j = sizer->GetRows();
        for(auto &prop : props) {
            sizer->Add(new wxCheckBox(panel,
                                      wxID_ANY,
                                      prop.name,
                                      wxDefaultPosition, wxDefaultSize,
                                      0L, WDEdCBBitValidator<T>(value, prop.bitmask)
                                     ), wxGBPosition(j, i++));
            if(i != i % 4) {
                j++;
                i = i % 4;
            }
        }
        return this;
    }

    WDEdPropertiesDialog *AddTextureField(wxString name, std::map<wxString, WDEdTexture2D *> textures, char (*value)[8]);

    template <typename T> WDEdPropertiesDialog *AddNumericComboBox(wxString name, T* value, T bitmask, std::initializer_list<WDEdNumericComboboxProperties<T> > props) {
    	wxString choices[props.size()];
    	int i = 0;
    	T oldValue = (*value) & bitmask;
    	wxString oldStr = wxString::Format("%d", oldValue);
    	for(auto &prop : props) {
    		choices[i] = wxString::Format("%d: %s", prop.number, prop.name);
    		if(prop.number == oldValue)
    			oldStr = choices[i];
    		i++;
    	}
    	sizer->Add(new wxStaticText(panel, wxID_ANY, name), wxGBPosition(sizer->GetRows(), 0), wxDefaultSpan, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    	sizer->Add(new wxComboBox(panel,
								  wxID_ANY,
								  oldStr,
								  wxDefaultPosition, wxDefaultSize,
								  i, choices, 0L, WDEdValidatorNumericCombobox<T>(value, bitmask, oldStr)
								 ), wxGBPosition(sizer->GetRows() - 1, 1), wxDefaultSpan, wxEXPAND);
    	if(!sizer->IsColGrowable(1)) sizer->AddGrowableCol(1, 1);
    	return this;
    }
    WDEdPropertiesDialog *Finish();
};
