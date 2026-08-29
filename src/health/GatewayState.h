#pragma once

#include <cstdint>
#include <string>


class GatewayState
{

public:

    bool Load(
        const std::string& filename);


    bool Save(
        const std::string& filename);


    uint64_t NextSequence();


    uint64_t GetSequence() const;

    void SetSequence(uint64_t sequence);


    void SetGatewayId(
        const std::string& id);


private:

    uint64_t m_sequence = 0;

    std::string m_gatewayId;

};
