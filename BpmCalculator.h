
#ifndef BPM_CALCULATOR_H_
#define BPM_CALCULATOR_H_

#include <gst/gst.h>
#include <glib.h>
#include <memory>


class BpmCalculator {
public:
    BpmCalculator( );
    ~BpmCalculator( );
    void calculate( );
    gboolean busCallHandler( GstMessage *msg );

private:
    std::unique_ptr< GMainLoop, void( * )( GMainLoop* ) > mLoop;
    std::unique_ptr< GstElement, void( * )( gpointer )  > mPipeline;
    guint mBusWatchId;
};


#endif /* BPM_CALCULATOR_H_ */
