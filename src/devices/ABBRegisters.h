namespace ABBRegisters
{
        /*
    ===============================================================================
    ABB M1M12 MODBUS REGISTER MAP
    ===============================================================================

    Data Format:
    - Function Code : 03 (Read Holding Registers)
    - Data Type     : IEEE 754 Float
    - Register Size : 2 registers per Float (4 bytes)
    - Byte Order    : Byte Swapped (handled by ByteBuffer)

    -------------------------------------------------------------------------------
    POWER MEASUREMENTS
    -------------------------------------------------------------------------------

    ABB Address        Modbus Register       Parameter
    -------------------------------------------------------------------------------
    40101              100                   Active Power Total
    40103              102                   Active Power L1
    40105              104                   Active Power L2
    40107              106                   Active Power L3

    40109              108                   Reactive Power Total
    40111              110                   Reactive Power L1
    40113              112                   Reactive Power L2
    40115              114                   Reactive Power L3

    40117              116                   Apparent Power Total
    40119              118                   Apparent Power L1
    40121              120                   Apparent Power L2
    40123              122                   Apparent Power L3

    40125              124                   Power Factor Average
    40127              126                   Power Factor L1
    40129              128                   Power Factor L2
    40131              130                   Power Factor L3


    Block:
    Start Register : 100
    Count          : 32 registers


    -------------------------------------------------------------------------------
    VOLTAGE MEASUREMENTS
    -------------------------------------------------------------------------------

    ABB Address        Modbus Register       Parameter
    -------------------------------------------------------------------------------
    40133              132                   Voltage L-L Average
    40135              134                   Voltage L12
    40137              136                   Voltage L23
    40139              138                   Voltage L31

    40141              140                   Voltage L-N Average
    40143              142                   Voltage L1
    40145              144                   Voltage L2
    40147              146                   Voltage L3


    Block:
    Start Register : 132
    Count          : 18 registers


    -------------------------------------------------------------------------------
    CURRENT / FREQUENCY / ENERGY MEASUREMENTS
    -------------------------------------------------------------------------------

    ABB Address        Modbus Register       Parameter
    -------------------------------------------------------------------------------
    40149              148                   Current Average
    40151              150                   Current L1
    40153              152                   Current L2
    40155              154                   Current L3

    40157              156                   Frequency

    40159              158                   Energy Received Wh


    Block:
    Start Register : 148
    Count          : 12 registers


    -------------------------------------------------------------------------------
    REGISTER BLOCK SUMMARY
    -------------------------------------------------------------------------------

    Block Name              Start       Count       Bytes
    -------------------------------------------------------------------------------
    Power Block             100         32          64
    Voltage Block            132         18          36
    Current Block            148         12          24


    -------------------------------------------------------------------------------
    NOTES
    -------------------------------------------------------------------------------

    1. ABB M1M12 does not reliably accept very large combined reads.
    Individual blocks are used.

    2. Inter-request delay is required between consecutive Modbus transactions.
    Delay handling is implemented inside ModbusRTU.

    3. All floating point values use:
        FloatFormat::ByteSwapped

    4. ByteBuffer offsets are relative to the start of each block.

    Example:

    Voltage Block:
    Register 132 = Offset 0
    Register 134 = Offset 4
    Register 142 = Offset 20

    Current Block:
    Register 148 = Offset 0
    Register 156 = Offset 16

    Power Block:
    Register 100 = Offset 0
    Register 124 = Offset 48

    ===============================================================================
    */
    namespace PowerBlock
    {
        constexpr uint16_t Start = 100;
        constexpr uint16_t Count = 32;
    }
    namespace VoltageBlock
    {
        constexpr uint16_t Start = 132;
        constexpr uint16_t Count = 18;
    }

    namespace CurrentBlock
    {
        constexpr uint16_t Start = 148;
        constexpr uint16_t Count = 12;
    }
}