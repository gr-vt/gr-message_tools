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


#ifndef INCLUDED_MESSAGE_FILE_MESSAGE_FILE_SINK_H
#define INCLUDED_MESSAGE_FILE_MESSAGE_FILE_SINK_H

#include <message_file/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace message_file {

    /*!
     * \brief <+description of block+>
     * \ingroup message_file
     *
     */
    class MESSAGE_FILE_API message_file_sink : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<message_file_sink> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of message_file::message_file_sink.
       *
       * To avoid accidental use of raw pointers, message_file::message_file_sink's
       * constructor is in a private implementation
       * class. message_file::message_file_sink::make is the public interface for
       * creating new instances.
       */
      static sptr make(const char* filename);
    };

  } // namespace message_file
} // namespace gr

#endif /* INCLUDED_MESSAGE_FILE_MESSAGE_FILE_SINK_H */

