cmake_minimum_required(VERSION 3.20)

project(ModbusScanner)

add_executable(ModbusScanner
    main.cpp

    ../../src/config/Configuration.cpp

    ../../src/platform/windows/SerialPortWin.cpp

    ../../src/protocol/CRC16.cpp
    ../../src/protocol/ModbusFrame.cpp
    ../../src/protocol/ModbusRTU.cpp

    ../../src/utils/Logger.cpp
    ../../src/utils/ByteBuffer.cpp
    ../../src/utils/HexDump.cpp
)

target_include_directories(ModbusScanner PRIVATE

    ../../src
)