#include "GatewayState.h"

#include "../utils/Logger.h"

#ifdef PLATFORM_ESP32

#include <ArduinoJson.h>
#include <LittleFS.h>

#else

#include "../../thirdparty/nlohmann/json.hpp"
#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

#endif

bool GatewayState::Load(
    const std::string& filename)
{
#ifdef PLATFORM_ESP32

    if (!LittleFS.begin(true))
    {
        Logger::Error("LittleFS mount failed.");
        return false;
    }

    File file =
        LittleFS.open(filename.c_str(), "r");

    if (!file)
    {
        Logger::Info(
            "No gateway state found. Starting fresh.");

        m_sequence = 0;
        return true;
    }

    JsonDocument j;

    if (deserializeJson(j, file))
    {
        file.close();

        Logger::Error(
            "Invalid gateway state file.");

        m_sequence = 0;

        return false;
    }

    file.close();

    m_sequence =
        j["sequence"] | 0ULL;

    m_gatewayId =
        std::string(
            j["gatewayId"] | "");

#else

    std::ifstream file(filename);

    if (!file.is_open())
    {
        Logger::Info(
            "No gateway state found. Starting fresh.");

        m_sequence = 0;

        return true;
    }

    try
    {
        json j;

        file >> j;

        m_sequence =
            j.value(
                "sequence",
                0ULL);

        m_gatewayId =
            j.value(
                "gatewayId",
                "");
    }
    catch (...)
    {
        Logger::Error(
            "Invalid gateway state file.");

        m_sequence = 0;

        return false;
    }

#endif

    Logger::Info(
        "Gateway state loaded.");

    Logger::Info(
        "Last sequence : " +
        std::to_string(m_sequence));

    return true;
}

bool GatewayState::Save(
    const std::string& filename)
{

#ifdef PLATFORM_ESP32

    if (!LittleFS.begin(true))
    {
        Logger::Error(
            "LittleFS mount failed.");

        return false;
    }

    File file =
        LittleFS.open(
            filename.c_str(),
            "w");

    if (!file)
    {
        Logger::Error(
            "Unable to save gateway state.");

        return false;
    }

    JsonDocument j;

    j["gatewayId"] =
        m_gatewayId;

    j["sequence"] =
        m_sequence;

    serializeJsonPretty(
        j,
        file);

    file.close();

    return true;

#else

    try
    {
        fs::path path(filename);

        if (!fs::exists(path.parent_path()))
        {
            fs::create_directories(
                path.parent_path());
        }

        json j;

        j["gatewayId"] =
            m_gatewayId;

        j["sequence"] =
            m_sequence;

        std::ofstream file(filename);

        if (!file.is_open())
        {
            Logger::Error(
                "Unable to save gateway state.");

            return false;
        }

        file << j.dump(4);

        return true;
    }
    catch (...)
    {
        Logger::Error(
            "Exception while saving gateway state.");

        return false;
    }

#endif
}

void GatewayState::SetGatewayId(
    const std::string& id)
{
    m_gatewayId = id;
}

uint64_t GatewayState::NextSequence()
{
    ++m_sequence;
    return m_sequence;
}

uint64_t GatewayState::GetSequence() const
{
    return m_sequence;
}