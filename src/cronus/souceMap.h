#ifndef _cronus_sourceMap_h_
#define _cronus_sourceMap_h_

namespace cronus
{


struct Tag
{
    std::string rel_fname;
    std::string fname;
    std::string name;
    std::string kind;
    int line;
};

struct CacheItem
{
    int mtime;
    std::vector<Tag> data;
};

class SourceMap
{
public:
    SourceMap(std::string &workingDir);
    ~SourceMap();

private:

};

} // namespace cronus

#endif// _cronus_sourceMap_h_
