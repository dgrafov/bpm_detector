#include "player.h"
#include "debug/debug.h"

#include <exception>
#include <memory>

using namespace std;

//Helpers
static gboolean busCallHandlerWrapper( GstBus*, GstMessage* msg, gpointer data )
{
    Player * player = static_cast< Player * >( data );
    return player->busCallHandler( msg );
}


//Player methods
void Player::startPlayer( )
{
    // Set up the pipeline
    g_object_set ( G_OBJECT( mPipeline.get( ) ), "uri", "file:///home/grafov/workspace/sample.mp3", NULL );

    // Add a bus message handler
    //TODO: unique_ptr
    shared_ptr< GstBus > bus(
            gst_pipeline_get_bus( GST_PIPELINE ( mPipeline.get( ) ) ),
            gst_object_unref );
    mBusWatchId = gst_bus_add_watch (
            bus.get( ),
            busCallHandlerWrapper,
            static_cast< gpointer >( this ) )
    ;
    // Create a custom sink for the playbin
    GstElement* bin = gst_bin_new ( "bin" );
    GstElement* bpmDetector = gst_element_factory_make ( "bpmdetect", "detector" );
    GstElement* sink = gst_element_factory_make ( "fakesink", "sink" );

    if ( !bin || !bpmDetector || ! sink )
    {
        DEBUG_PRINT( DL_ERROR, "Custom sink for the playbin could not be created. Player not started.\n" );
        throw;
    }

    gst_bin_add_many( GST_BIN( bin ), bpmDetector, sink, NULL );
    gst_element_link_many( bpmDetector, sink, NULL );

    //TODO unique_ptr
    GstPad* pad = gst_element_get_static_pad ( bpmDetector, "sink" );
    GstPad* ghost_pad = gst_ghost_pad_new ( "sink", pad );
    gst_pad_set_active ( ghost_pad, TRUE );
    gst_element_add_pad ( bin, ghost_pad );
    gst_object_unref ( pad );

    // Set custom sink to the playbin
    g_object_set( GST_OBJECT( mPipeline.get( ) ), "audio-sink", bin, NULL );

    // Start playing
    gst_element_set_state ( mPipeline.get( ), GST_STATE_PLAYING );

    DEBUG_PRINT( DL_INFO, "Starting playback\n" );

    g_main_loop_run ( mLoop.get( ) );
}

//TODO constructor shouldn't throw!
Player::Player( )
{
    gst_init ( NULL, NULL );

    mLoop.reset( g_main_loop_new( NULL, FALSE ), g_main_loop_unref );

    if ( !mLoop )
    {
        DEBUG_PRINT( DL_ERROR, "Main loop couldn't be created\n" );
        throw;
    }

    mPipeline.reset( gst_element_factory_make( "playbin", "pipeline" ), gst_object_unref );

    if ( !mPipeline )
    {
        DEBUG_PRINT( DL_ERROR, "Playbin could not be created.\n" );
        throw;
    }
}

Player::~Player( ) {
    gst_element_set_state ( mPipeline.get( ), GST_STATE_NULL);
    g_source_remove ( mBusWatchId );
}



gboolean Player::busCallHandler( GstMessage* msg )
{
    switch ( GST_MESSAGE_TYPE ( msg ) ) {
    case GST_MESSAGE_EOS:
        DEBUG_PRINT( DL_INFO, "End of stream\n" );
        g_main_loop_quit ( mLoop.get( ) );
        break;
    case GST_MESSAGE_TAG:
    {
        GstTagList *tags = NULL;
        gst_message_parse_tag ( msg, &tags );
        DEBUG_PRINT( DL_INFO, "Got tags from element %s\n", GST_OBJECT_NAME ( msg->src ) );
        gdouble bpm = 0;
        if( gst_tag_list_get_double(tags, "beats-per-minute", &bpm ) )
        {
            DEBUG_PRINT( DL_INFO, "Bpm %f\n", static_cast< double >(bpm) );
        }
        gst_tag_list_free ( tags );
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












