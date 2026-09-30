#include "core/Helpers.h"

#include <fstream>
#include <iostream>

#if defined(_WIN32)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <limits.h>
#  include <vector>
#else
#  include <unistd.h>
#  include <limits.h>
#  include <vector>
#endif

namespace SE
{

namespace fs = std::filesystem;

std::filesystem::path GetBinPath()
{
    #if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
    if (len == 0) return {};
    return fs::path(buf);
    #elif defined(__APPLE__)
    uint32_t size = 0;
    // first call gets required buffer size
    _NSGetExecutablePath(NULL, &size);
    std::vector<char> buf(size);
    if (_NSGetExecutablePath(buf.data(), &size) != 0) return {};
    // canonicalize to resolve symlinks
    return fs::weakly_canonical(fs::path(buf.data()));
    #else // Linux / other Unix
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) return {};
    buf[len] = '\0';
    return fs::weakly_canonical(fs::path(buf));
    #endif
}

std::filesystem::path GetInstallDirectory()
{
    fs::path p = GetBinPath(); // install/bin/SEngineApp / build/debug/App/SEngineApp (executable)
    p = p.empty() ? fs::path() : p.parent_path(); // install/bin / build/debug/App
    return p.empty() ? fs::path() : p.parent_path(); // install / build/debug
}

std::filesystem::path GetResourcesDirectory()
{
    fs::path installDir = GetInstallDirectory();
    installDir /= "resources";
    return installDir;
}

std::vector<char> ReadFile(const std::string& filename) 
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary); //ate makes us start at end of file

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    std::vector<char> buffer(file.tellg()); //since we are at the end, we can get the num of chars, and use that as size
    file.seekg(0, std::ios::beg); //go back to beginning of file
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size())); //read from start to end

    file.close();

    return buffer;
}



void Timer::Start()
{
    if (m_running) return;

    m_running = true;
	m_start_time = std::chrono::steady_clock::now();
}

float Timer::Stop()
{
    if (!m_running) return 0.0f;

    m_running = false;
    auto stopTime = std::chrono::steady_clock::now();
    float timerMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>
		(stopTime - m_start_time).count() / 1000.0f;

    return timerMilliSeconds;
}

}