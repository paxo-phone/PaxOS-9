#ifndef FILE_STREAM_HPP
#define FILE_STREAM_HPP

#include <fstream>
#include <string>

#ifdef ESP_PLATFORM
#include <SD.h>
#endif

namespace storage
{

    enum Mode
    {
        READ,
        WRITE,
        APPEND
    };

    class FileStream
    {

      public:
        FileStream();

        FileStream(const std::string& path, Mode mode);

        ~FileStream();

        void open(const std::string& path, Mode mode);

        void close(void);

        std::string read(size_t returnSize = -1);
        std::string readline(void);
        std::string readword(void);
        std::size_t read(char* buffer, std::size_t len);
        char readchar(void);

        void write(const std::string& str);
        void write(const char* str, std::size_t len);
        void write(const char c);

        bool isopen(void) const;

        long size(void);
        long sizeFromCurrentPosition(void);

        friend FileStream& operator<<(FileStream& stream, const std::string& text);

        friend FileStream& operator>>(FileStream& stream, std::string& buff);

      private:
        std::fstream m_stream;
    };

    FileStream& operator<<(FileStream& stream, const std::string& text);

    FileStream& operator>>(FileStream& stream, const std::string& buff);
} // namespace storage

#endif /* FILE_STREAM_HPP */
