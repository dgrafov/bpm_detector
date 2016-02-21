#include "BpmCalculator.h"
#include "FSController.h"
#include "debug/DebugPrint.h"
#include <vector>
#include <iostream>

using namespace std;

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
    BpmCalculator calculator;
    if ( calculator.init( ) )
    {
        for( vector< string >::iterator it = files.begin( );
             it != files.end( ); ++it )
        {
            unsigned int bpm = calculator.calculate( it->c_str( ) );
            DEBUG_PRINT( DL_INFO, "File %s, bpm %d", it->c_str( ), bpm );
        }
    }

    return 0;
}

