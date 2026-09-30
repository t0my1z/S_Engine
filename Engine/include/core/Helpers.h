#pragma once

#include <filesystem>
#include <chrono>

namespace SE
{
// Returns empty path on failure.
std::filesystem::path GetBinPath();
std::filesystem::path GetInstallDirectory();
std::filesystem::path GetResourcesDirectory();
std::vector<char> ReadFile(const std::string& filename);


class Timer 
{
public:
	void Start();
	float Stop();

private:
	bool m_running = false;
	std::chrono::time_point<std::chrono::steady_clock> m_start_time;
};


}