#pragma once

#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/regex.h>
#include <type_traits>

template <typename T> class WDEdValidatorNumericCombobox : public wxValidator {
    T *val;
    T bitmask;
	wxString oldStr;

public:
    WDEdValidatorNumericCombobox(T *val, T bitmask, wxString oldStr) : wxValidator(), val(val), bitmask(bitmask), oldStr(oldStr) {}

    WDEdValidatorNumericCombobox(const WDEdValidatorNumericCombobox& v) : wxValidator(v) {
        val = v.val;
        bitmask = v.bitmask;
        oldStr = v.oldStr;
    }

    virtual ~WDEdValidatorNumericCombobox() {}

    virtual wxObject *Clone() const {
        return new WDEdValidatorNumericCombobox<T>(*this);
    }

    // Called when the value in the window must be validated
    virtual bool Validate(wxWindow * parent) {
        wxComboBox *combo = (wxComboBox *) m_validatorWindow;
        wxString value = combo->GetValue();
        wxRegEx reValue(std::is_unsigned<T>::value ? "^[[:digit:]]+.*$" : "^-?[[:digit:]]+.*$");
        return reValue.Matches(value, 0);
    }

    // Called to transfer data to the window
    virtual bool TransferToWindow() {
        if(!m_validatorWindow || !val) return false;
        wxComboBox *combo = (wxComboBox *) m_validatorWindow;
        combo->SetValue(oldStr);
        return true;
    }

    // Called to transfer data from the window
    virtual bool TransferFromWindow() {
        if(!m_validatorWindow || !val) return false;
        if(!Validate(nullptr)) return false;
        wxComboBox *combo = (wxComboBox *) m_validatorWindow;
        wxRegEx reValue(std::is_unsigned<T>::value ? "^([[:digit:]]+).*$" : "^(-?[[:digit:]]+).*$");
        reValue.Matches(combo->GetValue());
        unsigned long urval;
        long rval;
        wxString rvals = reValue.GetMatch(combo->GetValue(), 1);
        if(std::is_unsigned<T>::value) {
        	rvals.ToCULong(&urval);
        	*val = ((T) urval) & bitmask;
        } else {
        	rvals.ToCLong(&rval);
        	*val = ((T) rval) & bitmask;
        }
        return true;
    }

    wxDECLARE_NO_ASSIGN_CLASS(WDEdValidatorNumericCombobox);
};
