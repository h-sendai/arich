// -*- C++ -*-
/*!
 * @file 
 * @brief
 * @date
 * @author
 *
 */

#ifndef ARICHREADER_H
#define ARICHREADER_H

#include "DaqComponentBase.h"

#include <daqmw/Sock.h>

using namespace RTC;

class ArichReader
    : public DAQMW::DaqComponentBase
{
public:
    ArichReader(RTC::Manager* manager);
    ~ArichReader();

    // The initialize action (on CREATED->ALIVE transition)
    // former rtc_init_entry()
    virtual RTC::ReturnCode_t onInitialize();

    // The execution action that is invoked periodically
    // former rtc_active_do()
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

private:
    TimedOctetSeq          m_out_data;
    OutPort<TimedOctetSeq> m_OutPort;

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
    int read_data_from_detectors();
    int set_data(unsigned int data_byte_size);
    int write_OutPort();

    DAQMW::Sock* m_sock;               /// socket for data server

    // Arich format
    static const int N_CH             = 144;
    static const int HEADER_SIZE      = 34;
    static const int ONE_DATA_SIZE    = HEADER_SIZE + 1*N_CH; // 1: 1 bytes for one channel data

    static const int N_CHUNK          = 1000;
    static const int SEND_BUFFER_SIZE = ONE_DATA_SIZE * N_CHUNK; // Read 1000 data set at once
    unsigned char m_data[SEND_BUFFER_SIZE];
    unsigned int  m_recv_byte_size;

    BufferStatus m_out_status;

    int m_srcPort;                        /// Port No. of data server
    std::string m_srcAddr;                /// IP addr. of data server

    bool m_debug;
};


extern "C"
{
    void ArichReaderInit(RTC::Manager* manager);
};

#endif // ARICHREADER_H
