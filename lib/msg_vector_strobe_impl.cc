/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "msg_vector_strobe_impl.h"
#include <cstdio>
#include <fcntl.h>
#include <algorithm>
#include <stdio.h>
#include <stdexcept>

namespace gr {
  namespace message_tools {

    msg_vector_strobe::sptr
    msg_vector_strobe::make(float high_ms, float low_ms, const std::vector<uint8_t> &msg_vector)
    {
      return gnuradio::get_initial_sptr
        (new msg_vector_strobe_impl(high_ms, low_ms, msg_vector));
    }

    /*
     * The private constructor
     */
    msg_vector_strobe_impl::msg_vector_strobe_impl(float high_ms, float low_ms, const std::vector<uint8_t> &msg_vector)
      : gr::block("msg_vector_strobe",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        d_finished(false),
        d_high_ms(high_ms),
        d_low_ms(low_ms),
        d_data(msg_vector)
    {
      message_port_register_out(pmt::mp("strobe"));
      d_thread = boost::shared_ptr<boost::thread>
        (new boost::thread(boost::bind(&msg_vector_strobe_impl::run, this)));

      seed();
      set_period();
      get_msg_strobe();
    }

    /*
     * Our virtual destructor.
     */
    msg_vector_strobe_impl::~msg_vector_strobe_impl()
    {
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();
    }

    void msg_vector_strobe_impl::get_msg_strobe()
    {
      int8_t mess_vec[d_data.size()];
      memcpy( &mess_vec[0], &d_data[0], sizeof(uint8_t)*d_data.size() );
      pmt::pmt_t vec_pdu = pmt::init_s8vector((size_t)d_data.size(), &mess_vec[0]);
      d_msg = pmt::cons(pmt::PMT_NIL, vec_pdu);
      d_updated = false;
    }

    void msg_vector_strobe_impl::run()
    {
      while(!d_finished) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(d_period_ms)); 
        if(d_finished) {
          return;
        }

        if(d_updated) { get_msg_strobe(); }
        message_port_pub(pmt::mp("strobe"), d_msg);
      }
    }

    void msg_vector_strobe_impl::seed()
    {
      srand(time(0));
    }

    void msg_vector_strobe_impl::set_period()
    {
      float temp = rand() / float(RAND_MAX);
      d_period_ms = (d_high_ms - d_low_ms) * temp + d_low_ms;
    }

  } /* namespace message_tools */
} /* namespace gr */

