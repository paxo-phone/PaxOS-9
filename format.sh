#!/bin/bash

# ClangFormat script for Linux/macOS
# This script formats all C/C++ source files in the project using clang-format

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if clang-format is installed
check_clang_format() {
    if ! command -v clang-format &> /dev/null; then
        print_error "clang-format is not installed or not in PATH"
        echo "Please install clang-format:"
        echo "  Ubuntu/Debian: sudo apt-get install clang-format"
        echo "  macOS: brew install clang-format"
        echo "  Fedora/RHEL: sudo dnf install clang-tools-extra"
        exit 1
    fi

    local version=$(clang-format --version)
    print_status "Found clang-format: $version"
}

# Find all C/C++ source files
find_source_files() {
    local extensions=("*.c" "*.cpp" "*.cc" "*.cxx" "*.h" "*.hpp" "*.hxx")
    local find_args=()

    for ext in "${extensions[@]}"; do
        find_args+=(-name "$ext" -o)
    done

    # Remove the last -o
    unset find_args[-1]

    find . -type f \( "${find_args[@]}" \) | grep -v "/build/" | grep -v "/\.git/"
}

# Main formatting function
format_files() {
    local dry_run=false
    local check_only=false

    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --dry-run)
                dry_run=true
                shift
                ;;
            --check)
                check_only=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done

    local files=$(find_source_files)
    local file_count=$(echo "$files" | wc -l)

    if [[ -z "$files" ]]; then
        print_warning "No C/C++ source files found"
        exit 0
    fi

    print_status "Found $file_count source files to format"

    if [[ "$check_only" == true ]]; then
        print_status "Running format check (no files will be modified)..."
        local issues_found=false

        while IFS= read -r file; do
            if ! clang-format --dry-run --Werror "$file" &> /dev/null; then
                print_warning "File needs formatting: $file"
                issues_found=true
            fi
        done <<< "$files"

        if [[ "$issues_found" == true ]]; then
            print_error "Some files need formatting. Run without --check to fix them."
            exit 1
        else
            print_status "All files are properly formatted!"
            exit 0
        fi
    fi

    if [[ "$dry_run" == true ]]; then
        print_status "Dry run mode - showing what would be formatted:"
        while IFS= read -r file; do
            echo "  $file"
        done <<< "$files"
        exit 0
    fi

    print_status "Formatting files..."
    local formatted_count=0

    while IFS= read -r file; do
        if clang-format -i "$file"; then
            echo "  ✓ $file"
            ((formatted_count++))
        else
            print_error "Failed to format: $file"
        fi
    done <<< "$files"

    print_status "Successfully formatted $formatted_count files"
}

# Show usage information
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --dry-run    Show which files would be formatted without making changes"
    echo "  --check      Check if files need formatting (exit 1 if any do)"
    echo "  --help       Show this help message"
    echo ""
    echo "This script formats all C/C++ source files in the current directory"
    echo "and subdirectories using clang-format."
}

# Main execution
main() {
    if [[ "$1" == "--help" ]]; then
        show_usage
        exit 0
    fi

    check_clang_format
    format_files "$@"
}

main "$@"
