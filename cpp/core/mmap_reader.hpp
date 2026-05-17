#pragma once
#include <string>
#include <string_view>
#include <cstddef>

namespace logfire {

struct MmapFile {
    const char* data   = nullptr;
    std::size_t size   = 0;

#ifdef _WIN32
    void* handle = nullptr;   // HANDLE to the file mapping
#else
    int fd = -1;
#endif

    static MmapFile open(const std::string& path);
    void close();
    std::string_view view() const { return {data, size}; }
};

} // namespace logfire
