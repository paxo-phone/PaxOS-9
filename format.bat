@echo off
setlocal enabledelayedexpansion

REM ClangFormat script for Windows
REM This script formats all C/C++ source files in the project using clang-format

set "DRY_RUN=false"
set "CHECK_ONLY=false"

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse
if "%~1"=="--dry-run" (
    set "DRY_RUN=true"
    shift
    goto :parse_args
)
if "%~1"=="--check" (
    set "CHECK_ONLY=true"
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    goto :show_usage
)
echo [ERROR] Unknown option: %~1
goto :show_usage

:end_parse

REM Check if clang-format is installed
echo [INFO] Checking for clang-format...
clang-format --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] clang-format is not installed or not in PATH
    echo Please install clang-format:
    echo   - Download from LLVM releases: https://releases.llvm.org/
    echo   - Or install via chocolatey: choco install llvm
    echo   - Or install via scoop: scoop install llvm
    exit /b 1
)

for /f "tokens=*" %%a in ('clang-format --version') do set "VERSION=%%a"
echo [INFO] Found clang-format: !VERSION!

REM Create temporary file list
set "TEMP_FILE=%TEMP%\clang_files_%RANDOM%.txt"

REM Find all C/C++ source files
echo [INFO] Searching for source files...
(
    for /r %%f in (*.c *.cpp *.cc *.cxx *.h *.hpp *.hxx) do (
        set "filepath=%%f"
        echo !filepath! | findstr /v /i "\\build\\" | findstr /v /i "\\.git\\"
    )
) > "%TEMP_FILE%" 2>nul

REM Count files
set "FILE_COUNT=0"
for /f %%a in ('type "%TEMP_FILE%" ^| find /c /v ""') do set "FILE_COUNT=%%a"

if %FILE_COUNT%==0 (
    echo [WARNING] No C/C++ source files found
    del "%TEMP_FILE%" 2>nul
    exit /b 0
)

echo [INFO] Found %FILE_COUNT% source files to format

REM Handle check mode
if "%CHECK_ONLY%"=="true" (
    echo [INFO] Running format check ^(no files will be modified^)...
    set "ISSUES_FOUND=false"
    
    for /f "usebackq delims=" %%f in ("%TEMP_FILE%") do (
        clang-format --dry-run --Werror "%%f" >nul 2>&1
        if errorlevel 1 (
            echo [WARNING] File needs formatting: %%f
            set "ISSUES_FOUND=true"
        )
    )
    
    if "!ISSUES_FOUND!"=="true" (
        echo [ERROR] Some files need formatting. Run without --check to fix them.
        del "%TEMP_FILE%" 2>nul
        exit /b 1
    ) else (
        echo [INFO] All files are properly formatted!
        del "%TEMP_FILE%" 2>nul
        exit /b 0
    )
)

REM Handle dry run mode
if "%DRY_RUN%"=="true" (
    echo [INFO] Dry run mode - showing what would be formatted:
    for /f "usebackq delims=" %%f in ("%TEMP_FILE%") do (
        echo   %%f
    )
    del "%TEMP_FILE%" 2>nul
    exit /b 0
)

REM Format files
echo [INFO] Formatting files...
set "FORMATTED_COUNT=0"

for /f "usebackq delims=" %%f in ("%TEMP_FILE%") do (
    clang-format -i "%%f"
    if not errorlevel 1 (
        echo [OK] %%f
        set /a FORMATTED_COUNT+=1
    ) else (
        echo [ERROR] Failed to format: %%f
    )
)

echo [INFO] Successfully formatted !FORMATTED_COUNT! files

del "%TEMP_FILE%" 2>nul
goto :eof

:show_usage
echo Usage: %~nx0 [OPTIONS]
echo.
echo Options:
echo   --dry-run    Show which files would be formatted without making changes
echo   --check      Check if files need formatting ^(exit 1 if any do^)
echo   --help       Show this help message
echo.
echo This script formats all C/C++ source files in the current directory
echo and subdirectories using clang-format.
exit /b 0
