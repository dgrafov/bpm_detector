#include "FSController.h"
#include "debug/DebugPrint.h"

using namespace boost::filesystem;
using namespace std;

FSController::FSController( const char * pathPtr )
    : mFullPath( system_complete( path( pathPtr ) ) )
{
}

bool FSController::getAudioFiles( std::vector< std::string >& files )
{
    if ( !exists( mFullPath ) )
    {
        DEBUG_PRINT( DL_ERROR, "Not found: %s", mFullPath.string( ).c_str( ) );
        return false;
    }

    if ( is_directory( mFullPath ) )
    {
        directory_iterator endIter;
        for ( directory_iterator dirIter( mFullPath ); dirIter != endIter; ++dirIter )
        {
            try
            {
                if ( is_directory( dirIter->status( ) ) )
                {
                }
                else if ( is_regular_file( dirIter->status( ) ) )
                {
                    files.push_back( dirIter->path( ).string( ) );
                }
            }
            catch ( const exception & ex )
            {
                DEBUG_PRINT( DL_ERROR, "%s %s", dirIter->path().string( ).c_str( ), ex.what( ) );
            }
        }
    }
    else // must be a file
    {
        files.push_back( mFullPath.string( ) );
    }
    return true;
}

