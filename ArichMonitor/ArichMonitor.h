// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef ARICHMONITOR_H
#define ARICHMONITOR_H

#include "DaqComponentBase.h"

#include <arpa/inet.h> // for ntohl()

////////// ROOT Include files //////////
#include "TH1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TApplication.h"

#include "SampleData.h"

using namespace RTC;

class ArichMonitor
    : public DAQMW::DaqComponentBase
{
public:
    ArichMonitor(RTC::Manager* manager);
    ~ArichMonitor();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
    TimedOctetSeq          m_in_data;
    InPort<TimedOctetSeq>  m_InPort;

private:
    int daq_dummy();
    int daq_configure();
    int daq_unconfigure();
    int daq_start();
    int daq_run();
    int daq_stop();
    int daq_pause();
    int daq_resume();

    int parse_params(::NVList* list);
    int reset_InPort();

    unsigned int read_InPort();
    //int online_analyze();
    int decode_data(const unsigned char* mydata);
    int fill_data(const unsigned char* mydata, const int size);

    BufferStatus m_in_status;

    ////////// ROOT Histogram //////////
    TCanvas *m_canvas;
    TH1F    *m_hist;
    int      m_bin;
    double   m_min;
    double   m_max;
    int      m_monitor_update_rate;
    const static unsigned int DATA_BUF_SIZE = 1024*1024;
    unsigned char m_recv_data[DATA_BUF_SIZE];
    unsigned int  m_event_byte_size;
    struct sampleData m_sampleData;

    const static int HEADER_SIZE = 34;
    const static int N_CHUNK     = 1000;
    const static int N_CH        = 144;
    const static int ONE_DATA_SIZE = HEADER_SIZE + 1*N_CH; // 1 = 1 bytes for one ch
    bool m_debug;
};


extern "C"
{
    void ArichMonitorInit(RTC::Manager* manager);
};

#endif // ARICHMONITOR_H
