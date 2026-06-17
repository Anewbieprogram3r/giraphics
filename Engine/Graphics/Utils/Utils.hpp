#pragma once

#include <fstream>      // For file stream operations (std::ifstream)
#include <vector>       // For dynamic array (std::vector)
#include <string>       // For string handling (std::string)
#include <stdexcept>    // For standard exceptions (std::runtime_error)
#include <functional>   // For function objects, function wrappers, and binders (std::function)
#include <memory>       // For smart pointers and memory management (std::unique_ptr, std::shared_ptr)

namespace giraphics {
// NonCopyable class: prevents copying
class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

// NonMovable class: prevents moving
class NonMovable {
protected:
    NonMovable() = default;
    ~NonMovable() = default;

public:
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

// Singleton template class: makes a class a singleton
template <typename T>
class Singleton : public NonCopyable, public NonMovable {
public:
    // Static method to access the single instance with variadic arguments
    template <typename... Args>
    static T& getInstance(Args&&... args) {
        if (!instance) {
            instance.reset(new T(std::forward<Args>(args)...));
        }
        return *instance;
    }

    // Static method to reinitialize the singleton instance with new parameters
    template <typename... Args>
    static void reinitialize(Args&&... args) {
        if (instance) {
            instance.reset();
        }
        instance.reset(new T(std::forward<Args>(args)...));
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    static std::unique_ptr<T> instance;
};

// Define the static member
template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

static std::vector<char> readFile(const std::string& filepath) {
    // Open the file in binary mode at the end to determine its size
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    // Check if the file was opened successfully
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Get the size of the file
    std::streamsize fileSize = file.tellg();

    // Create a vector with enough space to hold the file content
    std::vector<char> buffer(fileSize);

    // Go back to the beginning of the file and read its content
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), fileSize);

    // Close the file
    file.close();

    return buffer;
}

// Reference - https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
};


// Custom assert macro with message
#define ASSERT_WITH_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: (" #cond "), function " << __FUNCTION__ \
                      << ", file " << __FILE__ << ", line " << __LINE__ << ".\n" \
                      << "Message: " << msg << std::endl; \
            std::abort(); \
        } \
    } while (false)
}  // namespace giraphics