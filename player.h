
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
    //TODO: unique_ptr
    std::shared_ptr< GMainLoop > mLoop;
    std::shared_ptr< GstElement > mPipeline;
    guint mBusWatchId;
};


#endif /* PLAYER_H_ */
