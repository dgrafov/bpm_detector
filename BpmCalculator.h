
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
    static const unsigned int WINDOW_SIZE = 4096;
    static const unsigned int HOP_SIZE = 128;

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
    CompletedCallback mCallback;
    //TODO think of using RAII
    aubio_tempo_t* mAubioBpmCalculator;
    fvec_t* mAubioInputBuffer;
    fvec_t* mAubioOutputBuffer;
    unsigned int mAubioInputBufferSamples;
    std::map< unsigned int, unsigned int > mBpmMap;
};


#endif /* BPM_CALCULATOR_H_ */
