#include "FSController.h"
#include "debug/DebugPrint.h"

#include <set>

using namespace boost::filesystem;
using namespace std;

static const set< string > SUPPORTED_AUDIO_FORMATS
{
    ".mp3", ".aac", ".wav", ".wma", ".aiff", ".ogg"
};

FSController::FSController( const char * pathPtr )
    : mFullPath( system_complete( path( pathPtr ) ) )
{
}

bool FSController::getAudioFiles( std::vector< std::string >& files ) const
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
                    getAudioFiles( dirIter->path( ), files );
                }
                else if ( is_regular_file( dirIter->status( ) ) &&
                          SUPPORTED_AUDIO_FORMATS.find( dirIter->path( ).extension( ).string( ) ) !=
                              SUPPORTED_AUDIO_FORMATS.end( ) )
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
        if ( SUPPORTED_AUDIO_FORMATS.find( mFullPath.extension( ).string( ) ) !=
             SUPPORTED_AUDIO_FORMATS.end( ) )
        {
            files.push_back( mFullPath.string( ) );
        }
    }
    return true;
}

void FSController::getAudioFiles( const path& dirPath, vector< string >& files ) const
{
    directory_iterator endIter;
    for ( directory_iterator dirIter( dirPath ); dirIter != endIter; ++dirIter )
    {
        try
        {
            if ( is_directory( dirIter->status( ) ) )
            {
                getAudioFiles( dirIter->path( ), files );
            }
            else if ( is_regular_file( dirIter->status( ) ) &&
                      SUPPORTED_AUDIO_FORMATS.find( dirIter->path( ).extension( ).string( ) ) !=
                          SUPPORTED_AUDIO_FORMATS.end( ) )
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

