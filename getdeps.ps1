$IncludeDir="include"
$LibDir="lib"
$TmpFolder="temporary"
$Raylib="raylib-5.5_win64_mingw-w64"
$RaylibZip="$Raylib.zip"

function Get-Dependencies {
    Write-Host "Downloading dependencies..."
    If (-Not (Test-Path "$TmpFolder")) {
        New-Item -Path "$TmpFolder" -ItemType Directory | Out-Null
    }

    If (-Not (Test-Path "$LibDir")) {
        New-Item -Path "$LibDir" -ItemType Directory | Out-Null
    }

    If (-Not (Test-Path "$IncludeDir")) {
        New-Item -Path "$IncludeDir" -ItemType Directory | Out-Null
    }

    Invoke-WebRequest "https://github.com/raysan5/raylib/releases/download/5.5/$RaylibZip" -OutFile "$TmpFolder\$RaylibZip"
    Expand-Archive "$TmpFolder\$RaylibZip" -DestinationPath "$TmpFolder" -Force

    Move-Item "$TmpFolder\$Raylib\include\*" "$IncludeDir"
    Move-Item "$TmpFolder\$Raylib\lib\*" "$LibDir"

    Remove-Item "$TmpFolder" -Recurse
    
    Write-Host "Done downloading dependencies"
}

If (
    (-Not (Test-Path "$LibDir\libraylib.a")) -or
    (-Not (Test-Path "$LibDir\libraylibdll.a")) -or
    (-Not (Test-Path "$LibDir\raylib.dll")) -or
    (-Not (Test-Path "$IncludeDir\raylib.h")) -or
    (-Not (Test-Path "$IncludeDir\rlgl.h")) -or
    (-Not (Test-Path "$IncludeDir\raymath.h"))
) {
    Write-Host "Couldn't find a file"
    Get-Dependencies
} Else {
    Write-Host "All good to go!"
}

Write-Host "Done!"