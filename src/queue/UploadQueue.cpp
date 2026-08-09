#include "UploadQueue.h"

#include "../utils/Logger.h"

#ifdef PLATFORM_ESP32

#include <LittleFS.h>
#include <Arduino.h>

#else

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace fs = std::filesystem;

#endif

#include <sstream>
#include <iomanip>
#include <algorithm>

UploadQueue::UploadQueue()
{
    m_queueFolder = QUEUE_DIR;

#ifdef PLATFORM_ESP32

    LittleFS.begin(true);

    if (!LittleFS.exists(m_queueFolder.c_str()))
    {
        LittleFS.mkdir(m_queueFolder.c_str());

        Logger::Info(
            "Created queue folder.");
    }

#else

    if (!fs::exists(m_queueFolder))
    {
        fs::create_directories(m_queueFolder);

        Logger::Info(
            "Created queue folder.");
    }

#endif
}

bool UploadQueue::HasPending() const
{
    return Count() > 0;
}

size_t UploadQueue::Count() const
{
#ifdef PLATFORM_ESP32

    File dir =
        LittleFS.open(m_queueFolder.c_str());

    if (!dir || !dir.isDirectory())
        return 0;

    size_t count = 0;

    File file =
        dir.openNextFile();

    while (file)
    {
        ++count;

        file = dir.openNextFile();
    }

    return count;

#else

    size_t count = 0;

    for (const auto& entry :
         fs::directory_iterator(m_queueFolder))
    {
        if (entry.is_regular_file())
            ++count;
    }

    return count;

#endif
}

std::string UploadQueue::GetOldestFile() const
{
    std::vector<std::string> files;

#ifdef PLATFORM_ESP32

    File dir =
        LittleFS.open(m_queueFolder.c_str());

    if (!dir || !dir.isDirectory())
        return "";

    File file =
        dir.openNextFile();

    while (file)
    {
        files.push_back(
            std::string(file.path()));

        file = dir.openNextFile();
    }

#else

    for (const auto& entry :
         fs::directory_iterator(m_queueFolder))
    {
        if (entry.is_regular_file())
            files.push_back(entry.path().string());
    }

#endif

    if (files.empty())
        return "";

    std::sort(
        files.begin(),
        files.end());

    return files.front();
}

std::string UploadQueue::Load(
    const std::string& filename) const
{
#ifdef PLATFORM_ESP32

    File file =
        LittleFS.open(
            filename.c_str(),
            "r");

    if (!file)
        return "";

    std::string json;

    while (file.available())
    {
        json +=
            static_cast<char>(
                file.read());
    }

    file.close();

    return json;

#else

    std::ifstream file(filename);

    if (!file.is_open())
        return "";

    return std::string(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

#endif
}

bool UploadQueue::Remove(
    const std::string& filename)
{
#ifdef PLATFORM_ESP32

    if (LittleFS.remove(filename.c_str()))
    {
        Logger::Info(
            "Removed queued file: " +
            filename);

        return true;
    }

    return false;

#else

    if (fs::remove(filename))
    {
        Logger::Info(
            "Removed queued file: " +
            filename);

        return true;
    }

    return false;

#endif
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

#ifdef PLATFORM_ESP32

    File file =
        LittleFS.open(
            filename.str().c_str(),
            "w");

    if (!file)
    {
        Logger::Error(
            "Unable to create queue file.");

        return false;
    }

    file.print(json.c_str());

    file.close();

#else

    std::ofstream file(
        filename.str());

    if (!file.is_open())
    {
        Logger::Error(
            "Unable to create queue file.");

        return false;
    }

    file << json;

    file.close();

#endif

    Logger::Info(
        "Queued upload: " +
        filename.str());

    return true;
}