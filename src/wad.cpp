#include "wad.h"

WDEdWADInputStream::WDEdWADInputStream(wxInputStream *is) : wxArchiveInputStream(is, wxConvUTF8) {
    Init();
}

WDEdWADInputStream::WDEdWADInputStream(wxInputStream &is) : wxArchiveInputStream(is, wxConvUTF8) {
    Init();
}

WDEdWADInputStream::~WDEdWADInputStream() {
    if(m_current_entry) delete m_current_entry;
}

void WDEdWADInputStream::Init() {
    if(!m_parent_i_stream->IsSeekable()) {
        wxPrintf("Error: trying to open wad from non-seekable stream\n");
        wxExit();
    }
    m_parent_i_stream->SeekI(0, wxFromStart);
    m_parent_i_stream->Read(&m_header_desc, sizeof(m_header_desc));
    m_next_directory_item = 0;

    CloseEntry();
}

bool WDEdWADInputStream::IsSeekable() const {
    return true;
}

bool WDEdWADInputStream::IsOk() const {
    return m_parent_i_stream->IsOk();
}

wxFileOffset WDEdWADInputStream::GetLength() const {
    return m_current_entry ? m_current_entry->GetSize() : wxInvalidOffset;
}

size_t WDEdWADInputStream::GetSize() const {
    return GetLength() == wxInvalidOffset ? 0 : (size_t)GetLength();
}

bool WDEdWADInputStream::CanRead() const {
    return m_current_entry && !Eof() && IsOk();
}

bool WDEdWADInputStream::Eof() const {
    return m_current_entry ? TellI() >= m_current_entry->GetSize() : true;
}

wxFileOffset WDEdWADInputStream::TellI() const {
    return m_current_entry ? m_parent_i_stream->TellI() - m_current_entry->GetOffset() : wxInvalidOffset;
}

wxFileOffset WDEdWADInputStream::SeekI(wxFileOffset pos, wxSeekMode mode) {
    if(!m_current_entry) {
        return wxInvalidOffset;
    }
    switch(mode) {
        case wxFromStart:
            if(pos >= m_current_entry->GetSize()) return wxInvalidOffset;
            return m_parent_i_stream->SeekI(pos + m_current_entry->GetOffset(), wxFromStart);
        case wxFromEnd:
            if(pos >= m_current_entry->GetSize()) return wxInvalidOffset;
            return m_parent_i_stream->SeekI(m_current_entry->GetOffset() + m_current_entry->GetSize() - 1 - pos, wxFromStart);
        case wxFromCurrent:
            if(pos + TellI() >= m_current_entry->GetSize() || pos + TellI() < 0) return wxInvalidOffset;
            return m_parent_i_stream->SeekI(pos, wxFromCurrent);
    }
    return wxInvalidOffset;
}
    
char WDEdWADInputStream::Peek() {
    if(!m_current_entry) return 0;
    if(!Eof() && IsOk()) return m_parent_i_stream->Peek();
    return 0;
}

wxInputStream &WDEdWADInputStream::Read(void* buffer, size_t size) {
    m_last_read = OnSysRead(buffer, size);
    return *this;
}

size_t WDEdWADInputStream::OnSysRead(void *buffer, size_t size) {
    m_last_read = 0;
    if(!m_current_entry) return 0;
    if(!Eof() && IsOk()) {
        if(size + TellI() >= m_current_entry->GetSize()) {
            m_parent_i_stream->Read(buffer, m_current_entry->GetSize() - TellI());
            return m_parent_i_stream->LastRead();
        } else {
            m_parent_i_stream->Read(buffer, size);
            return m_parent_i_stream->LastRead();
        }
    }
    return 0;
}

size_t WDEdWADInputStream::LastRead() const {
    return m_last_read;
}

wxString WDEdWADInputStream::GetWadType() const {
    return wxString(m_header_desc.desc, 4);
}

bool WDEdWADInputStream::CloseEntry() {
    if(m_current_entry) delete m_current_entry;
    m_current_entry = nullptr;
    return m_parent_i_stream->SeekI(m_header_desc.dirPointer + m_next_directory_item * sizeof(WDEdWADEntryDesc), wxFromStart) != wxInvalidOffset;
}

bool WDEdWADInputStream::OpenEntry(wxArchiveEntry &entry) {
    WDEdWADEntry &wentry = dynamic_cast<WDEdWADEntry &>(entry);
    m_current_entry = &wentry;
    return m_parent_i_stream->SeekI(wentry.GetOffset(), wxFromStart) != wxInvalidOffset;
}

wxArchiveEntry *WDEdWADInputStream::DoGetNextEntry() {
    if(m_current_entry) CloseEntry();
    if(m_next_directory_item >= m_header_desc.numEntries) return nullptr;
    WDEdWADEntry *entry = new WDEdWADEntry();
    m_parent_i_stream->Read(&entry->desc, sizeof(WDEdWADEntryDesc));
    m_next_directory_item++;
    OpenEntry(*entry);
    return entry;
}


WDEdWADEntry::WDEdWADEntry() : wxArchiveEntry::wxArchiveEntry() {}

wxDateTime WDEdWADEntry::GetDateTime() const {
    return wxInvalidDateTime;
}

wxFileOffset WDEdWADEntry::GetSize() const {
    return desc.lumpSize;
}

wxFileOffset WDEdWADEntry::GetOffset() const {
    return desc.lumpPointer;
}

bool WDEdWADEntry::IsDir() const {
    return false;
}

bool WDEdWADEntry::IsReadOnly() const {
    return false;
}

wxString WDEdWADEntry::GetInternalName() const {
    char name[9] = {0};
    for(int i = 0; i < 8; i++) {
        name[i] = desc.name[i];
    }
    return name;
}

wxPathFormat WDEdWADEntry::GetInternalFormat() const {
    return wxPATH_UNIX;
}

wxString WDEdWADEntry::GetName(wxPathFormat) const {
    return GetInternalName();
}

void WDEdWADEntry::SetDateTime(const wxDateTime&) {}

void WDEdWADEntry::SetSize(wxFileOffset size) {
    desc.lumpSize = size;
}

void WDEdWADEntry::SetIsDir(bool) {}


void WDEdWADEntry::SetIsReadOnly(bool) {}

void WDEdWADEntry::SetName(const wxString& name, wxPathFormat) {
    memset(&desc.name, 0, 8);
    for(int i = 0; i < 8 && i < name.Len(); i++) {
        desc.name[i] = name[i];
    }
}

void WDEdWADEntry::SetOffset(long offset) {
    // Okay. Use at your own risk.
    desc.lumpPointer = offset;
}

wxArchiveEntry* WDEdWADEntry::DoClone() const {
    WDEdWADEntry *newentry = new WDEdWADEntry;
    newentry->desc = desc;
    return newentry;
}