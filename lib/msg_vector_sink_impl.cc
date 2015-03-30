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
#include <stdio.h>

namespace gr {
  namespace message_tools {

    msg_vector_sink::sptr
    msg_vector_sink::make(int vlen)
    {
      return gnuradio::get_initial_sptr
        (new msg_vector_sink_impl(vlen));
    }

    msg_vector_sink_impl::msg_vector_sink_impl(int vlen)
      : gr::block("msg_vector_sink",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        d_vlen(vlen)
    {
      message_port_register_in(pmt::mp("print_pdu"));
      set_msg_handler(pmt::mp("print_pdu"), boost::bind(&msg_vector_sink_impl::print_pdu, this, _1));
    }

    msg_vector_sink_impl::~msg_vector_sink_impl()
    {}

    void msg_vector_sink_impl::print_pdu(pmt::pmt_t pdu)
    {
      //std::stringstream sout;
      pmt::pmt_t meta = pmt::car(pdu);
      pmt::pmt_t vec = pmt::cdr(pdu);
      //std::cout << "* MESSAGE DEBUG PRINT PDU VERBOSE *\n";
      //pmt::print(meta);
      size_t len = pmt::length(vec);
      //std::cout << "pdu_length = " << len << std::endl;
      //std::cout << "contents = " << std::endl;
      size_t offset(0);
      const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vec, offset);
      for(size_t i=0; i<len; i++){
        //printf("%04x: ", ((unsigned int)i));
        //for(size_t j=i; j<std::min(i+16,len); j++){
        d_data.push_back((uint8_t)d[i]);
          //sout << boost::format("%02x ")%((unsigned int)d[j]);
        //}
        //printf("\n");
        //std::cout << std::endl;
      }
      //std::vector<tag_t> tags;
      
      //printf("\n");

      //std::cout << "***********************************\n";
      /*sout << "\n";
      
      @TYPE@ *iptr = (@TYPE@*)input_items[0];

      for(int i = 0; i < noutput_items * d_vlen; i++)
        d_data.push_back (iptr[i]);
      std::vector<tag_t> tags;
      get_tags_in_range(tags, 0, nitems_read(0), nitems_read(0) + noutput_items);
      d_tags.insert(d_tags.end(), tags.begin(), tags.end());
      return noutput_items;
      */
    }

    int msg_vector_sink_impl::num_messages()
    {
      return (int)d_messages.size();
    }

    std::vector<uint8_t> msg_vector_sink_impl::data() const
    {
      return d_data;
    }

    std::vector<tag_t> msg_vector_sink_impl::tags() const
    {
      return d_tags;
    }

  } /* namespace message_tools */
} /* namespace gr */

