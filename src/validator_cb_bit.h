#pragma once

#include <wx/wx.h>
#include <wx/defs.h> 

template <typename T> class WDEdCBBitValidator : public wxValidator {
    T *val;
    T bitmask;
public:
    WDEdCBBitValidator(T *val, T bitmask) : wxValidator(), val(val), bitmask(bitmask) {
        wxPrintf("Validator init\n");
    }

    WDEdCBBitValidator(const WDEdCBBitValidator& v) : wxValidator(v) {
        val = v.val;
        bitmask = v.bitmask;
    }

    virtual ~WDEdCBBitValidator() {}

    virtual wxObject *Clone() const {
        wxPrintf("Validator Clone %p %p\n", m_validatorWindow, val);
        return new WDEdCBBitValidator<T>(*this);
    }

    // Called when the value in the window must be validated: this is not used
    // by this class
    virtual bool Validate(wxWindow * WXUNUSED(parent)) {
        return true;
    }

    // Called to transfer data to the window
    virtual bool TransferToWindow() override {
        wxPrintf("TransferToWindow %p %p\n", m_validatorWindow, val);
        if(!m_validatorWindow || !val) return false;
        wxCheckBox *cb = (wxCheckBox *) m_validatorWindow;
        cb->SetValue((*val & bitmask) ? true : false);
        return true;
    }

    // Called to transfer data from the window
    virtual bool TransferFromWindow() override {
        wxPrintf("TransferFromWindow %p %p\n", m_validatorWindow, val);
        if(!m_validatorWindow || !val) return false;
        wxCheckBox *cb = (wxCheckBox *) m_validatorWindow;
        if(cb->GetValue()) {
            *val |= bitmask;
        } else {
            *val &= ~bitmask;
        }
        return true;
    }
};