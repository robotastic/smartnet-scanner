/*
 * Copyright 2011 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


/*
 * GNU Radio C++ example creating dial tone
 * ("the simplest thing that could possibly work")
 *
 * Send a tone each to the left and right channels of stereo audio
 * output and let the user's brain sum them.
 *
 * GNU Radio makes extensive use of Boost shared pointers.  Signal processing
 * blocks are typically created by calling a "make" factory function, which
 * returns an instance of the block as a typedef'd shared pointer that can
 * be used in any way a regular pointer can.  Shared pointers created this way
 * keep track of their memory and free it at the right time, so the user
 * doesn't need to worry about it (really).
 *
 */

// Include header files for each block used in flowgraph

#include <iostream>
#include <string> 
//#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "audio_receiver_dsd.h"
#include "talkgroup.h"
#include <smartnet_crc.h>
#include <smartnet_deinterleave.h>

#include <osmosdr_source_c.h>
#include <osmosdr_sink_c.h>

#include <boost/program_options.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/tokenizer.hpp>

#include <filter/freq_xlating_fir_filter_ccf.h>
#include <filter/firdes.h>

#include <digital_fll_band_edge_cc.h>
#include <digital_clock_recovery_mm_ff.h>
#include <digital_binary_slicer_fb.h>

#include <gr_firdes.h>
#include <gr_fir_filter_ccf.h>

#include <gr_pll_freqdet_cf.h>
#include <gr_sig_source_f.h>
#include <gr_sig_source_c.h>
#include <gr_audio_sink.h>
#include <gr_correlate_access_code_tag_bb.h>
#include <gr_msg_queue.h>
#include <gr_message.h>
#include <gr_file_sink.h>
#include <gr_complex.h>
#include <gr_fir_filter_ccf.h>
#include <gr_top_block.h>
#include <gr_multiply_cc.h>

#include <ncurses.h>
#include <menu.h>
#include <fstream> 
#include <algorithm>    // copy
#include <iterator> 





namespace po = boost::program_options;

using namespace std;
using namespace boost;

int lastcmd = 0;
double center_freq;
int tg;

WINDOW *active_tg_win;
WINDOW *tg_menu_win;
WINDOW *status_win;
MENU *tg_menu;


//log_dsd_sptr log_dsd;

gr_top_block_sptr tb;
osmosdr_source_c_sptr src;
log_dsd_sptr rx;
vector<Talkgroup *> talkgroups;
vector<Talkgroup *> monitored_tg;
char **menu_choices;
ITEM **tg_menu_items;
Talkgroup *active_tg;
bool is_active;

void update_active_tg_win() {
	werase(active_tg_win); 
	box(active_tg_win, 0, 0);
	if (active_tg == NULL) {
		mvwprintw(active_tg_win,1,2,"TG: ");
		
		mvwprintw(active_tg_win,1,40,"Num: ");
		mvwprintw(active_tg_win,2,40,"Tag: ");
		mvwprintw(active_tg_win,2,60,"Group: ");
	} else {
		mvwprintw(active_tg_win,1,2,"TG: %s", active_tg->alpha_tag.c_str());
		mvwprintw(active_tg_win,2,2,"%s ", active_tg->description.c_str());
		mvwprintw(active_tg_win,1,40,"Num: %5lu", active_tg->number);
		mvwprintw(active_tg_win,2,40,"Tag: %s", active_tg->tag.c_str());
		mvwprintw(active_tg_win,2,60,"Group: %s", active_tg->group.c_str());
	}

	mvwprintw(active_tg_win,5,2,"Monitoring: ");
	for(std::vector<Talkgroup *>::iterator it = monitored_tg.begin(); it != monitored_tg.end(); ++it) {
		Talkgroup *tg = (Talkgroup *) *it;	
		wprintw(active_tg_win, "%s | ", tg->alpha_tag.c_str());
	}

	wrefresh(active_tg_win);

}
void update_status_win(char *c) {
	wclear(status_win);	
	//wattron(status_win,A_REVERSE);
	mvwprintw(status_win,0,2,"%s",c);
	wrefresh(status_win);
}
void create_status_win() {
	int startx, starty, width, height;

	height = 1;
	width = COLS;
	starty = LINES-1;
	startx = 0;

	status_win = newwin(height, width, starty, startx);
}
void create_active_tg_win() {
	int startx, starty, width, height;

	height = 10;
	width = COLS;
	starty = 0;
	startx = 0;

	active_tg_win = newwin(height, width, starty, startx);
	box(active_tg_win, 0, 0);

	wrefresh(active_tg_win);
}

void create_tg_menu() {
	std::string s;
	int n_choices, i;
 	char *c;

       	//printw("%s\n", s.c_str());
	menu_choices = new char*[talkgroups.size()];
	//menu_choices = malloc(talkgroups.size(), sizeof(char *));
	i=0;	
	for(std::vector<Talkgroup *>::iterator it = talkgroups.begin(); it != talkgroups.end(); ++it) {
		
		Talkgroup *tg = (Talkgroup *) *it;		
		s = tg->menu_string();
		c = (char *) malloc((s.size() + 1) * sizeof(char));
		//strncpy(c, s.c_str(), s.size());
    		//c[s.size()] = '\0';
		strcpy(c, s.c_str());			
		menu_choices[i] = c;
		i++;
	}

	n_choices = talkgroups.size(); //ARRAY_SIZE(menu_choices);
	tg_menu_items = (ITEM **) calloc(n_choices + 1, sizeof(ITEM *));
		
	
	for (i=0; i < n_choices; ++i) {
		tg_menu_items[i] = new_item(menu_choices[i], menu_choices[i]);
		set_item_userptr(tg_menu_items[i], (void *) talkgroups[i]);
	}

	tg_menu = new_menu((ITEM **) tg_menu_items);

	tg_menu_win = newwin(LINES - 11, COLS, 10, 0);
	keypad(tg_menu_win, TRUE);
	
		 
	set_menu_win(tg_menu, tg_menu_win);
	set_menu_sub(tg_menu, derwin(tg_menu_win, LINES - 15, COLS - 4, 2, 2));	
	set_menu_format(tg_menu, LINES - 14 , 1);
	//set_menu_mark(tg_menu, " * ");
	box(tg_menu_win,0,0);	
	menu_opts_off(tg_menu, O_SHOWDESC | O_ONEVALUE);
	//menu_opts_off(tg_menu, O_ONEVALUE);

	post_menu(tg_menu);
		 
}

float getfreq(int cmd) {
	float freq;
		if (cmd < 0x1b8) {	
			freq = float(cmd * 0.025 + 851.0125);
		} else if (cmd < 0x230) {
			freq = float(cmd * 0.025 + 851.0125 - 10.9875);
			} else {
			freq = 0;
			}
	
	return freq;
}

void parse_file(string filename) {
    ifstream in(filename.c_str());
    if (!in.is_open()) return;

	boost::char_separator<char> sep(",");
	typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer;

    vector< string > vec;
    string line;


    while (getline(in,line))
    {
	
	t_tokenizer tok(line, sep);
	//Tokenizer tok(line);
        vec.assign(tok.begin(),tok.end());

        if (vec.size() < 7) continue;

	Talkgroup *tg = new Talkgroup(atoi( vec[0].c_str()), vec[2].at(0),vec[3].c_str(),vec[4].c_str(),vec[5].c_str() ,vec[6].c_str());

	talkgroups.push_back(tg);

	
    }
	
}

float parse_message(string s) {
	float retfreq = 0;
	bool rxfound = false;
	std::vector<std::string> x;
	boost::split(x, s, boost::is_any_of(","), boost::token_compress_on);
	//vector<string> x = split(s, ","); 
	int address = atoi( x[0].c_str() ) & 0xFFF0;
	int groupflag = atoi( x[1].c_str() );
	int command = atoi( x[2].c_str() );
	
	
            
        if (command < 0x2d0) {

		if ( lastcmd == 0x308) {
		        // Channel Grant
			if ((address != 56016) && (address != 8176)) {
				retfreq = getfreq(command);
			}
		} else {
			// Call continuation
			if  ((address != 56016) && (address != 8176))  {
				retfreq = getfreq(command);
			}
		}
	}
        
	if (retfreq) {
		char c[100];
		
		if (!is_active) {
			for(std::vector<Talkgroup *>::iterator it = monitored_tg.begin(); it != monitored_tg.end(); ++it) {
				Talkgroup *target = (Talkgroup *) *it;
				if (target->number == address) {
					tb->lock();
					rx->tune_offset(retfreq);
					rx->set_talkgroup(address);
					rx->unmute();
					tb->unlock();
					active_tg = target;
					is_active = true;
					break;
				}		
			}

		} else {	
			if (rx->get_talkgroup() == address) {
				
				if (rx->get_freq() != retfreq) {
					tb->lock();
					rx->tune_offset(retfreq);
					rx->unmute();
					tb->unlock();
				}
				rx->active();

			} else {				
				if (rx->get_freq() == retfreq) {
					tb->lock();
					rx->mute();
					tb->unlock();
					is_active = false;
					active_tg = NULL;
				}
			}
		}		
		sprintf(c,"RX - Talkgroup: %5d Freq: %5g", address,retfreq);
		update_status_win(c);
		update_active_tg_win();	
	}

	if ((is_active) && (rx->rx_timeout() > 5.0)) {
	
			
			tb->lock();
			rx->mute();
			tb->unlock();
			is_active = false;
			active_tg = NULL;
			update_active_tg_win();
	}
	
	//cout << "Command: " << command << " Address: " << address << "\t GroupFlag: " << groupflag << " Freq: " << retfreq << endl;

	lastcmd = command;
       

	return retfreq;
}


int main(int argc, char **argv)
{
std::string device_addr;
    double  samp_rate, chan_freq, error;
	int if_gain, bb_gain, rf_gain;
    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("arg", po::value<std::string>(&device_addr)->default_value(""), "the device arguments in string format")
        ("rate", po::value<double>(&samp_rate)->default_value(1e6), "the sample rate in samples per second")
        ("center", po::value<double>(&center_freq)->default_value(10e6), "the center frequency in Hz")
	("error", po::value<double>(&error)->default_value(0), "the Error in frequency in Hz")
	("freq", po::value<double>(&chan_freq)->default_value(10e6), "the frequency in Hz of the trunking channel")
        ("rfgain", po::value<int>(&rf_gain)->default_value(14), "RF Gain")
	("bbgain", po::value<int>(&bb_gain)->default_value(25), "BB Gain")
	("ifgain", po::value<int>(&if_gain)->default_value(25), "IF Gain")
	("tg", po::value<int>(&tg)->default_value(0), "Talkgroup # in decimal format")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout
            << boost::format("SmartNet Trunking Scanner %s") % desc << std::endl
            << "The tags sink demo block will print USRP source time stamps." << std::endl
            << "The tags source demo block will send bursts to the USRP sink." << std::endl
            << "Look at the USRP output on a scope to see the timed bursts." << std::endl
            << std::endl;
        return ~0;
    }




 
 	tb = gr_make_top_block("smartnet");

	
	src = osmosdr_make_source_c();
	cout << "Setting sample rate to: " << samp_rate << endl;
	src->set_sample_rate(samp_rate);
	cout << "Tunning to " << center_freq - error << "hz" << endl;
	src->set_center_freq(center_freq - error,0);

	cout << "Setting RF gain to " << rf_gain << endl;
	cout << "Setting BB gain to " << bb_gain << endl;
	cout << "Setting IF gain to " << if_gain << endl;

	src->set_gain(rf_gain);
	src->set_if_gain(if_gain);
	src->set_bb_gain(bb_gain);




	float samples_per_second = samp_rate;
	float syms_per_sec = 3600;
	float gain_mu = 0.01;
	float mu=0.5;
	float omega_relative_limit = 0.3;
	float offset = center_freq - chan_freq;
	float clockrec_oversample = 3;
	int decim = int(samples_per_second / (syms_per_sec * clockrec_oversample));
	float sps = samples_per_second/decim/syms_per_sec; 
	const double pi = boost::math::constants::pi<double>();
	
	gr_msg_queue_sptr queue = gr_make_msg_queue();


	gr_sig_source_c_sptr offset_sig = gr_make_sig_source_c(samp_rate, GR_SIN_WAVE, offset, 1.0, 0.0);

	gr_multiply_cc_sptr mixer = gr_make_multiply_cc();
	
	gr_fir_filter_ccf_sptr downsample = gr_make_fir_filter_ccf(decim, gr_firdes::low_pass(1, samples_per_second, 10000, 5000, gr_firdes::WIN_HANN));

	/*prefilter = gr_make_freq_xlating_fir_filter_ccf(decim, 
						       gr_firdes::low_pass(1, samp_rate, xlate_bandwidth/2, 6000),
						       offset, 
						       samp_rate);*/

	//gr::filter::freq_xlating_fir_filter_ccf::sptr downsample = gr::filter::freq_xlating_fir_filter_ccf::make(decim, gr::filter::firdes::low_pass(1, samples_per_second, 10000, 1000, gr::filter::firdes::WIN_HANN), 0,samples_per_second);

	gr_pll_freqdet_cf_sptr pll_demod = gr_make_pll_freqdet_cf(2.0 / clockrec_oversample, 										 2*pi/clockrec_oversample, 
										-2*pi/clockrec_oversample);

	digital_fll_band_edge_cc_sptr carriertrack = digital_make_fll_band_edge_cc(sps, 0.6, 64, 1.0);

	digital_clock_recovery_mm_ff_sptr softbits = digital_make_clock_recovery_mm_ff(sps, 0.25 * gain_mu * gain_mu, mu, gain_mu, omega_relative_limit); 


	digital_binary_slicer_fb_sptr slicer =  digital_make_binary_slicer_fb();
gr_correlate_access_code_tag_bb_sptr start_correlator = gr_make_correlate_access_code_tag_bb("10101100",0,"smartnet_preamble");


	smartnet_deinterleave_sptr deinterleave = smartnet_make_deinterleave();

	smartnet_crc_sptr crc = smartnet_make_crc(queue);

	rx = make_log_dsd( chan_freq, center_freq,tg, samp_rate);

  	//audio_sink::sptr sink = audio_make_sink(44100);

	

	tb->connect(offset_sig, 0, mixer, 0);
	tb->connect(src, 0, mixer, 1);
	tb->connect(mixer, 0, downsample, 0);
	tb->connect(downsample, 0, carriertrack, 0);
	tb->connect(carriertrack, 0, pll_demod, 0);
	tb->connect(pll_demod, 0, softbits, 0);
	tb->connect(softbits, 0, slicer, 0);
	tb->connect(slicer, 0, start_correlator, 0);
	tb->connect(start_correlator, 0, deinterleave, 0);
	tb->connect(deinterleave, 0, crc, 0);

	tb->connect(src, 0, rx, 0);
	

	is_active = false;
	active_tg = NULL;	
	tb->start();
	initscr();
	cbreak();
	noecho();
	nodelay(tg_menu_win,TRUE);
	
	parse_file("ChanList.csv");
	create_status_win();
	create_active_tg_win();
	update_active_tg_win();
	
	create_tg_menu();
	wrefresh(tg_menu_win);
	update_status_win("Initialized...");
	int c;	



	while (1) {
		if (!queue->empty_p())
		{
			std::string sentence;
			gr_message_sptr msg;
			msg = queue->delete_head();
			sentence = msg->to_string();
			parse_message(sentence);
		} else {
			
			boost::this_thread::sleep(boost::posix_time::milliseconds(1.0/10));
		}
		wtimeout(tg_menu_win, 0);
		c = wgetch(tg_menu_win);
		switch(c)
		{
			
			case KEY_DOWN:
				menu_driver(tg_menu, REQ_DOWN_ITEM);
				wrefresh(tg_menu_win);
				break;
			case KEY_UP:
				menu_driver(tg_menu, REQ_UP_ITEM);
				wrefresh(tg_menu_win);				
				break;
			case KEY_LEFT:
				menu_driver(tg_menu, REQ_LEFT_ITEM);
				wrefresh(tg_menu_win);
				break;
			case KEY_RIGHT:
				menu_driver(tg_menu, REQ_RIGHT_ITEM);
				wrefresh(tg_menu_win);
				break;
			case KEY_NPAGE:
				menu_driver(tg_menu, REQ_SCR_DPAGE);
				wrefresh(tg_menu_win);
				break;
			case KEY_PPAGE:
				menu_driver(tg_menu, REQ_SCR_UPAGE);
				wrefresh(tg_menu_win);
				break;
			case ' ':
				ITEM **items;
				Talkgroup *tg;


				menu_driver(tg_menu, REQ_TOGGLE_ITEM);

				monitored_tg.clear();
				items = menu_items(tg_menu);
				
				for(int i = 0; i < item_count(tg_menu); ++i)
					if(item_value(items[i]) == TRUE)
					{	
						tg = (Talkgroup *) item_userptr(items[i]);;
						monitored_tg.push_back(tg);
					}
				
				wrefresh(tg_menu_win);
				update_active_tg_win();
				break;
		}

		
		
		
	}
	
 	endwin(); 

  // Exit normally.
  return 0;
}
