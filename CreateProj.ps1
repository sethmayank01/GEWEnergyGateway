# ==========================================
# GEW Energy Gateway Project Generator
# ==========================================

$Project = "GEWEnergyGateway"

Write-Host ""
Write-Host "Creating $Project ..."
Write-Host ""

New-Item -ItemType Directory -Force -Path $Project | Out-Null

$folders = @(
"main",
"src",
"src/config",
"src/protocol",
"src/meters",
"src/models",
"src/json",
"src/network",
"src/cloud",
"src/queue",
"src/hal",
"src/utils",
"src/platform",
"src/platform/windows",
"src/platform/esp32",
"docs",
"test",
"thirdparty",
"build"
)

foreach($folder in $folders)
{
    New-Item -ItemType Directory -Force -Path "$Project/$folder" | Out-Null
}

$files = @(
".gitignore",
"CMakeLists.txt",
"README.md",

"main/main.cpp",

"src/config/Config.h",
"src/config/Config.cpp",

"src/protocol/Modbus.h",
"src/protocol/Modbus.cpp",

"src/meters/IMeter.h",
"src/meters/ABBM1M12.h",
"src/meters/ABBM1M12.cpp",

"src/models/MeterReading.h",
"src/models/GatewayInfo.h",
"src/models/Configuration.h",

"src/json/JsonBuilder.h",
"src/json/JsonBuilder.cpp",

"src/network/IHttpClient.h",
"src/network/HttpClient.h",
"src/network/HttpClient.cpp",

"src/cloud/Uploader.h",
"src/cloud/Uploader.cpp",

"src/queue/UploadQueue.h",
"src/queue/UploadQueue.cpp",

"src/hal/ISerial.h",
"src/hal/INetwork.h",
"src/hal/IStorage.h",
"src/hal/IClock.h",

"src/utils/Logger.h",
"src/utils/Logger.cpp",

"src/platform/windows/SerialWin.h",
"src/platform/windows/SerialWin.cpp",
"src/platform/windows/HttpWin.h",
"src/platform/windows/HttpWin.cpp",

"src/platform/esp32/SerialESP.h",
"src/platform/esp32/SerialESP.cpp",
"src/platform/esp32/HttpESP.h",
"src/platform/esp32/HttpESP.cpp"
)

foreach($file in $files)
{
    New-Item -ItemType File -Force -Path "$Project/$file" | Out-Null
}

Write-Host ""
Write-Host "Project created successfully!"
Write-Host ""
Write-Host "Location:"
Write-Host (Resolve-Path $Project)