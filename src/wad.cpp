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

WDEdWADOutputStream::WDEdWADOutputStream(wxOutputStream *os, wxString wadType) : wxArchiveOutputStream(os, wxConvUTF8) {
    Init(wadType);
}

WDEdWADOutputStream::WDEdWADOutputStream(wxOutputStream &os, wxString wadType) : wxArchiveOutputStream(os, wxConvUTF8) {
    Init(wadType);
}

WDEdWADOutputStream::~WDEdWADOutputStream() {}

void WDEdWADOutputStream::Init(wxString &wadType) {
    wxASSERT(m_parent_o_stream->IsSeekable());
    wxASSERT(wadType.Len() == 4);
    m_parent_o_stream->SeekO(0, wxFromStart);
    m_parent_o_stream->WriteAll((const char *) wadType.c_str(), 4);
    m_parent_o_stream->SeekO(12, wxFromStart);
    CloseEntry();
}

bool WDEdWADOutputStream::CloseEntry() {
    if(currentEntry) currentEntry = nullptr;
    return true;
}

bool WDEdWADOutputStream::Close() {
    CloseEntry();
    
    uint32_t offset = m_parent_o_stream->TellO();

    m_parent_o_stream->SeekO(4, wxFromStart);

    uint32_t size = entries.size();
    m_parent_o_stream->WriteAll((const char *) &size, 4);
    m_parent_o_stream->WriteAll((const char *) &offset, 4);

    m_parent_o_stream->SeekO(0, wxFromEnd);

    for(std::vector<WDEdWADEntry>::iterator it = entries.begin();
            it != entries.end();
            ++it) {
                m_parent_o_stream->WriteAll((const char *) &it->GetDesc(), sizeof(WDEdWADEntryDesc));
            }
    return IsOk();
}

bool WDEdWADOutputStream::PutNextDirEntry(const wxString&, const wxDateTime&) {
    return false;
}

bool WDEdWADOutputStream::PutNextEntry(const wxString& name, const wxDateTime& dt, wxFileOffset size) {
    CloseEntry();
    if(name.Len() > 8) return false;
    WDEdWADEntry _entry;
    entries.push_back(_entry);
    currentEntry = &entries.back();
    currentEntry->SetName(name, wxPATH_UNIX);
    currentEntry->SetSize(0);
    currentEntry->SetOffset(m_parent_o_stream->TellO());
    return true;
}

bool WDEdWADOutputStream::PutNextEntry(wxArchiveEntry* _entry) {
    CloseEntry();
    WDEdWADEntry *entry = dynamic_cast<WDEdWADEntry *>(_entry);
    entries.push_back(*entry);
    currentEntry = &entries.back();
    currentEntry->SetOffset(m_parent_o_stream->TellO());
    currentEntry->SetSize(0);
    return true;
}

bool WDEdWADOutputStream::IsOk() const {
    return m_parent_o_stream->IsOk();
}

size_t WDEdWADOutputStream::GetSize() const {
    if(!currentEntry) return 0;
    wxFileOffset size = currentEntry->GetSize();
    return size == wxInvalidOffset ? 0 : size;
}

wxFileOffset WDEdWADOutputStream::GetLength() const {
    return currentEntry ? currentEntry->GetSize() : wxInvalidOffset;
}

bool WDEdWADOutputStream::IsSeekable() const {
    return true;
}

wxOutputStream &WDEdWADOutputStream::Write(const void *buffer, size_t size) {
    if(!currentEntry) {
        m_last_write = 0;
        return *this;
    }
    wxFileOffset ofs = m_parent_o_stream->TellO();
    if(ofs >= currentEntry->GetSize()) currentEntry->SetSize(currentEntry->GetSize() + size);
    m_parent_o_stream->Write(buffer, size);
    m_last_write = m_parent_o_stream->LastWrite();
    return *this;
}

size_t WDEdWADOutputStream::LastWrite () const {
    return m_last_write;
}

wxFileOffset WDEdWADOutputStream::TellO () const {
    return currentEntry ? m_parent_o_stream->TellO() - currentEntry->GetOffset() : wxInvalidOffset;
}

wxFileOffset WDEdWADOutputStream::OnSysSeek(long pos, wxSeekMode mode) {
    if(!currentEntry) return wxInvalidOffset;
    switch(mode) {
        case wxFromEnd:
            if(pos >= currentEntry->GetSize()) {
                return wxInvalidOffset;
            }
            return m_parent_o_stream->SeekO(pos, mode);
        break;
        case wxFromStart:
            if(pos >= currentEntry->GetSize()) {
                return wxInvalidOffset;
            }
            return m_parent_o_stream->SeekO(currentEntry->GetOffset() + pos, mode);
        break;
        case wxFromCurrent:
            if(TellO() + pos >= currentEntry->GetSize() || TellO() + pos < 0) {
                return wxInvalidOffset;
            }
            return m_parent_o_stream->SeekO(pos, mode);
        break;
    }
    return wxInvalidOffset;
}

wxFileOffset WDEdWADOutputStream::OnSysTell() const {
    return TellO();
}

wxFileOffset WDEdWADOutputStream::SeekO(long pos, wxSeekMode mode) {
    return OnSysSeek(pos, mode);
}

bool WDEdWADOutputStream::CopyEntry(wxArchiveEntry *entry, wxArchiveInputStream &is) {
    if(!is.OpenEntry(*entry)) return false;
    if(!is.Read(*this)) return false;
    return true;
}

bool WDEdWADOutputStream::CopyArchiveMetaData(wxArchiveInputStream &is) {
    WDEdWADInputStream *wis = dynamic_cast<WDEdWADInputStream *>(&is);
    if(wis) {
        wxFileOffset ofs = m_parent_o_stream->TellO();
        m_parent_o_stream->SeekO(0, wxFromStart);
        m_parent_o_stream->WriteAll((const char *) wis->GetWadType().c_str(), 4);
        return true;
    }
    return false;
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