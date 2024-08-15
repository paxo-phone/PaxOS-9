#include <string>

#include <fstream>
#include "filestream.hpp"

namespace storage
{

    FileStream::FileStream() {}

    FileStream::FileStream(const std::string &path,
                           Mode mode)
    {
        this->open(path, mode);
    }

    FileStream::~FileStream()
    {
        m_stream.close();
    }

    void FileStream::open(const std::string &path,
                          Mode mode)
    {
        if (mode == READ)
            m_stream.open(path, std::ios::in | std::ios::binary);
        else if (mode == WRITE)
            m_stream.open(path, std::ios::out | std::ios::binary);
        else if (mode == APPEND)
            m_stream.open(path, std::ios::app | std::ios::binary);
    }

    void FileStream::close(void)
    {
        m_stream.close();
    }

    std::string FileStream::read(void)
    {
        std::string text = "";
        std::string line;
        bool firstLine = true;

        while (std::getline(m_stream, line))
        {
            if (firstLine)
            {
                firstLine = false;
            }
            else
            {
                text += "\n";
            }
            text += line;
        }

        return text;
    }

    std::string FileStream::readline(void)
    {
        std::string line;
        std::getline(m_stream, line);
        return line;
    }

    std::string FileStream::readword(void)
    {
        std::string word;
        m_stream >> word;
        return word;
    }

    char FileStream::readchar(void)
    {
        return m_stream.get();
    }

    void FileStream::write(const std::string &str)
    {
        m_stream << str;
    }

    void FileStream::write(const char c)
    {
        m_stream << c;
    }

    bool FileStream::isopen(void) const
    {
        return m_stream.is_open();
    }

    long FileStream::size(void)
    {
        const auto begin = m_stream.tellg();
        m_stream.seekg(0, std::ios::end);
        const auto end = m_stream.tellg();
        const auto fsize = (end - begin);
        return fsize;
    }

    FileStream &operator<<(FileStream &stream,
                           const std::string &text)
    {
        stream.write(text);
        return stream;
    }

    FileStream &operator>>(FileStream &stream,
                           std::string &buff)
    {
        buff = stream.readword();
        return stream;
    }

}