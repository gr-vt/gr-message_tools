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
#include "msg_vector_sink_impl.h"

namespace gr {
  namespace message_file {

    msg_vector_sink::sptr
    msg_vector_sink::make()
    {
      return gnuradio::get_initial_sptr
        (new msg_vector_sink_impl());
    }

    /*
     * The private constructor
     */
    msg_vector_sink_impl::msg_vector_sink_impl()
      : gr::sync_block("msg_vector_sink",
              gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0))
    {}

    /*
     * Our virtual destructor.
     */
    msg_vector_sink_impl::~msg_vector_sink_impl()
    {
    }

    int
    msg_vector_sink_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];

        // Do <+signal processing+>

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace message_file */
} /* namespace gr */
