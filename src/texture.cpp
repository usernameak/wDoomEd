#include "texture.h"

bool WDEdDoomFlatHandler::DoCanRead (wxInputStream &stream) {
    return stream.GetLength() == 4096;
}

bool WDEdDoomFlatHandler::LoadFile (wxImage *image, wxInputStream & stream, bool verbose, int index) {
    if(!CanRead(stream)) return false;
    if(image->IsOk() && image->HasAlpha()) image->ClearAlpha();
    unsigned char *buf = (unsigned char *) malloc(4096);
    if(!stream.ReadAll(buf, 4096)) {
        free(buf);
        return false;
    }
    DeferPalette(image, buf);
    return true;
    
}

void WDEdDoomFlatHandler::LoadFileFinal(wxImage  *image, unsigned char *buf) {
    unsigned char *buf2 = (unsigned char *) malloc(4096 * 3);
    for(int i = 0; i < 4096; i++) {
        buf2[i * 3] = palette[buf[i] * 3];
        buf2[i * 3 + 1] = palette[buf[i] * 3 + 1];
        buf2[i * 3 + 2] = palette[buf[i] * 3 + 2];
    }
    free(buf);
    image->SetData(buf2, 64, 64);
}

void WDEdDoomFlatHandler::DeferPalette(wxImage *image, unsigned char *buf) {
    if(palette) LoadFileFinal(image, buf);
    deferred[image] = buf;
}

void WDEdDoomFlatHandler::SetPalette(unsigned char *buf) {
    palette = buf;
    for(std::map<wxImage *, unsigned char *>::iterator it = deferred.begin(); it != deferred.end(); it++) {
        LoadFileFinal(it->first, it->second);
    }
}

bool WDEdDoomFlatHandler::SaveFile (wxImage *image, wxOutputStream & stream, bool verbose) {
    // Not implemented
    return false;
}


WDEdTexture2D::WDEdTexture2D(wxImage *img, wxString name, bool owns) : img(img), name(name), owns(owns) {}

WDEdTexture2D::~WDEdTexture2D() {
    if(owns) delete img;
}

void WDEdTexture2D::Bind(wxGLContext *ctx) {
    bool haveToCreate = false;
    if(!units.count(ctx)) {
        haveToCreate = true;
        glGenTextures(1, &units[ctx]);
    }

    glBindTexture(GL_TEXTURE_2D, units[ctx]);

    if(haveToCreate) {
        int imageWidth = img->GetWidth();
        int imageHeight = img->GetHeight();
        GLubyte *bitmapData=img->GetData();
        GLubyte *alphaData=img->GetAlpha();

        int bytesPerPixel = img->HasAlpha() ?  4 : 3;

        int imageSize = imageWidth * imageHeight * bytesPerPixel;
        GLubyte *imageData=new GLubyte[imageSize];

        int rev_val=imageHeight-1;

        for(int y=0; y<imageHeight; y++) {
            for(int x=0; x<imageWidth; x++) {
                imageData[(x+y*imageWidth)*bytesPerPixel+0]=
                bitmapData[( x+(rev_val-y)*imageWidth)*3];

                imageData[(x+y*imageWidth)*bytesPerPixel+1]=
                bitmapData[( x+(rev_val-y)*imageWidth)*3 + 1];

                imageData[(x+y*imageWidth)*bytesPerPixel+2]=
                bitmapData[( x+(rev_val-y)*imageWidth)*3 + 2];

                if(bytesPerPixel==4) imageData[(x+y*imageWidth)*bytesPerPixel+3]=
                alphaData[ x+(rev_val-y)*imageWidth ];
            }
        }

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     bytesPerPixel,
                     imageWidth,
                     imageHeight,
                     0,
                     img->HasAlpha() ?  GL_RGBA : GL_RGB,
                     GL_UNSIGNED_BYTE,
                     imageData);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        delete [] imageData;
    }
}

void WDEdTexture2D::Destroy(wxGLContext *ctx) {
    if(!units.count(ctx)) return;
    glDeleteTextures(1, &units[ctx]);
}