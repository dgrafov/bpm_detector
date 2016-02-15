#include "BpmCalculator.h"
#include "FSController.h"
#include "debug/DebugPrint.h"
#include <vector>
#include <iostream>

using namespace std;

void on_bpm_completed( unsigned int bpm )
{
    DEBUG_PRINT( DL_INFO, "Bpm %d\n", bpm );
}

int main( int argc, char* argv[] )
{
    if ( argc < 2 )
    {
        cout << "Usage: " << argv[ 0 ] << " path " << endl;
        return 0;
    }

    FSController fsc( argv[ 1 ] );

    vector< string > files;
    if( fsc.getAudioFiles( files ) )
    {
        DEBUG_PRINT( DL_INFO, "Files found:" );
        for ( auto it = files.begin( ); it != files.end( ); ++it )
        {
            DEBUG_PRINT( DL_INFO, "%s", it->c_str( ) );
        }
    }
    else
    {
        cout << "Path " << argv[ 0 ] << " not found" << endl;
    }

    //BpmCalculator calculator( on_bpm_completed );
    //calculator.calculate( "file:///home/grafov/storage/workspace/sample.mp3" );
    return 0;
}

