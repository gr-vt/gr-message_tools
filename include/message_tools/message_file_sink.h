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


#ifndef INCLUDED_MESSAGE_TOOLS_MESSAGE_FILE_SINK_H
#define INCLUDED_MESSAGE_TOOLS_MESSAGE_FILE_SINK_H

#include <message_tools/api.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/file_sink_base.h>

namespace gr {
  namespace message_tools {

    class MESSAGE_TOOLS_API message_file_sink : virtual public block,
                                               virtual public blocks::file_sink_base
    {
     public:
      typedef boost::shared_ptr<message_file_sink> sptr;

      static sptr make(const char* filename, bool append=false);

      virtual int num_messages() = 0;

      virtual pmt::pmt_t get_message(int i) = 0;

      virtual void set_unbuffered(bool unbuffered) = 0;
    };

  } // namespace message_tools
} // namespace gr

#endif /* INCLUDED_MESSAGE_TOOLS_MESSAGE_FILE_SINK_H */

