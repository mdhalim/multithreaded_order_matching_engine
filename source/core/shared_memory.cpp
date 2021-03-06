#include <core/compiler/unused.h>
#include "shared_memory.h"

#ifdef __linux__
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cstring>
#include <stdlib.h>
using namespace std;

namespace core
{

SharedMemory::SharedMemory() : m_buffer(nullptr), m_size(0), m_writtenSize(0)
{
#ifdef __linux__
    m_fileDescriptor = 0;
#elif _WIN32
    m_handle = INVALID_HANDLE_VALUE;
    m_fileHandle = INVALID_HANDLE_VALUE;
#endif
}

SharedMemory::~SharedMemory()
{
    close();
}

void SharedMemory::close()
{
#ifdef __linux__
    if( m_buffer != nullptr)
    {
        munmap (m_buffer, m_size);
    }

    ::close(m_fileDescriptor);
#elif _WIN32
    if (m_buffer)
    {
        UnmapViewOfFile(m_buffer);
    }

    if (m_handle)
    {
        CloseHandle(m_handle);
    }

    if (m_fileHandle)
    {
        CloseHandle(m_fileHandle);
    }

    m_handle = INVALID_HANDLE_VALUE;
    m_fileHandle = INVALID_HANDLE_VALUE;
#endif
    m_size = 0;
    m_buffer = nullptr;
    m_writtenSize = 0;
}

bool SharedMemory::open(string name, size_t maxSize, bool createFile, bool ipc, bool buffered)
{
    bool ret = true;
    m_size = core::VirtualMemory::adjustSizeToPageSize(maxSize);
#ifdef __linux__
    UNUSED(buffered); // Not using O_DIRECT due to alignment requirements
    UNUSED(ipc);
    // Prepare a file large enough to hold an unsigned integer.
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    m_fileDescriptor = ::open (name.c_str(), flags, S_IRUSR | S_IWUSR);

    if(lseek(m_fileDescriptor, m_size, SEEK_SET) != -1 )
    {
         // Something needs to be written at the end of the file to have the file actually have the new size.
        if( ::write(m_fileDescriptor, "", 1) == 1)
        {
            int mmapFlags = MAP_SHARED;
            // Create memory mapping
            m_buffer = static_cast<char*>(mmap (0, m_size, PROT_WRITE, mmapFlags, m_fileDescriptor, 0));
        }
        else
        {
            ret = false;
        }
    }
    else
    {
        ret = false;
    }
#elif _WIN32
    if (createFile)
    {
        DWORD access = 0;

        if (ipc)
        {
            access = FILE_SHARE_READ | FILE_SHARE_WRITE;
        }

        DWORD flags = 0;

        if (buffered == false)
        {
            flags = FILE_FLAG_NO_BUFFERING;
        }

        m_fileHandle = CreateFile(name.c_str(),                // name of the write
                GENERIC_WRITE | GENERIC_READ,
                access,
                NULL,                   // default security
                OPEN_ALWAYS,             // create new file only
                flags,  // normal file
                NULL);                  // no attr. template
    }

    m_handle = CreateFileMapping(
            m_fileHandle,   // use paging file
            NULL,                   // default security
            PAGE_READWRITE,         // read/write access
            0,                      // maximum object size (high-order DWORD)
            m_size,                // maximum object size (low-order DWORD)
            name.c_str());          // name of mapping object

    if (m_handle == nullptr)
    {
        ret = false;
    }
    else
    {
        m_buffer = (LPTSTR)MapViewOfFile(m_handle,   // handle to map object
                FILE_MAP_ALL_ACCESS, // read/write permission
                0,
                0,
                0);

        if (m_buffer == nullptr)
        {
            ret = false;
        }
    }
#endif
    if (ret)
    {
        std::memset(m_buffer, 0, m_size);
    }
    return ret;
}

void SharedMemory::write(void* buffer, size_t size)
{
#ifdef __linux__
    std::memcpy(m_buffer + (m_writtenSize % m_size), buffer, size);
#elif _WIN32
    CopyMemory(m_buffer + (m_writtenSize % m_size), buffer, size);
#endif
    m_writtenSize += size;
}

} // namespace