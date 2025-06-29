#ifndef PATH_HPP
#define PATH_HPP

#include <string>
#include <vector>

namespace storage
{
    bool init();

    class Path
    {
      public:
        Path(void);
        Path(const std::string& raw);
        Path(const Path& other);

        void join(const Path& other);
        void join(const std::string& other);

        Path operator/(const Path& other) const;
        Path operator/(const std::string& other) const;

        Path& operator/=(const Path& other);
        Path& operator/=(const std::string& other);

        Path& operator=(const Path& other);
        Path& operator=(const std::string& other);

        bool operator==(const Path& other) const;

        void assign(const Path& other);
        void assign(const std::string& other);

        void clear(void);

        std::string str(void) const;

        // filetree

        std::vector<std::string> listdir(bool onlyDirs = false) const;

        bool exists(void) const;
        bool isfile(void) const;
        bool isdir(void) const;
        bool newfile(void) const;
        bool newdir(void) const;
        bool copyTo(const Path& destinationPath) const;
        bool remove(void) const;

        bool rename(const Path& to);
        bool copy(const Path& to);

        std::vector<std::string> m_steps;

      private:
        void parse(const std::string& raw);
        void simplify(void);
    };

} // namespace storage

#endif /* PATH_HPP */
