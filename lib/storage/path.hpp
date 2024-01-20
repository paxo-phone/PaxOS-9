#ifndef PATH_HPP
#define PATH_HPP

#include <string>
#include <vector>

namespace storage {
    bool init();

    class Path {
        public:

            Path(void);
            Path(const std::string& raw);

            void join(const Path& other);
            
            Path operator/(const Path& other) const;
            
            Path& operator/=(const Path& other);
            Path& operator=(const Path& other);
            void assign(const Path& other);
            void clear(void);


            std::string str(void) const;

            // filetree

            std::vector<std::string> listdir(bool onlyDirs = false) const;

            bool exists  (void) const;
            bool isfile  (void) const;
            bool isdir   (void) const;
            bool newfile (void) const;
            bool newdir  (void) const;
            bool remove  (void) const;
            
            bool rename (const Path& to);

        private:

            void parse(const std::string& raw);
            void simplify(void);

            std::vector<std::string> m_steps ; 
    };


}



#endif /* PATH_HPP */