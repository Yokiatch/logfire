
#include "mmap_reader.hpp"
#include <stdexcept>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
  #include <unistd.h>
#endif

namespace logfire {

MmapFile MmapFile::open(const std::string& path) {
    MmapFile f;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Cannot open: " + path);

    LARGE_INTEGER size{};
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        throw std::runtime_error("Cannot get file size: " + path);
    }
    f.size = static_cast<std::size_t>(size.QuadPart);

    if (f.size == 0) {
        CloseHandle(hFile);
        f.data = nullptr;
        return f;
    }

    HANDLE hMap = CreateFileMappingA(
        hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
    CloseHandle(hFile);   // mapping keeps file alive
    if (!hMap)
        throw std::runtime_error("CreateFileMapping failed: " + path);

    f.data = static_cast<const char*>(
        MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
    f.handle = hMap;      // store so we can unmap later

    if (!f.data) {
        CloseHandle(hMap);
        throw std::runtime_error("MapViewOfFile failed: " + path);
    }

#else
    f.fd = ::open(path.c_str(), O_RDONLY);
    if (f.fd < 0)
        throw std::runtime_error("Cannot open: " + path);

    struct stat st{};
    if (fstat(f.fd, &st) < 0)
        throw std::runtime_error("fstat failed");
    f.size = static_cast<std::size_t>(st.st_size);

    if (f.size == 0) { f.data = nullptr; return f; }

    f.data = static_cast<const char*>(
        mmap(nullptr, f.size, PROT_READ,
             MAP_PRIVATE | MAP_POPULATE, f.fd, 0));
    if (f.data == MAP_FAILED)
        throw std::runtime_error("mmap failed");

    madvise(const_cast<char*>(f.data), f.size, MADV_SEQUENTIAL);
#endif

    return f;
}

void MmapFile::close() {
#ifdef _WIN32
    if (data) UnmapViewOfFile(data);
    if (handle) CloseHandle(static_cast<HANDLE>(handle));
    handle = nullptr;
#else
    if (data && data != MAP_FAILED)
        munmap(const_cast<char*>(data), size);
    if (fd >= 0) ::close(fd);
    fd = -1;
#endif
    data = nullptr;
    size = 0;
}

} // namespace logfire
