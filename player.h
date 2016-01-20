
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
    gboolean busCallHandler( GstMessage *msg );

private:
    std::unique_ptr< GMainLoop, void( * )( GMainLoop* ) > mLoop;
    std::unique_ptr< GstElement, void( * )( gpointer )  > mPipeline;
    guint mBusWatchId;
};


#endif /* PLAYER_H_ */
