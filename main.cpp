#include "BpmCalculator.h"
#include "debug/DebugPrint.h"

void on_bpm_completed( unsigned int bpm ) 
{
    DEBUG_PRINT( DL_INFO, "Bpm %d\n", bpm );
}

int main( void ) {
    BpmCalculator calculator( on_bpm_completed );
    calculator.calculate( "file:///home/grafov/workspace/sample.mp3" );    
    return 0;
}

