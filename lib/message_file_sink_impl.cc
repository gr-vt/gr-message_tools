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
#include "message_file_sink_impl.h"

namespace gr {
  namespace message_file {

    message_file_sink::sptr
    message_file_sink::make(const char* filename)
    {
      return gnuradio::get_initial_sptr
        (new message_file_sink_impl(filename));
    }

    void
    message_file_sink_impl::print_pdu(pmt::pmt_t pdu)
    {
      pmt::pmt_t meta = pmt::car(pdu);
      pmt::pmt_t vector = pmt::cdr(pdu);
      std::cout << "* MESSAGE DEBUG PRINT PDU VERBOSE *\n";
      pmt::print(meta);
      size_t len = pmt::length(vector);
      std::cout << "pdu_length = " << len << std::endl;
      std::cout << "contents = " << std::endl;
      size_t offset(0);
      const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vector, offset);
      for(size_t i=0; i<len; i+=16){
        printf("%04x: ", ((unsigned int)i));
        for(size_t j=i; j<std::min(i+16,len); j++){
          printf("%02x ",d[j] );
        }

        std::cout << std::endl;
      }

      std::cout << "***********************************\n";
    }

    int
    message_file_sink_impl::num_messages()
    {
      return (int)d_messages.size();
    }

    pmt::pmt_t
    message_file_sink_impl::get_message(int i)
    {
      gr::thread::scoped_lock guard(d_mutex);

      if((size_t)i >= d_messages.size()) {
        throw std::runtime_error("message_debug: index for message out of bounds.\n");
      }

      return d_messages[i];
    }


    message_file_sink_impl::message_file_sink_impl(const char* filename)
      : gr::block("message_file_sink",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0))
    {
      message_port_register_in(pmt::mp("print_pdu"));
      set_msg_handler(pmt::mp("print_pdu"), boost::bind(&message_file_sink_impl::print_pdu, this, _1));
    }


    message_file_sink_impl::~message_file_sink_impl()
    {
    }

/*    int
    message_file_sink_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];

        // Do <+signal processing+>

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }*/

  } /* namespace message_file */
} /* namespace gr */

