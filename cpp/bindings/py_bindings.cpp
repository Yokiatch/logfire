#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>

#include "mmap_reader.hpp"
#include "line_scanner.hpp"
#include "query_filter.hpp"
#include "serializer.hpp"

namespace py = pybind11;
using namespace logfire;

// Main function exposed to Python.
// Releases the GIL during C++ work so Python threads aren't blocked.
std::string query_file(
    const std::string& path,
    const std::string& pattern,
    const std::string& field_filter,
    std::size_t limit,
    std::size_t offset)
{
    py::gil_scoped_release release;

    auto file    = MmapFile::open(path);
    auto lines   = scan_lines(file.view());
    QueryOptions opts{pattern, field_filter, limit, offset};
    QueryFilter  filter{opts};
    auto matched = filter.apply(lines);
    auto result  = to_json(matched);
    file.close();
    return result;
}

PYBIND11_MODULE(_logfire, m) {
    m.doc() = "logfire — high-performance log query engine";

    m.def("query_file", &query_file,
        py::arg("path"),
        py::arg("pattern")      = "",
        py::arg("field_filter") = "",
        py::arg("limit")        = 0,
        py::arg("offset")       = 0,
        "Query a log file and return matched lines as a JSON array string."
    );
}