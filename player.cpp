#include "player.h"
#include "debug/debug.h"

#include <exception>
#include <memory>

using namespace std;

void Player::startPlayer( )
{
    /* Set up the pipeline */
    g_object_set ( G_OBJECT( source ), "location", "sample.mp3", NULL);

    /* we add a message handler */
    //TODO: unique_ptr
    shared_ptr< GstBus > bus(
            gst_pipeline_get_bus( GST_PIPELINE ( pipeline.get( ) ) ),
            gst_object_unref );
    busWatchId = gst_bus_add_watch (
            bus.get( ),
            busCallHandlerWrapper, 
            static_cast< gpointer >( this ) );

    /* we add all elements into the pipeline */
    gst_bin_add_many( GST_BIN ( pipeline.get( ) ),
            source,
            decoder,
            converter,
            bpmDetector,
            sink,
            NULL );

    gst_element_link_many(
            source,
            decoder,
            converter,
            bpmDetector,
            sink,
            NULL );
    gst_element_set_state ( pipeline.get( ), GST_STATE_PLAYING );

    DEBUG_PRINT( DL_INFO, "Starting playback\n" );

    g_main_loop_run ( loop.get( ) );
}

Player::Player( )
{
    gst_init ( NULL, NULL );

    loop.reset( g_main_loop_new( NULL, FALSE ), g_main_loop_unref );
    pipeline.reset( gst_pipeline_new( "bpm-player" ), gst_object_unref );
   
    source = gst_element_factory_make( "filesrc", "source" );

    decoder = gst_element_factory_make ( "mad", "decoder" );
    converter = gst_element_factory_make ( "audioconvert", "converter" );
    bpmDetector = gst_element_factory_make ( "bpmdetect", "detector" );
    sink = gst_element_factory_make ( "fakesink", "audiosink" );

    if ( !loop ||
         !pipeline  ||
         !source ||
         !decoder ||
         !converter ||
         !bpmDetector ||
         !sink )
    {
        DEBUG_PRINT( DL_ERROR, "One of pipeline elements could not be created. Player not started.\n" );
        throw;
    }
}

Player::~Player( ) {
    gst_element_set_state (pipeline.get(), GST_STATE_NULL);
    g_source_remove (busWatchId);
}

gboolean Player::busCallHandlerWrapper( GstBus* bus, GstMessage* msg, gpointer data )
{
    Player * player = static_cast< Player * >( data );
    return player->busCallHandler( bus, msg, player->loop.get( ) );
}

gboolean Player::busCallHandler( GstBus* bus, GstMessage* msg, gpointer data )
{
    GMainLoop *loop = static_cast< GMainLoop * >( data );

    switch ( GST_MESSAGE_TYPE ( msg ) ) {
    case GST_MESSAGE_EOS:
        DEBUG_PRINT( DL_INFO, "End of stream\n" );
        g_main_loop_quit ( loop );
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
        g_main_loop_quit ( loop );
        break;
    default:
       break;
    }
    return TRUE;
}












