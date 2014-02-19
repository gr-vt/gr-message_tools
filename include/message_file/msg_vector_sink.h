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


#ifndef INCLUDED_MESSAGE_FILE_MSG_VECTOR_SINK_H
#define INCLUDED_MESSAGE_FILE_MSG_VECTOR_SINK_H

#include <message_file/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace message_file {

    class MESSAGE_FILE_API msg_vector_sink : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<msg_vector_sink> sptr;

      static sptr make(int vlen = 1);

      virtual void reset() = 0;
      virtual std::vector<uint8_t> data() const = 0;
      virtual std::vector<tag_t> tags() const = 0;
      virtual int num_messages() = 0;
    };

  } // namespace message_file
} // namespace gr

#endif /* INCLUDED_MESSAGE_FILE_MSG_VECTOR_SINK_H */

