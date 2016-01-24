#include "BpmCalculator.h"
#include "debug/DebugPrint.h"

#include <gst/app/gstappsink.h>

#include <exception>
#include <memory>
#include <algorithm>
#include <math.h>
#include <fstream>

using namespace std;

//Helpers
static gboolean busCallHandlerWrapper( GstBus*, GstMessage* msg, gpointer data )
{
    BpmCalculator* calc = static_cast< BpmCalculator* >( data );
    return calc->busCallHandler( msg );
}

//TODO clean debug
ofstream myfile;

//TODO put inside class, write a wrapper
static GstFlowReturn new_buffer ( GstElement *sink, gpointer data ) {
    GstSample *sample = gst_app_sink_pull_sample( GST_APP_SINK( sink ) );
    if ( sample )
    {
        GstBuffer * buf = gst_sample_get_buffer( sample );
        DEBUG_PRINT( DL_INFO, "Got buffer %d, blocks %u, time %lu",  gst_buffer_get_size(buf), gst_buffer_n_memory(buf), GST_BUFFER_PTS(buf) );
        GstMapInfo info;
        gst_buffer_map ( buf, &info, GST_MAP_READ );
        guint8 *dataPtr = info.data;
        for( gsize i = 0; i < info.size; i++ ) 
        {
            myfile << *( dataPtr + i );
        }
        gst_sample_unref( sample );
        gst_buffer_unmap ( buf, &info );

        return GST_FLOW_OK;
    }
    return GST_FLOW_EOS;
}

//Class  methods
//TODO create init method separate from start
//TODO think if you really need exceptions here
void BpmCalculator::calculate( const string& filename )
{

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
                                        "format", G_TYPE_STRING, "S16LE",
                                        "rate", G_TYPE_INT, 44100,
                                        "channels", G_TYPE_INT, 2,
                                        NULL ), NULL );

    g_signal_connect ( G_OBJECT( sink ), "new-sample", G_CALLBACK ( new_buffer ), NULL );

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
{}

BpmCalculator::~BpmCalculator( ) {
    gst_element_set_state ( mPipeline.get( ), GST_STATE_NULL);
    g_source_remove ( mBusWatchId );
    myfile.close();
}


gboolean BpmCalculator::busCallHandler( GstMessage* msg )
{
    switch ( GST_MESSAGE_TYPE ( msg ) ) {
    case GST_MESSAGE_EOS:
        DEBUG_PRINT( DL_INFO, "End of stream\n" );
        mCallback( calculateBpm( ) );
        g_main_loop_quit ( mLoop.get( ) );
        break;
    case GST_MESSAGE_TAG:
    {
        GstTagList *tags = NULL;
        gst_message_parse_tag ( msg, &tags );
        unique_ptr< GstTagList, void( * )( GstTagList* ) > tagsPtr( tags, gst_tag_list_unref );
        gdouble bpm = 0;
        if ( gst_tag_list_get_double(tags, "beats-per-minute", &bpm ) && bpm > 0 )
        {
            DEBUG_PRINT( DL_INFO, "Bpm %f (%d)\n", bpm, (unsigned int)round( bpm ) );
            auto ret = mBpmMap.insert( pair< unsigned int, unsigned int >( round( bpm ), 1 ) );
            if ( ret.second == false )
            {
                //element wasn't inserted - update the element
                ret.first->second += 1;
            }
        }
        break;
    }
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

unsigned int BpmCalculator::calculateBpm( )
{
    auto it = std::max_element
    (
        mBpmMap.begin( ), mBpmMap.end( ),
        [] ( const decltype( mBpmMap )::value_type& p1, 
             const decltype( mBpmMap )::value_type& p2 )
            {
                return p1.second < p2.second;
            }
    );
    if ( it->second > 1 )
    {
        return it->first;
    }
    else
    {
        return 0;
    }
}












