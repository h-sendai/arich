// -*- C++ -*-
/*!
 * @file
 * @brief
 * @date
 * @author
 *
 */

#include "ArichMonitor.h"

using DAQMW::FatalType::DATAPATH_DISCONNECTED;
using DAQMW::FatalType::INPORT_ERROR;
using DAQMW::FatalType::HEADER_DATA_MISMATCH;
using DAQMW::FatalType::FOOTER_DATA_MISMATCH;
using DAQMW::FatalType::USER_DEFINED_ERROR1;

// Module specification
// Change following items to suit your component's spec.
static const char* arichmonitor_spec[] =
{
    "implementation_id", "ArichMonitor",
    "type_name",         "ArichMonitor",
    "description",       "ArichMonitor component",
    "version",           "1.0",
    "vendor",            "Kazuo Nakayoshi, KEK",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

ArichMonitor::ArichMonitor(RTC::Manager* manager)
    : DAQMW::DaqComponentBase(manager),
      m_InPort("arichmonitor_in",   m_in_data),
      m_in_status(BUF_SUCCESS),
      m_canvas(0),
      m_hist(0),
      m_timestamp(0),
      m_timestamp_x(0.2),
      m_timestamp_y(0.92),
      m_timestamp_size(0.1),
      m_bin(0),
      m_min(0),
      m_max(0),
      m_monitor_update_rate(30),
      m_use_log_y(false),
      m_monitor_histogram_reset_rate(0),
      m_event_byte_size(0),
      m_debug(false)
{
    // Registration: InPort/OutPort/Service

    // Set InPort buffers
    registerInPort ("arichmonitor_in",  m_InPort);

    init_command_port();
    init_state_table();
    set_comp_name("ARICHMONITOR");
}

ArichMonitor::~ArichMonitor()
{
}

RTC::ReturnCode_t ArichMonitor::onInitialize()
{
    if (m_debug) {
        std::cerr << "ArichMonitor::onInitialize()" << std::endl;
    }

    return RTC::RTC_OK;
}

RTC::ReturnCode_t ArichMonitor::onExecute(RTC::UniqueId ec_id)
{
    daq_do();

    return RTC::RTC_OK;
}

int ArichMonitor::daq_dummy()
{
    if (m_canvas) {
        m_canvas->Update();
        // daq_dummy() will be invoked again after 10 msec.
        // This sleep reduces X servers' load.
        sleep(1);
    }

    return 0;
}

int ArichMonitor::daq_configure()
{
    std::cerr << "*** ArichMonitor::configure" << std::endl;

    ::NVList* paramList;
    paramList = m_daq_service0.getCompParams();
    parse_params(paramList);

    return 0;
}

int ArichMonitor::parse_params(::NVList* list)
{

    std::cerr << "param list length:" << (*list).length() << std::endl;

    int len = (*list).length();
    for (int i = 0; i < len; i+=2) {
        std::string sname  = (std::string)(*list)[i].value;
        std::string svalue = (std::string)(*list)[i+1].value;

        std::cerr << "sname: " << sname << "  ";
        std::cerr << "value: " << svalue << std::endl;
        
        if (sname == "monitorUpdateRate") {
            if (m_debug) {
                std::cerr << "monitor update rate: " << svalue << std::endl;
            }
            char *offset;
            m_monitor_update_rate = (int)strtol(svalue.c_str(), &offset, 10);
        }
        if (sname == "useLogy") {
            if (svalue == "YES" || svalue == "Yes" || svalue == "yes") {
                m_use_log_y = true;
            }
        }
        if (sname == "monitorHistogramResetRate") {
            char *offset;
            m_monitor_histogram_reset_rate = (int)strtol(svalue.c_str(), &offset, 10);
            std::cerr << "monitor histogram_reset_rate: " << svalue << std::endl;
        }
        // If you have more param in config.xml, write here
    }

    return 0;
}

int ArichMonitor::daq_unconfigure()
{
    std::cerr << "*** ArichMonitor::unconfigure" << std::endl;
    if (m_canvas) {
        delete m_canvas;
        m_canvas = 0;
    }

    if (m_hist) {
        delete m_hist;
        m_hist = 0;
    }
    if (m_timestamp) {
        delete m_timestamp;
        m_timestamp = 0;
    }

    return 0;
}

int ArichMonitor::daq_start()
{
    std::cerr << "*** ArichMonitor::start" << std::endl;

    m_in_status  = BUF_SUCCESS;

    //////////////// CANVAS FOR HISTOS ///////////////////
    if (m_canvas) {
        delete m_canvas;
        m_canvas = 0;
    }
    m_canvas = new TCanvas("c1", "histos", 0, 0, 600, 400);

    ////////////////       HISTOS      ///////////////////
    if (m_hist) {
        delete m_hist;
        m_hist = 0;
    }
    if (m_timestamp) {
        delete m_timestamp;
        m_timestamp = 0;
    }

    int m_hist_bin = N_CH;
    double m_hist_min = 0.0;
    double m_hist_max = (double) N_CH;

    // gStyle->SetStatW(0.4);
    // gStyle->SetStatH(0.2);
    // gStyle->SetOptStat("em");

    m_hist = new TH1F("hist", "hist", m_hist_bin, m_hist_min, m_hist_max);
    // m_hist->GetXaxis()->SetNdivisions(5);
    // m_hist->GetYaxis()->SetNdivisions(4);
    // m_hist->GetXaxis()->SetLabelSize(0.07);
    // m_hist->GetYaxis()->SetLabelSize(0.06);
    m_hist->SetTitle("");  // don't draw histogram title

    m_timestamp = new TText();

    return 0;
}

int ArichMonitor::daq_stop()
{
    std::cerr << "*** ArichMonitor::stop" << std::endl;

    struct timeval tv;
    struct tm tm;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    strftime(m_timebuf, sizeof(m_timebuf), "%F %T", &tm);

    m_timestamp->SetNDC();
    m_timestamp->SetTextSize(m_timestamp_size);
    m_timestamp->SetTextColor(2);
    m_timestamp->SetText(m_timestamp_x, m_timestamp_y, m_timebuf);

    m_hist->Draw();
    m_timestamp->Draw("same");
    if (m_use_log_y) {
        m_canvas->SetLogy();
    }
    m_canvas->Update();

    reset_InPort();

    return 0;
}

int ArichMonitor::daq_pause()
{
    std::cerr << "*** ArichMonitor::pause" << std::endl;

    return 0;
}

int ArichMonitor::daq_resume()
{
    std::cerr << "*** ArichMonitor::resume" << std::endl;

    return 0;
}

int ArichMonitor::reset_InPort()
{
    int ret = true;
    while(ret == true) {
        ret = m_InPort.read();
    }

    return 0;
}

int ArichMonitor::decode_data(const unsigned char* mydata)
{
    m_sampleData.magic      = mydata[0];
    m_sampleData.format_ver = mydata[1];
    m_sampleData.module_num = mydata[2];
    m_sampleData.reserved   = mydata[3];
    unsigned int netdata    = *(unsigned int*)&mydata[4];
    m_sampleData.data       = ntohl(netdata);

    if (m_debug) {
        std::cerr << "magic: "      << std::hex << (int)m_sampleData.magic      << std::endl;
        std::cerr << "format_ver: " << std::hex << (int)m_sampleData.format_ver << std::endl;
        std::cerr << "module_num: " << std::hex << (int)m_sampleData.module_num << std::endl;
        std::cerr << "reserved: "   << std::hex << (int)m_sampleData.reserved   << std::endl;
        std::cerr << "data: "       << std::dec << (int)m_sampleData.data       << std::endl;
    }

    return 0;
}

int ArichMonitor::fill_data(const unsigned char* mydata, const int size)
{
    //for (int i = 0; i < size/(int)ONE_EVENT_SIZE; i++) {
    //    decode_data(mydata);
    //    float fdata = m_sampleData.data/1000.0; // 1000 times value is received
    //    m_hist->Fill(fdata);
    //
    //   mydata+=ONE_EVENT_SIZE;
    //}

    // Channel Assignment
    // +-------+
    // |header |
    // |       |
    // +-------+
    // |  143  |
    // +-------+
    // |  142  |
    // +-------+
    //     :
    // +-------+
    // |    2  |
    // +-------+
    // |    1  |
    // +-------+
    // |    0  |
    // +-------+
    for (int i = 0; i < N_CH; i++) {
        unsigned char data = mydata[HEADER_SIZE + i];
        unsigned int ch = N_CH - i - 1;
        if (data != 0) { // does not need to decode (only lookup zero or not)
            m_hist->Fill(ch);
        }
    }

    return 0;
}

unsigned int ArichMonitor::read_InPort()
{
    /////////////// read data from InPort Buffer ///////////////
    unsigned int recv_byte_size = 0;
    bool ret = m_InPort.read();

    //////////////////// check read status /////////////////////
    if (ret == false) { // false: TIMEOUT or FATAL
        m_in_status = check_inPort_status(m_InPort);
        if (m_in_status == BUF_TIMEOUT) { // Buffer empty.
            if (check_trans_lock()) {     // Check if stop command has come.
                set_trans_unlock();       // Transit to CONFIGURE state.
            }
        }
        else if (m_in_status == BUF_FATAL) { // Fatal error
            fatal_error_report(INPORT_ERROR);
        }
    }
    else {
        recv_byte_size = m_in_data.data.length();
    }

    if (m_debug) {
        std::cerr << "m_in_data.data.length():" << recv_byte_size
                  << std::endl;
    }

    return recv_byte_size;
}

int ArichMonitor::daq_run()
{
    if (m_debug) {
        std::cerr << "*** ArichMonitor::run" << std::endl;
    }

    unsigned int recv_byte_size = read_InPort();
    if (recv_byte_size == 0) { // Timeout
        return 0;
    }

    check_header_footer(m_in_data, recv_byte_size); // check header and footer
    m_event_byte_size = get_event_size(recv_byte_size);
    if (m_event_byte_size > DATA_BUF_SIZE) {
        fatal_error_report(USER_DEFINED_ERROR1, "DATA BUF TOO SHORT");
    }

    /////////////  Write component main logic here. /////////////
    memcpy(&m_recv_data[0], &m_in_data.data[HEADER_BYTE_SIZE], m_event_byte_size);

    for (int i = 0; i < N_CHUNK; i++) {
        fill_data(&m_recv_data[i*ONE_DATA_SIZE], m_event_byte_size);
    }

    if (m_monitor_update_rate == 0) {
        m_monitor_update_rate = 1000;
    }

    unsigned long sequence_num = get_sequence_num();
    if ((sequence_num % m_monitor_update_rate) == 0) {
        struct timeval tv;
        struct tm tm;
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &tm);
        strftime(m_timebuf, sizeof(m_timebuf), "%F %T", &tm);

        m_timestamp->SetNDC();
        m_timestamp->SetTextSize(m_timestamp_size);
        m_timestamp->SetTextColor(2);
        m_timestamp->SetText(m_timestamp_x, m_timestamp_y, m_timebuf);

        m_hist->Draw();
        m_timestamp->Draw("same");
        if (m_use_log_y) {
            m_canvas->SetLogy();
        }
        m_canvas->Update();
    }
    if (m_monitor_histogram_reset_rate > 0 && sequence_num != 0) {
        if ((sequence_num % m_monitor_histogram_reset_rate) == 0) {
            std::cerr << "histogram reset" << std::endl;
            m_hist->Reset();
        }
    }
    /////////////////////////////////////////////////////////////
    inc_sequence_num();                      // increase sequence num.
    inc_total_data_size(m_event_byte_size);  // increase total data byte size

    return 0;
}

extern "C"
{
    void ArichMonitorInit(RTC::Manager* manager)
    {
        RTC::Properties profile(arichmonitor_spec);
        manager->registerFactory(profile,
                    RTC::Create<ArichMonitor>,
                    RTC::Delete<ArichMonitor>);
    }
};
