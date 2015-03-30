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

#ifndef INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_STROBE_IMPL_H
#define INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_STROBE_IMPL_H

#include <message_tools/msg_vector_strobe.h>

namespace gr {
  namespace message_tools {

    class msg_vector_strobe_impl : public msg_vector_strobe
    {
     private:
      boost::shared_ptr<boost::thread> d_thread;
      bool d_finished;
      float d_period_ms;
      pmt::pmt_t d_msg;

      void run();

      float d_high_ms;
      float d_low_ms;
      std::vector<uint8_t> d_data;
      bool d_updated;

      void seed();
      void set_period();
      void get_msg_strobe();

     public:
      msg_vector_strobe_impl(float high_ms, float low_ms, const std::vector<uint8_t> &msg_vector);
      ~msg_vector_strobe_impl();

      void set_highP(float high_ms) { d_high_ms = high_ms; }
      void set_lowP(float low_ms) { d_low_ms = low_ms; }
      float period() const { return d_period_ms; }
      void set_msg_vec(const std::vector<uint8_t> &nmv) { d_data = nmv; d_updated = true;}
      pmt::pmt_t msg() const { return d_msg; }

    };

  } // namespace message_tools
} // namespace gr

#endif /* INCLUDED_MESSAGE_TOOLS_MSG_VECTOR_STROBE_IMPL_H */

