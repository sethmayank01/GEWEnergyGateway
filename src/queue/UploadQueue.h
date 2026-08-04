#pragma once

#include <string>
#include <cstdint>

class UploadQueue
{
public:

    UploadQueue();

    bool Save(
        const std::string& json,
        uint64_t timestamp,
        uint64_t sequence);

    bool HasPending() const;

    std::string GetOldestFile() const;

    std::string Load(
        const std::string& filename) const;

    bool Remove(
        const std::string& filename);

    size_t Count() const;

private:

    std::string m_queueFolder;
};