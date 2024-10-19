# Paxos Storage Modules

This document details the `storage` and `appFile` modules used within the Paxos project for file system interaction.  The modules provide a platform-agnostic interface for file and directory manipulation, reading, and writing.

## 1. `storage` Module

This module provides the core file system functionalities.

### 1.1 `storage::Path` Class

Represents a file or directory path.

#### 1.1.1 Constructors

* `Path()`: Creates an empty path.
* `Path(const std::string& raw)`: Creates a path from a string, parsing and simplifying it.
* `Path(const Path& other)`: Copy constructor.

#### 1.1.2 Methods

* `join(const Path& other)`: Appends another path to this path.
* `join(const std::string& other)`: Appends a string representation of a path to this path.
* `operator/(const Path& other) const`: Returns a new path by joining this path with another.
* `operator/(const std::string& other) const`: Returns a new path by joining this path with a string representation of a path.
* `operator/=(const Path& other)`: Appends another path to this path (in-place).
* `operator/=(const std::string& other)`: Appends a string representation of a path to this path (in-place).
* `operator=(const Path& other)`: Assignment operator.
* `operator=(const std::string& other)`: Assigns a string representation of a path to this path.
* `operator==(const Path& other) const`: Equality operator.
* `assign(const Path& other)`: Assigns another path to this path.
* `assign(const std::string& other)`: Assigns a string representation of a path to this path.
* `clear()`: Clears the path.
* `str() const`: Returns the string representation of the path.
* `listdir(bool onlyDirs = false) const`: Returns a vector of filenames within the directory represented by this path. If `onlyDirs` is true, only directory names are returned.
* `exists() const`: Checks if the path exists.
* `isfile() const`: Checks if the path represents a file.
* `isdir() const`: Checks if the path represents a directory.
* `newfile() const`: Creates a new empty file at the specified path.
* `newdir() const`: Creates a new directory at the specified path.
* `remove() const`: Removes the file or directory at the specified path.
* `rename(const Path& to)`: Renames the file or directory at the specified path.

### 1.2 `storage::FileStream` Class

Provides an interface for reading and writing files.

#### 1.2.1 Constructors

* `FileStream()`: Creates an empty filestream.
* `FileStream(const std::string& path, Mode mode)`: Creates a filestream and opens the specified file with the given mode.

#### 1.2.2 Methods

* `open(const std::string& path, Mode mode)`: Opens the specified file with the given mode.
* `close()`: Closes the file.
* `read()`: Reads the entire file content into a string.
* `readline()`: Reads a single line from the file.
* `readword()`: Reads a single word from the file.
* `readchar()`: Reads a single character from the file.
* `write(const std::string& str)`: Writes a string to the file.
* `write(const char* str, std::size_t len)`: Writes a character array to the file.
* `write(const char c)`: Writes a single character to the file.
* `isopen() const`: Checks if the file is open.
* `size()`: Returns the size of the file.

#### 1.2.3 Operators

* `operator<<(FileStream& stream, const std::string& text)`: Writes a string to the filestream.
* `operator>>(FileStream& stream, std::string& buff)`: Reads a word from the filestream.

### 1.3 `storage::init()` function

Initializes the storage module (specifically for ESP32 platform). Returns `true` on success, `false` otherwise.

## 2. `appFile` Module

Provides higher-level file operations, including JSON parsing and saving.

### 2.1 Functions

* `load(std::string filename)`: Loads the content of a file into a string, handling path restrictions.
* `parse(std::string str)`: Parses a JSON string into a `nlohmann::json` object. Returns `NULL` on failure.
* `save(std::string filename, json jsonObj)`: Saves a `nlohmann::json` object to a file.


## 3. Notes

* The codebase is designed to be platform-agnostic, supporting both desktop (Linux, Windows, macOS) and embedded (ESP32) systems.
* The `PATH_LOCATION` macro defines the base directory for file operations.
* The `storage::init()` function should be called before any other `storage` functions on ESP32.
* Error handling is implemented for JSON parsing and SD card initialization.  File operations generally rely on the underlying system's error handling.
* The `appFile` module provides a convenient way to work with JSON files, built upon the `storage` module's functionalities.


This documentation provides a comprehensive overview of the `storage` and `appFile` modules, enabling developers to understand and utilize their functionalities effectively within the Paxos project.