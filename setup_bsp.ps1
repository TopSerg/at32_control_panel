param([string]$BspRoot = "")
$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Find-One {
    param([string]$Root,[string]$Name,[string]$Prefer = "")
    $items = @(Get-ChildItem -Path $Root -Recurse -File | Where-Object { $_.Name -ieq $Name })
    if ($items.Count -eq 0) { throw "Cannot find $Name under $Root" }
    if ($Prefer) {
        $preferred = @($items | Where-Object { $_.FullName -match $Prefer })
        if ($preferred.Count -gt 0) { return $preferred[0] }
    }
    return $items[0]
}

if ([string]::IsNullOrWhiteSpace($BspRoot)) {
    $BspRoot = Join-Path $ProjectRoot "third_party\AT32A403A_Firmware_Library"
    if (-not (Test-Path $BspRoot)) {
        New-Item -ItemType Directory -Force -Path (Split-Path $BspRoot -Parent) | Out-Null
        Write-Host "Cloning official Artery AT32A403A Firmware Library..."
        git clone --depth 1 https://gitee.com/arterytek/AT32A403A_Firmware_Library.git $BspRoot
    }
}
$BspRoot = (Resolve-Path $BspRoot).Path
$Vendor = Join-Path $ProjectRoot "vendor"
if (Test-Path $Vendor) { Remove-Item -Recurse -Force $Vendor }

@("drivers\inc","drivers\src","cmsis\core_support","cmsis\device_support","startup","template","linker") |
    ForEach-Object { New-Item -ItemType Directory -Force -Path (Join-Path $Vendor $_) | Out-Null }

$adcHeader = Find-One $BspRoot "at32a403a_adc.h" "drivers"
$adcSource = Find-One $BspRoot "at32a403a_adc.c" "drivers"
$coreCm4   = Find-One $BspRoot "core_cm4.h" "core_support"
$deviceHdr = Find-One $BspRoot "at32a403a.h" "device_support"
$systemSrc = Find-One $BspRoot "system_at32a403a.c" "device_support"
$startup   = Find-One $BspRoot "startup_at32a403a.s" "gcc"
$clockC    = Find-One $BspRoot "at32a403a_clock.c" "templates"
$clockH    = Find-One $BspRoot "at32a403a_clock.h" "templates"
$linker    = Find-One $BspRoot "AT32A403AxG_FLASH.ld" "gcc"

Copy-Item (Join-Path $adcHeader.Directory.FullName "*") (Join-Path $Vendor "drivers\inc") -Force
Copy-Item (Join-Path $adcSource.Directory.FullName "*") (Join-Path $Vendor "drivers\src") -Force
Copy-Item (Join-Path $coreCm4.Directory.FullName "*") (Join-Path $Vendor "cmsis\core_support") -Force
Copy-Item (Join-Path $deviceHdr.Directory.FullName "*") (Join-Path $Vendor "cmsis\device_support") -Force
Copy-Item $systemSrc.FullName (Join-Path $Vendor "cmsis\device_support\system_at32a403a.c") -Force
Copy-Item $startup.FullName (Join-Path $Vendor "startup\startup_at32a403a.s") -Force
Copy-Item $clockC.FullName (Join-Path $Vendor "template\at32a403a_clock.c") -Force
Copy-Item $clockH.FullName (Join-Path $Vendor "template\at32a403a_clock.h") -Force
Copy-Item $linker.FullName (Join-Path $Vendor "linker\AT32A403AxG_FLASH.ld") -Force

$required = @(
 "vendor\drivers\inc\at32a403a_adc.h",
 "vendor\drivers\src\at32a403a_adc.c",
 "vendor\drivers\src\at32a403a_dma.c",
 "vendor\drivers\src\at32a403a_gpio.c",
 "vendor\drivers\src\at32a403a_crm.c",
 "vendor\drivers\src\at32a403a_usart.c",
 "vendor\drivers\src\at32a403a_tmr.c",
 "vendor\cmsis\device_support\at32a403a.h",
 "vendor\cmsis\device_support\system_at32a403a.c",
 "vendor\startup\startup_at32a403a.s",
 "vendor\template\at32a403a_clock.c",
 "vendor\linker\AT32A403AxG_FLASH.ld"
)
foreach ($rel in $required) {
    if (-not (Test-Path (Join-Path $ProjectRoot $rel))) { throw "BSP setup incomplete: missing $rel" }
}
Write-Host ""
Write-Host "AT32A403A BSP is ready." -ForegroundColor Green
Write-Host "Open AT32_ControlPanel.sln in Visual Studio 2022 / VisualGDB."
