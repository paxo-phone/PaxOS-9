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
        #ifdef ESP_PLATFORM
        if(m_file != NULL)
            fclose(m_file);
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

        #ifdef ESP_PLATFORM

        if(m_file == NULL)
        {
            if( mode == READ ) {
                m_file = fopen(path.c_str(), "r");
            }
            else { // if ( mode == WRITE ) 
                if(erase) {
                    m_file = fopen(path.c_str(), "w");
                }
                else {
                    m_file = fopen(path.c_str(), "a");
                }
            }
        }
        
        #endif
    }

    void FileStream::close(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream.close();
        #endif
        #ifdef ESP_PLATFORM
            if(m_file != NULL)
                fclose(m_file);
        #endif
    }

    std::string FileStream::read(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string text = "";
            
            std::string line;
            while( std::getline(m_stream, line) )
            {
                text += line;
                if(line.back() != '\n')
                    text+="\n";
            }

            return text;
        #endif
        #ifdef ESP_PLATFORM
            char line[1024]; // Taille maximale d'une ligne

            if (m_file == NULL) {
                return "";
            }

            std::string output;

            while (fgets(line, sizeof(line), m_file) != NULL) {
                std::string line_str(line);
                if (!line_str.empty() && line_str.back() != '\n') {
                    line_str.push_back('\n');
                }
                output += line_str;
            }
            
            return output;

        #endif
    }

    std::string FileStream::readline(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string line;
            std::getline(m_stream, line);
            return line;
        #endif

        #ifdef ESP_PLATFORM
            char buffer[1024];

            if (m_file == NULL) {
                return "";
            }

            while (fgets(buffer, sizeof(buffer), m_file) != NULL) {
                return std::string(buffer);
            }

            return "";
        #endif
    }

    std::string FileStream::readword(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::string word;
            m_stream >> word;
            return word;
        #endif
        #ifdef ESP_PLATFORM
            if (m_file == NULL) {
                return "";
            }

            char word[1024];
            while (fscanf(m_file, "%s", word) != EOF) {
                printf("Word: %s\n", word);
            }
            return std::string(word);
        #endif
    }

    char FileStream::readchar(void) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return m_stream.get();
        #endif
        #ifdef ESP_PLATFORM
            if (m_file == NULL) {
                return -1;
            }

            int character;
            if ((character = fgetc(m_file)) != EOF) {
                return char(character);
            }
            return -1;
        #endif
    }

    void FileStream::write(const std::string& str) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream << str;
        #endif
        #ifdef ESP_PLATFORM
            if (m_file == NULL) {
                return;
            }

            fprintf(m_file, "%s", str.c_str());
        #endif
    }

    void FileStream::write(const char c) {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            m_stream << c;
        #endif
        #ifdef ESP_PLATFORM
            if (m_file == NULL) {
                return;
            }

            fputc(c, m_file);
        #endif
    }

    bool FileStream::isopen(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return m_stream.is_open();
        #endif
        #ifdef ESP_PLATFORM
            return (m_file != NULL);
        #endif
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