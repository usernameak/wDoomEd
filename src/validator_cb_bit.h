#pragma once

#include <wx/wx.h>
#include <wx/defs.h> 

template <typename T> class WDEdCBBitValidator : public wxValidator {
    T *val;
    T bitmask;
public:
    WDEdCBBitValidator(T *val, T bitmask) : wxValidator(), val(val), bitmask(bitmask) {}

    WDEdCBBitValidator(const WDEdCBBitValidator& v) : wxValidator(v) {
        val = v.val;
        bitmask = v.bitmask;
    }

    virtual ~WDEdCBBitValidator() {}

    virtual wxObject *Clone() const {
        return new WDEdCBBitValidator<T>(*this);
    }

    // Called when the value in the window must be validated: this is not used
    // by this class
    virtual bool Validate(wxWindow * WXUNUSED(parent)) {
        return true;
    }

    // Called to transfer data to the window
    virtual bool TransferToWindow() {
        if(!m_validatorWindow || !val) return false;
        wxCheckBox *cb = (wxCheckBox *) m_validatorWindow;
        cb->SetValue((*val & bitmask) ? true : false);
        return true;
    }

    // Called to transfer data from the window
    virtual bool TransferFromWindow() {
        if(!m_validatorWindow || !val) return false;
        wxCheckBox *cb = (wxCheckBox *) m_validatorWindow;
        if(cb->GetValue()) {
            *val |= bitmask;
        } else {
            *val &= ~bitmask;
        }
        return true;
    }

    wxDECLARE_NO_ASSIGN_CLASS(WDEdCBBitValidator);
};