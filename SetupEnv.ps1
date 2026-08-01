# ============================================================
# GEW Energy Gateway Development Environment Setup
# ============================================================

Write-Host ""
Write-Host "=============================================="
Write-Host " GEW Energy Gateway Development Setup"
Write-Host "=============================================="
Write-Host ""

#-------------------------------------------------------------
# Check Winget
#-------------------------------------------------------------

if (!(Get-Command winget -ErrorAction SilentlyContinue))
{
    Write-Host "Winget not found."
    Write-Host "Please install App Installer from Microsoft Store."
    exit
}

#-------------------------------------------------------------
# Install Git
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing Git..."

winget install --id Git.Git -e --silent

#-------------------------------------------------------------
# Install VS Code
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing VS Code..."

winget install --id Microsoft.VisualStudioCode -e --silent

#-------------------------------------------------------------
# Install CMake
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing CMake..."

winget install --id Kitware.CMake -e --silent

#-------------------------------------------------------------
# Install Ninja
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing Ninja..."

winget install --id Ninja-build.Ninja -e --silent

#-------------------------------------------------------------
# Install Doxygen
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing Doxygen..."

winget install --id DimitriVanHeesch.Doxygen -e --silent

#-------------------------------------------------------------
# Install Graphviz
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing Graphviz..."

winget install --id Graphviz.Graphviz -e --silent

#-------------------------------------------------------------
# Install VS Build Tools
#-------------------------------------------------------------

Write-Host ""
Write-Host "------------------------------------------------"
Write-Host "Visual Studio Build Tools"
Write-Host "------------------------------------------------"
Write-Host ""

Write-Host "If Visual Studio Build Tools are NOT installed,"
Write-Host "download them from:"
Write-Host ""
Write-Host "https://visualstudio.microsoft.com/downloads/"
Write-Host ""
Write-Host "Install ONLY:"
Write-Host ""
Write-Host "Desktop Development with C++"
Write-Host ""
Pause

#-------------------------------------------------------------
# VS Code Extensions
#-------------------------------------------------------------

Write-Host ""
Write-Host "Installing VS Code Extensions..."

$extensions = @(

"ms-vscode.cpptools"

"ms-vscode.cmake-tools"

"twxs.cmake"

"eamodio.gitlens"

"mhutchie.git-graph"

"xaver.clang-format"

"streetsidesoftware.code-spell-checker"

"ms-azuretools.vscode-docker"

"redhat.vscode-yaml"

"ms-vscode.hexeditor"

)

foreach($ext in $extensions)
{
    code --install-extension $ext --force
}

#-------------------------------------------------------------
# Git Configuration
#-------------------------------------------------------------

Write-Host ""
Write-Host "Configuring Git..."

$name = Read-Host "Git Username"

$email = Read-Host "Git Email"

git config --global user.name "$name"

git config --global user.email "$email"

#-------------------------------------------------------------
# Verify
#-------------------------------------------------------------

Write-Host ""
Write-Host "========================================"
Write-Host "Versions"
Write-Host "========================================"

git --version

cmake --version

code --version

Write-Host ""
Write-Host "========================================"
Write-Host "Setup Complete"
Write-Host "========================================"

Pause