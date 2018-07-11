#pragma once

#include <GL/glew.h>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <map>

class WDEdDoomFlatHandler : public wxImageHandler {
    public:
        WDEdDoomFlatHandler() : palette(nullptr) {}
        ~WDEdDoomFlatHandler() {delete palette;}
        virtual int DoGetImageCount (wxInputStream &stream) {return 1;}
        virtual bool DoCanRead (wxInputStream &stream);
        virtual bool LoadFile (wxImage *image, wxInputStream & stream, bool verbose = true, int index = -1);
        virtual bool SaveFile (wxImage *image, wxOutputStream & stream, bool verbose = true);
        void DeferPalette(wxImage  *image, unsigned char *buf);
        void LoadFileFinal(wxImage  *image, unsigned char *buf);
        void SetPalette(unsigned char *buf);
    private:
        unsigned char *palette;
        std::map<wxImage *, unsigned char *> deferred;
};

class WDEdTexture2D {
    public:
        WDEdTexture2D(wxImage *img, wxString name = "<unknown texture>", bool owns = true);
        ~WDEdTexture2D();
        void Bind(wxGLContext *);
        void Destroy(wxGLContext *);
        wxImage *img;
        wxString name;
    private:
        std::map<wxGLContext *, GLuint> units;
        bool owns;
        
};