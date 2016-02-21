
#ifndef BPM_CALCULATOR_H_
#define BPM_CALCULATOR_H_

#include <gst/gst.h>
#include <glib.h>
#include <map>
#include <memory>
#include <functional>
#include <aubio/aubio.h>


class BpmCalculator
{
public:
    static const unsigned int SAMPLE_RATE = 44100;
    static const unsigned int WINDOW_SIZE = 2048;
    static const unsigned int HOP_SIZE = 256;


    BpmCalculator( );
    ~BpmCalculator( );
    unsigned int calculate( const std::string& filename );
    bool init( );

    gboolean busCallHandler( GstMessage *msg );
    GstFlowReturn newBufferHandler ( GstElement *sink );

private:
    unsigned int calculateBpm( );
    std::unique_ptr< GMainLoop, void( * )( GMainLoop* ) > mLoop;
    std::unique_ptr< GstElement, void( * )( gpointer )  > mPipeline;
    guint mBusWatchId;
    std::unique_ptr< aubio_tempo_t, void( * )( aubio_tempo_t* ) > mAubioBpmCalculator;
    std::unique_ptr< fvec_t, void( * )( fvec_t* ) > mAubioInputBuffer;
    std::unique_ptr< fvec_t, void( * )( fvec_t* ) > mAubioOutputBuffer;
    unsigned int mAubioInputBufferSamples;
    std::map< unsigned int, unsigned int > mBpmMap;
};


#endif /* BPM_CALCULATOR_H_ */
