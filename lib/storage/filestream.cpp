#include <string>

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
    #include <fstream>
#endif

#include "filestream.hpp"

namespace storage {

    FileStream::FileStream() {}

    FileStream::FileStream(const std::string& path, 
                           Mode mode, 
                           bool erase)
    {
        this->open(path, mode, erase);
    }

    FileStream::~FileStream() {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream.close();
        #endif
    }

    void FileStream::open(const std::string& path, 
                          Mode mode, 
                          bool erase)
    {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

            if( mode == READ ) {
                m_stream.open(path, std::ios::in);
            }
            else { // if ( mode == WRITE ) 
                if(erase) {
                    m_stream.open(path, std::ios::out);
                }
                else {
                    m_stream.open(path, std::ios::app);
                }
            }

        #endif
    }

    void FileStream::close(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream.close();
        #endif
    }

    std::string FileStream::read(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string text = "";
            
            std::string line;
            while( std::getline(m_stream, line) )
                text += line + '\n';

            return text;
        #endif
        return "";
    }

    std::string FileStream::readline(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string line;
            std::getline(m_stream, line);
            return line;
        #endif
        return "";
    }

    std::string FileStream::readword(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string word;
            m_stream >> word;
            return word;
        #endif
        return "";
    }

    char FileStream::readchar(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return m_stream.get();
        #endif
        return -1;
    }

    void FileStream::write(const std::string& str) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream << str;
        #endif
    }

    void FileStream::write(const char c) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream << c;
        #endif
    }

    bool FileStream::isopen(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return m_stream.is_open();
        #endif
        return false;
    }

    FileStream& operator<<(FileStream& stream, 
                           const std::string& text)
    {
        stream.write(text);
        return stream;
    }

    FileStream& operator>>(FileStream& stream, 
                           std::string& buff)
    {
        buff = stream.readword();
        return stream;
    }

}