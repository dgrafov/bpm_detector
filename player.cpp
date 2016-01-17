#include "player.h"
#include "debug/debug.h"

#include <exception>
#include <memory>

using namespace std;

void Player::startPlayer( )
{
    /* Set up the pipeline */
    g_object_set ( G_OBJECT( sourceSp.get( ) ), "location", "sample.mp3", NULL);

    /* we add a message handler */
    std::shared_ptr< GstBus > busSp(
            gst_pipeline_get_bus( GST_PIPELINE (pipelineSp.get( ) ) ),
            gst_object_unref );
    busWatchId = gst_bus_add_watch (
            busSp.get( ),
            busCallHandlerWrapper, 
            static_cast< gpointer >( this ) );

    /* we add all elements into the pipeline */
    gst_bin_add_many( GST_BIN ( pipelineSp.get( ) ),
            sourceSp.get( ),
            decoderSp.get( ),
            sinkSp.get( ),
            NULL);

    gst_element_link_many(
            sourceSp.get( ),
            decoderSp.get( ),
            sinkSp.get( ),
            NULL );
    gst_element_set_state ( pipelineSp.get( ), GST_STATE_PLAYING );

    DEBUG_PRINT( DL_INFO, "Starting playback\n" );

    g_main_loop_run ( loopSp.get( ) );
}

Player::Player( )
{
    gst_init ( NULL, NULL );

    loopSp.reset( g_main_loop_new( NULL, FALSE ), g_main_loop_unref );
    pipelineSp.reset(
            gst_pipeline_new( "bpm-player" ),
            gst_object_unref);
   
    sourceSp.reset(
            gst_element_factory_make( "filesrc", "source" ),
            gst_object_unref );

    decoderSp.reset(
            gst_element_factory_make ( "mad", "decoder" ),
            gst_object_unref );
    sinkSp.reset(
            gst_element_factory_make ( "autoaudiosink", "audiosink" ),
            gst_object_unref );

    if ( !loopSp ||
         !pipelineSp  ||
         !sourceSp ||
         !decoderSp ||
         !sinkSp )
    {
        DEBUG_PRINT( DL_ERROR, "One of pipeline elements could not \
                be created. Player not started.\n" );
        throw;
    }
}

Player::~Player( ) {
    gst_element_set_state (pipelineSp.get(), GST_STATE_NULL);
    g_source_remove (busWatchId);
}

gboolean Player::busCallHandlerWrapper( GstBus* bus, GstMessage* msg, gpointer data )
{
    Player * player = static_cast< Player * >( data );
    return player->busCallHandler( bus, msg, player->loopSp.get( ) );
}

gboolean Player::busCallHandler( GstBus* bus, GstMessage* msg, gpointer data )
{
    GMainLoop *loop = static_cast< GMainLoop * >( data );

    switch ( GST_MESSAGE_TYPE ( msg ) ) {
    case GST_MESSAGE_EOS:
        DEBUG_PRINT( DL_INFO, "End of stream\n" );
        g_main_loop_quit ( loopSp.get( ) );
        break;
    case GST_MESSAGE_ERROR:
        gchar  *debug;
        GError *error;
        gst_message_parse_error ( msg, &error, &debug );
        g_free ( debug );
        DEBUG_PRINT( DL_ERROR, "Error: %s\n", error->message );
        g_error_free ( error );
        g_main_loop_quit ( loopSp.get( ) );
        break;
    default:
       break;
    }
    return TRUE;
}












