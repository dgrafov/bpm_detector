#ifndef FS_CONTROLLER_H_
#define FS_CONTROLLER_H_

#include <string>
#include <vector>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_SYSTEM_NO_DEPRECATED

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

class FSController
{
public:
    FSController( const char * path );
    bool getAudioFiles( std::vector< std::string >& files ) const;
private:
    void getAudioFiles( const boost::filesystem::path& dirPath, std::vector< std::string >& files ) const;
    boost::filesystem::path mFullPath;
};

#endif
