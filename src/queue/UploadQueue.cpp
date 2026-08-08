#ifdef PLATFORM_ESP32

#include "UploadQueue.h"

UploadQueue::UploadQueue()
{
}

bool UploadQueue::Save(
    const std::string&,
    uint64_t,
    uint64_t)
{
    return false;
}

bool UploadQueue::HasPending() const
{
    return false;
}

size_t UploadQueue::Count() const
{
    return 0;
}

std::string UploadQueue::GetOldestFile() const
{
    return "";
}

std::string UploadQueue::Load(
    const std::string& filename) const
{
    (void)filename;
    return "";
}

bool UploadQueue::Remove(
    const std::string&)
{
    return true;
}

#else

#include "UploadQueue.h"

#include "../utils/Logger.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

bool UploadQueue::HasPending() const
{
    return Count() > 0;
}

size_t UploadQueue::Count() const
{
    size_t count = 0;

    for (const auto& entry : fs::directory_iterator(m_queueFolder))
    {
        if (entry.is_regular_file())
            ++count;
    }

    return count;
}

std::string UploadQueue::GetOldestFile() const
{
    std::vector<std::string> files;

    for (const auto& entry : fs::directory_iterator(m_queueFolder))
    {
        if (entry.is_regular_file())
            files.push_back(entry.path().string());
    }

    if (files.empty())
        return "";

    std::sort(files.begin(), files.end());

    return files.front();
}

std::string UploadQueue::Load(
    const std::string& filename) const
{
    std::ifstream file(filename);

    if (!file.is_open())
        return "";

    return std::string(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());
}

UploadQueue::UploadQueue()
{
    m_queueFolder = "queue";

    if (!fs::exists(m_queueFolder))
    {
        fs::create_directories(m_queueFolder);

        Logger::Info(
            "Created queue folder.");
    }
}

bool UploadQueue::Remove(
    const std::string& filename)
{
    if (fs::remove(filename))
    {
        Logger::Info(
            "Removed queued file: " + filename);

        return true;
    }

    return false;
}

bool UploadQueue::Save(
    const std::string& json,
    uint64_t timestamp,
    uint64_t sequence)
{
    std::ostringstream filename;

    filename
        << m_queueFolder
        << "/"
        << timestamp
        << "_"
        << std::setw(6)
        << std::setfill('0')
        << sequence
        << ".json";

    std::ofstream file(filename.str());

    if (!file.is_open())
    {
        Logger::Error(
            "Unable to create queue file.");

        return false;
    }

    file << json;

    file.close();

    Logger::Info(
        "Queued upload: " +
        filename.str());

    return true;
}
#endif