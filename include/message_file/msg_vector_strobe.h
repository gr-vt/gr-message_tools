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


#ifndef INCLUDED_MESSAGE_FILE_MSG_VECTOR_STROBE_H
#define INCLUDED_MESSAGE_FILE_MSG_VECTOR_STROBE_H

#include <message_file/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace message_file {

    class MESSAGE_FILE_API msg_vector_strobe : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<msg_vector_strobe> sptr;

      static sptr make(float high_ms, float low_ms, const std::vector<uint8_t> &msg_vector);

      virtual void set_highP(float high_ms) = 0;

      virtual void set_lowP(float low_ms) = 0;

      virtual float period() const = 0;

      virtual void set_msg_vec(const std::vector<uint8_t> &nmv) = 0;

      virtual pmt::pmt_t msg() const = 0;
    };

  } // namespace message_file
} // namespace gr

#endif /* INCLUDED_MESSAGE_FILE_MSG_VECTOR_STROBE_H */

