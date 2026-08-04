#include "GatewayState.h"

#include "../utils/Logger.h"

#include "../../thirdparty/nlohmann/json.hpp"

#include <fstream>
#include <filesystem>


using json = nlohmann::json;

namespace fs = std::filesystem;



bool GatewayState::Load(
    const std::string& filename)
{

    std::ifstream file(filename);


    if(!file.is_open())
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


        Logger::Info(
            "Gateway state loaded.");

        Logger::Info(
            "Last sequence : "
            + std::to_string(m_sequence));


        return true;

    }
    catch(...)
    {
        Logger::Error(
            "Invalid gateway state file.");

        m_sequence = 0;

        return false;
    }
}



bool GatewayState::Save(
    const std::string& filename)
{

    try
    {

        fs::path path(filename);


        if(!fs::exists(path.parent_path()))
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


        if(!file.is_open())
        {
            Logger::Error(
                "Unable to save gateway state.");

            return false;
        }


        file << j.dump(4);


        return true;

    }
    catch(...)
    {
        Logger::Error(
            "Exception while saving gateway state.");

        return false;
    }

}

void GatewayState::SetGatewayId(
    const std::string& id)
{
    m_gatewayId = id;
}


uint64_t GatewayState::NextSequence()
{

    m_sequence++;

    return m_sequence;

}



uint64_t GatewayState::GetSequence() const
{
    return m_sequence;
}