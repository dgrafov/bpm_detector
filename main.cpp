#include "BpmCalculator.h"
#include "FSController.h"
#include "debug/DebugPrint.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdlib.h>

using namespace std;

int main( int argc, char* argv[] )
{
    if ( argc < 2 || argc > 4 )
    {
        cout << "Usage: " << endl;
        cout << "\t" << argv[ 0 ] << " path [bpm]" << endl;
        cout << "\t" << argv[ 0 ] << " path [bpm_range_start] [bpm_range_end]" << endl;
        return 0;
    }
    string path( argv[ 1 ] );
    int refBpm = -1;
    int refBpmRangeStart = -1;
    int refBpmRangeEnd = -1;
    string plsPath( path );
    ostringstream oss;

    switch( argc )
    {
    case 2:
        cout << "Creating playlist for all files in folder: " << path << endl;
        cout << "(BPM of each song will be printed to the console output)" << endl;
        plsPath += "/playlist.m3u";
        break;
    case 3:
        refBpm = atoi( argv[ 2 ] );
        cout << "Creating playlist for all files in folder: " << path << " with BPM " << refBpm << endl;
        oss << "/playlist_" << refBpm << ".m3u";
        plsPath += oss.str( );
        break;
    case 4:
        refBpmRangeStart = atoi( argv[ 2 ] );
        refBpmRangeEnd = atoi( argv[ 3 ] );
        cout << "Creating playlist for all files in folder: " << path << " with BPM in range "
            << refBpmRangeStart << " - " << refBpmRangeEnd << endl;
        oss << "/playlist_" << refBpmRangeStart << "_" << refBpmRangeEnd << ".m3u";
        plsPath += oss.str( );
        break;
    }

    FSController fsc( path.c_str( ) );

    vector< string > files;
    if( fsc.getAudioFiles( files ) )
    {
        DEBUG_PRINT( DL_INFO, "Files found:" );
        for ( auto it = files.begin( ); it != files.end( ); ++it )
        {
            DEBUG_PRINT( DL_INFO, "%s", it->c_str( ) );
        }
        DEBUG_PRINT( DL_INFO, "" );
    }
    else
    {
        cout << "Path " << argv[ 1 ] << " not found" << endl;
    }
    BpmCalculator calculator;
    if ( calculator.init( ) )
    {
        ofstream pls;
        pls.open( plsPath.c_str( ) );
        pls << "#EXTM3U" << endl;

        size_t fullPathLength = fsc.getFullPath( ).length( );

        for( vector< string >::iterator it = files.begin( );
             it != files.end( ); ++it )
        {
            int bpm = calculator.calculate( *it );
            string relativePath = it->substr( fullPathLength );
            //TODO do acquisition of a relative path in a proper platform-independent way inside FSController
            if ( relativePath[ 0 ] == '/' )
            {
                relativePath = relativePath.substr( 1 );
            }

            if ( refBpm != -1 )
            {
                if ( bpm == refBpm )
                {
                    cout << *it << ": " << bpm << endl;
                    pls << relativePath << endl;
                }
            }
            else if ( refBpmRangeStart != -1 && refBpmRangeEnd != -1 )
            {
                if ( bpm >= refBpmRangeStart && bpm <= refBpmRangeEnd )
                {
                    cout << *it << ": " << bpm << endl;
                    pls << relativePath << endl;
                }
            }
            else
            {
                cout << *it << ": " << bpm << endl;
                pls << relativePath << endl;
            }
        }
        pls.close( );
    }

    return 0;
}

