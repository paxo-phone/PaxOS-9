#ifndef FILE_STREAM_HPP
#define FILE_STREAM_HPP

#include <string>

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
    #include <fstream>
#endif

namespace storage {
    
    enum Mode {
        READ,
        WRITE,
        APPEND
    };

    class FileStream {

        public:

            FileStream();

            FileStream(const std::string& path, 
                       Mode mode, 
                       bool erase = false);

            ~FileStream();
            
            void open(const std::string& path, 
                      Mode mode, 
                      bool erase = false);

            void close(void);

            std::string read(void);
            std::string readline(void);
            std::string readword(void);
            char readchar(void);

            void write(const std::string& str);
            void write(const char c);

            bool isopen(void) const;

            friend FileStream& operator<<(FileStream& stream, 
                                          const std::string& text);

            friend FileStream& operator>>(FileStream& stream, 
                                          std::string& buff);

        private:

            #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
                std::fstream m_stream;
            #endif
    };

    FileStream& operator<<(FileStream& stream, 
                           const std::string& text);

    FileStream& operator>>(FileStream& stream, 
                           const std::string& buff);
}

#endif /* FILE_STREAM_HPP */
