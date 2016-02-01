
#ifndef BPM_CALCULATOR_H_
#define BPM_CALCULATOR_H_

#include <gst/gst.h>
#include <glib.h>
#include <map>
#include <memory>
#include <functional>
#include "soundtouch/BPMDetect.h"


class BpmCalculator {
public:
    typedef std::function< void( unsigned int bpm ) > CompletedCallback;

    BpmCalculator( const CompletedCallback& cb );
    ~BpmCalculator( );
    void calculate( const std::string& filename );
    gboolean busCallHandler( GstMessage *msg ); 
    GstFlowReturn newBufferHandler ( GstElement *sink ); 

private:
    unsigned int calculateBpm( );
    std::unique_ptr< GMainLoop, void( * )( GMainLoop* ) > mLoop;
    std::unique_ptr< GstElement, void( * )( gpointer )  > mPipeline;
    guint mBusWatchId;
    std::map< unsigned int, unsigned int > mBpmMap;
    CompletedCallback mCallback;
    soundtouch::BPMDetect mBpmDetect;
    soundtouch::SAMPLETYPE* mBufferPtr;
    unsigned int mBufferPos;

};


#endif /* BPM_CALCULATOR_H_ */
