
#ifndef PLAYER_H_
#define PLAYER_H_

#include <gst/gst.h>
#include <glib.h>
#include <memory>
#include <string>


class Player {
public:
    Player( );
    ~Player( );
    void startPlayer( );
private:
    static gboolean busCallHandlerWrapper ( GstBus *bus,
                                            GstMessage *msg,
                                            gpointer data );

    gboolean busCallHandler( GstBus *bus,
                             GstMessage *msg,
                             gpointer data);

    std::shared_ptr< GMainLoop > loopSp;
    std::shared_ptr< GstElement > pipelineSp;
    std::shared_ptr< GstElement > sourceSp;
    std::shared_ptr< GstElement > decoderSp;
    std::shared_ptr< GstElement > sinkSp;

    guint busWatchId;
};


#endif /* PLAYER_H_ */
