#pragma once

#include <exception>

namespace kiwix {

class KiwixError : public std::exception {
public:
    const char* what() const noexcept override {
        return "Kiwix error";
    }
};

class FileNotFound : public KiwixError {
public:
    const char* what() const noexcept override {
        return "ZIM file not found";
    }
};

class PermissionDenied : public KiwixError {
public:
    const char* what() const noexcept override {
        return "Permission denied while accessing ZIM file";
    }
};

class InvalidZim : public KiwixError {
public:
    const char* what() const noexcept override {
        return "Invalid ZIM file";
    }
};

class LibraryNotWritable : public KiwixError {
public:
    const char* what() const noexcept override {
        return "Library file is not writable";
    }
};

} // namespace kiwix
