#include "BpmCalculator.h"
#include "debug/DebugPrint.h"

#include <gst/app/gstappsink.h>

#include <exception>
#include <memory>
#include <algorithm>
#include <math.h>
#include <fstream>
#include <string.h>
#include <stdio.h>

using namespace std;

//Helpers
static gboolean busCallHandlerWrapper( GstBus*, GstMessage* msg, gpointer data )
{
    BpmCalculator* calc = static_cast< BpmCalculator* >( data );
    return calc->busCallHandler( msg );
}

static GstFlowReturn newBufferHandlerWrapper ( GstElement *sink, gpointer data ) {
    BpmCalculator* calc = static_cast< BpmCalculator* >( data );
    return calc->newBufferHandler( sink );
}

//TODO clean debug
ofstream myfile;

//Class  methods
//TODO create init method separate from start
//TODO think if you really need exceptions here
void BpmCalculator::calculate( const string& filename )
{
    //TODO clear data here for reuse of object. Create additional function for creating pipelines and so on
    mBpmMap.clear( );

    //TODO clean debug
    myfile.open("test.wav", ios::out | ios::binary);
    gst_init ( NULL, NULL );

    mPipeline.reset( gst_element_factory_make( "playbin", "pipeline" ) );

    if ( !mLoop )
    {
        DEBUG_PRINT( DL_ERROR, "Main loop couldn't be created\n" );
        throw;
    }

    if ( !mPipeline )
    {
        DEBUG_PRINT( DL_ERROR, "Playbin could not be created.\n" );
        throw;
    }
    // Set up the pipeline
    g_object_set( G_OBJECT( mPipeline.get( ) ), "uri", filename.c_str( ), NULL );

    // Add a bus message handler
    unique_ptr< GstBus, void( * )( gpointer ) > bus(
            gst_pipeline_get_bus( GST_PIPELINE ( mPipeline.get( ) ) ),
            gst_object_unref );
    mBusWatchId = gst_bus_add_watch (
            bus.get( ),
            busCallHandlerWrapper,
            static_cast< gpointer >( this ) ) ;
    // Create a custom sink for the playbin
    GstElement* sink = gst_element_factory_make ( "appsink", "sink" );
    g_object_set( G_OBJECT( sink ), "sync", FALSE,
                                    "emit-signals", TRUE,
                                    "caps",
                                        gst_caps_new_simple ("audio/x-raw",
                                        "format", G_TYPE_STRING, sizeof( float ) == 4 ? "F32LE": "F64LE",
                                        "rate", G_TYPE_INT, SAMPLE_RATE,
                                        "channels", G_TYPE_INT, 1,
                                        NULL ), NULL );

    g_signal_connect ( G_OBJECT( sink ), "new-sample", G_CALLBACK ( newBufferHandlerWrapper ), static_cast< gpointer >( this ) );

    if ( !sink )
    {
        DEBUG_PRINT( DL_ERROR, "Custom sink for the playbin could not be created. Player not started.\n" );
        throw;
    }

    // Set custom sink to the playbin
    g_object_set( GST_OBJECT( mPipeline.get( ) ), "audio-sink", sink, NULL );

    // Start playing
    gst_element_set_state ( mPipeline.get( ), GST_STATE_PLAYING );

    DEBUG_PRINT( DL_INFO, "Starting playback\n" );

    g_main_loop_run ( mLoop.get( ) );
}

BpmCalculator::BpmCalculator( const CompletedCallback& cb )
    : mLoop( g_main_loop_new( NULL, FALSE ), g_main_loop_unref )
    , mPipeline( NULL, gst_object_unref )
    , mCallback( cb )
    , mAubioBpmCalculator( new_aubio_tempo( "default", WINDOW_SIZE, HOP_SIZE, SAMPLE_RATE ) )
    , mAubioInputBuffer( new_fvec ( HOP_SIZE ) )
    , mAubioOutputBuffer( new_fvec ( 2 ) )
    , mAubioInputBufferSamples( 0 )
{
}

BpmCalculator::~BpmCalculator( ) {
    gst_element_set_state ( mPipeline.get( ), GST_STATE_NULL);
    g_source_remove ( mBusWatchId );
    myfile.close();
    //TODO think of analyzing several files without recreating BPMCalculator
    del_aubio_tempo( mAubioBpmCalculator );
    del_fvec( mAubioInputBuffer );
    del_fvec( mAubioOutputBuffer );
    aubio_cleanup( );
}


gboolean BpmCalculator::busCallHandler( GstMessage* msg )
{
    switch ( GST_MESSAGE_TYPE ( msg ) ) {
    case GST_MESSAGE_EOS:
        DEBUG_PRINT( DL_INFO, "End of stream" );
        mCallback( calculateBpm( ) );
        g_main_loop_quit ( mLoop.get( ) );
        break;
    case GST_MESSAGE_ERROR:
        gchar  *debug;
        GError *error;
        gst_message_parse_error ( msg, &error, &debug );
        g_free ( debug );
        DEBUG_PRINT( DL_ERROR, "Error: %s\n", error->message );
        g_error_free ( error );
        g_main_loop_quit ( mLoop.get( ) );
        break;
    default:
       break;
    }
    return TRUE;
}

GstFlowReturn BpmCalculator::newBufferHandler ( GstElement *sink ) {
    GstSample *sample = gst_app_sink_pull_sample( GST_APP_SINK( sink ) );
    if ( sample )
    {
        GstBuffer * buf = gst_sample_get_buffer( sample );
        GstMapInfo info;
        gst_buffer_map ( buf, &info, GST_MAP_READ );


        //TODO clear debug
        guint8 *dataPtr = info.data;
        for ( gsize i = 0; i < info.size; i++ )
        {
            myfile << *( dataPtr + i );
        }
        //Fill aubio input buffer
        unsigned int samplesLeft = info.size / sizeof( float ); //size of float


        while ( samplesLeft >= HOP_SIZE )
        {
            if( mAubioInputBufferSamples > 0 )
            {
                memcpy( mAubioInputBuffer->data + mAubioInputBufferSamples, dataPtr, ( HOP_SIZE - mAubioInputBufferSamples ) * sizeof( float ) );
                dataPtr += ( HOP_SIZE  - mAubioInputBufferSamples ) * sizeof( float );
                samplesLeft -= ( HOP_SIZE - mAubioInputBufferSamples );
                mAubioInputBufferSamples = 0;
            }
            else
            {
                memcpy( mAubioInputBuffer->data, dataPtr, HOP_SIZE * sizeof( float ) );
                dataPtr += HOP_SIZE * sizeof( float );
                samplesLeft -= HOP_SIZE;
            }

            aubio_tempo_do( mAubioBpmCalculator, mAubioInputBuffer, mAubioOutputBuffer);
            if ( mAubioOutputBuffer->data[ 0 ] != 0 )
            {
                unsigned int bpm = round( aubio_tempo_get_bpm( mAubioBpmCalculator ) );
                auto ret = mBpmMap.insert( pair< unsigned int, unsigned int >( bpm, 1 ) );
                if ( ret.second == false )
                {
                    (*(ret.first)).second++;
                }
            }
        }
        if ( samplesLeft > 0 )
        {
            memcpy( mAubioInputBuffer->data, dataPtr, samplesLeft * sizeof( float ) );
            mAubioInputBufferSamples = samplesLeft;
        }


        gst_sample_unref( sample );
        gst_buffer_unmap ( buf, &info );

        return GST_FLOW_OK;
    }
    return GST_FLOW_EOS;
}

unsigned int BpmCalculator::calculateBpm( )
{
    using PairType = decltype( mBpmMap )::value_type;

    auto pair = std::max_element( mBpmMap.begin( ), mBpmMap.end( ),
        [] ( const PairType& p1, const PairType & p2 )
            {
                return p1.second < p2.second;
            } );
    return pair->first;
}












