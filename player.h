
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
    //TODO: unique_ptr
    std::shared_ptr< GMainLoop > loop;
    std::shared_ptr< GstElement > pipeline;
    GstElement* source;
    GstElement* decoder;
    GstElement* converter;
    GstElement* bpmDetector;
    GstElement* sink;

    guint busWatchId;
};


#endif /* PLAYER_H_ */
