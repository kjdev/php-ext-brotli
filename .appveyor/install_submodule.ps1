$ErrorActionPreference = "Stop"

if ($env:VCPKG -eq '0') {
    $command = "git"
    $args = "submodule update --init --recursive"
    $process = Start-Process $command -ArgumentList $args -Wait -NoNewWindow -PassThru
    if ($process.ExitCode -ne 0) {
        Write-Error -Category InvalidResult -Message "$command exits with $($process.ExitCode)"
    }
}
