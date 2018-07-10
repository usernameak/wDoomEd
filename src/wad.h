#pragma once

#include <wx/wx.h>
#include <wx/archive.h>
#include <stdint.h>

struct WDEdWADEntryDesc {
    uint32_t lumpPointer;
    uint32_t lumpSize;
    char name[8];
};

class WDEdWADEntry;

class WDEdWADInputStream : public wxArchiveInputStream {
private:
    size_t m_last_read;
    WDEdWADEntry *m_current_entry = nullptr;
    struct {
        char desc[4];
        uint32_t numEntries;
        uint32_t dirPointer;
    } m_header_desc;
    uint32_t m_next_directory_item;
public:
    using wxInputStream::Read;

    WDEdWADInputStream(wxInputStream *);
    WDEdWADInputStream(wxInputStream &);
    void Init();

    wxString GetWadType() const;

    virtual ~WDEdWADInputStream();
    virtual bool IsSeekable() const;
    virtual bool IsOk() const;
    virtual wxFileOffset GetLength() const;
    virtual size_t GetSize() const;

    virtual wxFileOffset TellI() const;
    virtual wxFileOffset SeekI(wxFileOffset pos, wxSeekMode mode=wxFromStart);
    virtual wxInputStream &Read (void *buffer, size_t size);
    virtual char Peek ();
    virtual size_t LastRead () const;
    virtual bool Eof () const;
    virtual bool CanRead () const;

    virtual bool OpenEntry (wxArchiveEntry &entry);
    virtual wxArchiveEntry *DoGetNextEntry ();
    virtual bool CloseEntry ();
protected:
    virtual size_t OnSysRead(void *buffer, size_t size);
};

class WDEdWADEntry : public wxArchiveEntry {
private:
    WDEdWADEntryDesc desc;
    friend wxArchiveEntry *WDEdWADInputStream::DoGetNextEntry();
public:
    WDEdWADEntry();
    virtual wxDateTime GetDateTime() const;
    virtual wxFileOffset GetSize() const;
    virtual wxFileOffset GetOffset() const;
    virtual bool IsDir() const;
    virtual bool IsReadOnly() const;
    virtual wxString GetInternalName() const;
    virtual wxPathFormat GetInternalFormat() const;
    virtual wxString GetName(wxPathFormat) const;
    virtual void SetDateTime(const wxDateTime&);
    virtual void SetSize(wxFileOffset);
    virtual void SetIsDir(bool);
    virtual void SetIsReadOnly(bool);
    virtual void SetName(const wxString&, wxPathFormat);
    virtual void SetOffset(wxFileOffset);
    virtual wxArchiveEntry* DoClone() const;
    inline const WDEdWADEntryDesc &GetDesc() {return desc;}
};

class WDEdWADOutputStream : public wxArchiveOutputStream {
        size_t m_last_write;
        wxVector<WDEdWADEntry> entries;
        WDEdWADEntry *currentEntry = nullptr;
    public:
        WDEdWADOutputStream(wxOutputStream *, wxString wadType);
        WDEdWADOutputStream(wxOutputStream &, wxString wadType);
        ~WDEdWADOutputStream();
        void Init(wxString &);

        virtual bool PutNextEntry(wxArchiveEntry *entry);
        virtual bool PutNextEntry(const wxString&, const wxDateTime&, wxFileOffset);
        virtual bool PutNextDirEntry(const wxString&, const wxDateTime&);
        virtual bool CopyEntry(wxArchiveEntry*, wxArchiveInputStream&);
        virtual bool CopyArchiveMetaData(wxArchiveInputStream& stream);
        virtual bool CloseEntry();
        virtual bool Close();

        virtual wxFileOffset SeekO (wxFileOffset pos, wxSeekMode mode=wxFromStart);
        virtual wxFileOffset TellO () const;
        virtual wxOutputStream &Write(const void *buffer, size_t size);
        virtual size_t LastWrite () const;

        virtual bool IsSeekable() const;
        virtual bool IsOk() const;
        virtual wxFileOffset GetLength() const;
        virtual size_t GetSize() const;
    protected:
        virtual wxFileOffset OnSysTell () const;
        virtual wxFileOffset OnSysSeek (wxFileOffset pos, wxSeekMode mode);
};