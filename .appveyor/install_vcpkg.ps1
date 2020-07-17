$ErrorActionPreference = "Stop"

if ($env:VCPKG -eq '1' -and -not (Test-Path 'C:\build-cache\deps')) {
    [void](New-Item 'C:\build-cache\deps' -ItemType 'directory')

    vcpkg install "brotli:$env:ARCH-windows"
    if (-not $?) {
        throw "installing failed with errorlevel $LastExitCode"
    }

    Copy-Item "C:\tools\vcpkg\installed\$env:ARCH-windows\bin" "C:\build-cache\deps" -Recurse -Force
    Copy-Item "C:\tools\vcpkg\installed\$env:ARCH-windows\include" "C:\build-cache\deps" -Recurse -Force
    Copy-Item "C:\tools\vcpkg\installed\$env:ARCH-windows\lib" "C:\build-cache\deps" -Recurse -Force
}
