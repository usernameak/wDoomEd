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
    deferred.clear();
}

bool WDEdDoomFlatHandler::SaveFile (wxImage *image, wxOutputStream & stream, bool verbose) {
    // Not implemented
    return false;
}



bool WDEdDoomPatchHandler::DoCanRead (wxInputStream &stream) {
    return stream.GetLength() != 0; // what to put here? :thinking:
}

bool WDEdDoomPatchHandler::LoadFile (wxImage *image, wxInputStream & stream, bool verbose, int index) {
    if(!CanRead(stream)) return false;
    if(image->IsOk() && image->HasAlpha()) image->ClearAlpha();
    uint16_t width, height;
    int16_t offsetX, offsetY;
    if(!stream.ReadAll(&width, sizeof(width))) return false;
    if(!stream.ReadAll(&height, sizeof(height))) return false;
    if(!stream.ReadAll(&offsetX, sizeof(offsetX))) return false;
    if(!stream.ReadAll(&offsetY, sizeof(offsetY))) return false;
    uint32_t columnPtrs[width];
    if(!stream.ReadAll(&columnPtrs, sizeof(columnPtrs))) return false;
    int bufSize = width * height * 2;
    int16_t *buf = new int16_t[width * height];
    memset(buf, -1, bufSize);
    for(int x = 0; x < width; x++) {
        stream.SeekI(columnPtrs[x], wxFromStart);
        while(true) {
            uint8_t spanPos;
            uint8_t spanSize;
            if(!stream.ReadAll(&spanPos, sizeof(spanPos))) return false;
            if(spanPos == 0xFF) break;
            if(!stream.ReadAll(&spanSize, sizeof(spanSize))) return false;
            uint16_t unknown;
            if(!stream.ReadAll(&unknown, sizeof(unknown))) return false;
            for(int y = spanPos; y < spanPos + spanSize; y++) {
                if(y + 1 >= height) goto next;
                uint8_t shit;
                if(!stream.ReadAll(&shit, sizeof(shit))) {
                	goto next;
                }
                buf[y * width + x] = shit;
            }
        }
        next:;
    }
    image->SetData((unsigned char *) malloc(width * height * 3), width, height);
    DeferPalette(image, buf);
    return true;
}

void WDEdDoomPatchHandler::DeferPalette (wxImage *image, int16_t *buf) {
    if(palette) LoadFileFinal(image, buf);
    else deferred[image] = buf;
}

void WDEdDoomPatchHandler::LoadFileFinal(wxImage  *image, int16_t *buf) {
    unsigned char *buf2 = (unsigned char *) malloc(image->GetWidth() * image->GetHeight() * 3);
    unsigned char *buf3 = (unsigned char *) malloc(image->GetWidth() * image->GetHeight());
    for(int i = 0; i < image->GetWidth() * image->GetHeight(); i++) {
        if(buf[i] >= 0) {
            buf2[i * 3] = palette[buf[i] * 3];
            buf2[i * 3 + 1] = palette[buf[i] * 3 + 1];
            buf2[i * 3 + 2] = palette[buf[i] * 3 + 2];
            buf3[i] = 255;
        } else {
            buf3[i] = 0;
        }
    }
    delete[] buf;
    image->SetData(buf2);
    image->SetAlpha(buf3);
}

void WDEdDoomPatchHandler::SetPalette(unsigned char *buf) {
    palette = buf;
    for(std::map<wxImage *, int16_t *>::iterator it = deferred.begin(); it != deferred.end(); it++) {
        LoadFileFinal(it->first, it->second);
    }
    deferred.clear();
}


bool WDEdDoomPatchHandler::SaveFile (wxImage *image, wxOutputStream & stream, bool verbose) {
    // Not implemented
    return false;
}


WDEdTexture2D::WDEdTexture2D(wxImage *img, wxString name, bool owns) : name(name), owns(owns) {

    imageWidth = img->GetWidth();
    imageHeight = img->GetHeight();
    GLubyte *bitmapData=img->GetData();
    hasAlpha = img->HasAlpha();
    GLubyte *alphaData=img->HasAlpha()?img->GetAlpha():nullptr;
    bytesPerPixel = img->HasAlpha()?4:3;
    int imageSize = imageWidth * imageHeight * bytesPerPixel;
	imageData=new GLubyte[imageSize];

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
	this->bitmap = new wxBitmap(*img);
	if(owns) delete img;
}

WDEdTexture2D::~WDEdTexture2D() {
	delete bitmap;
}

void WDEdTexture2D::Bind(wxGLContext *ctx) {
    bool haveToCreate = false;
    if(!units.count(ctx)) {
        haveToCreate = true;
        glGenTextures(1, &units[ctx]);
    }

    glBindTexture(GL_TEXTURE_2D, units[ctx]);

    if(haveToCreate) {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     bytesPerPixel,
                     imageWidth,
                     imageHeight,
                     0,
                     hasAlpha?  GL_RGBA : GL_RGB,
                     GL_UNSIGNED_BYTE,
                     imageData);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}

void WDEdTexture2D::Destroy(wxGLContext *ctx) {
    if(!units.count(ctx)) return;
    glDeleteTextures(1, &units[ctx]);
}
