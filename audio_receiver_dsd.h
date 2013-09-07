#ifndef LPF_H
#define LPF_H

#include <cstdio>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <boost/shared_ptr.hpp>
#include <gr_io_signature.h>
#include <gr_hier_block2.h>
#include <gr_firdes.h>
#include <gr_fir_filter_ccf.h>
#include <gr_fir_filter_fff.h>
#include <gr_freq_xlating_fir_filter_ccf.h>
#include <filter/firdes.h>
#include <filter/rational_resampler_base_ccc.h>
#include <gr_quadrature_demod_cf.h>
#include <analog/quadrature_demod_cf.h>
#include <dsd_block_ff.h>
#include <gr_sig_source_f.h>
#include <gr_sig_source_c.h>
#include <gr_multiply_cc.h>
#include <gr_audio_sink.h>
#include <gr_file_sink.h>
#include <gr_rational_resampler_base_ccf.h>
#include <gr_rational_resampler_base_fff.h>
#include <gr_block.h>
//Valve
#include <gr_null_sink.h>
#include <gr_null_source.h>
#include <gr_head.h>
#include <gr_kludge_copy.h>
//#include <smartnet_wavsink.h>
//#include <gr_wavfile_sink.h>
//#include <blocks/wavfile_sink.h>

class log_dsd;

typedef boost::shared_ptr<log_dsd> log_dsd_sptr;

log_dsd_sptr make_log_dsd(float f, float c, long t, long r);

class log_dsd : public gr_hier_block2
{
  friend log_dsd_sptr make_log_dsd(float f, float c, long t, long r);
protected:
    log_dsd(float f, float c, long t, long r);

public:
    ~log_dsd();
	void tune_offset(float f);
	float get_freq();
	long get_talkgroup();
	void set_talkgroup(long tg);
	long rx_timeout();
	void close();
	void mute();
	void unmute();
	void active();	
	//void forecast(int noutput_items, gr_vector_int &ninput_items_required);

private:
	float center, freq;
	bool muted;
	long talkgroup;
	double samp_rate;
	time_t timestamp;
    

    /* GR blocks */
    	gr_fir_filter_ccf_sptr lpf;
	gr_fir_filter_fff_sptr sym_filter;
	gr_freq_xlating_fir_filter_ccf_sptr prefilter;
	gr_sig_source_c_sptr offset_sig; 

	gr_multiply_cc_sptr mixer;
	gr_file_sink_sptr fs;

	gr_rational_resampler_base_ccf_sptr downsample_sig;
	gr_rational_resampler_base_fff_sptr upsample_audio;
	//gr::analog::quadrature_demod_cf::sptr demod;
	gr_quadrature_demod_cf_sptr demod;
	dsd_block_ff_sptr dsd;
	audio_sink::sptr sink;
	//smartnet_wavsink_sptr wav_sink
	//gr::blocks::wavfile_sink::sptr wav_sink;
	gr_null_sink_sptr null_sink;
	gr_null_source_sptr null_source;
	gr_head_sptr head_source;
	gr_kludge_copy_sptr copier;

};


#endif

