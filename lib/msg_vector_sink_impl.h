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

#ifndef INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_SINK_IMPL_H
#define INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_SINK_IMPL_H

#include <message_tools/msg_vector_sink.h>
#include <gnuradio/thread/thread.h>
#include <pmt/pmt.h>

namespace gr {
  namespace message_tools {

    class msg_vector_sink_impl : public msg_vector_sink
    {
     private:
      std::vector<uint8_t> d_data;
      std::vector<tag_t> d_tags;
      int d_vlen;

      void print_pdu(pmt::pmt_t pdu);

      gr::thread::mutex d_mutex;
      std::vector<pmt::pmt_t> d_messages;

     public:
      msg_vector_sink_impl(int vlen);
      ~msg_vector_sink_impl();

      void reset() { d_data.clear(); }
      std::vector<uint8_t> data() const;
      std::vector<tag_t> tags() const;
      int num_messages();
    };

  } // namespace message_tools
} // namespace gr

#endif /* INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_SINK_IMPL_H */

