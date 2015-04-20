/* -*- c++ -*- */
/* 
 * Copyright 2015 <+YOU OR YOUR COMPANY+>.
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


#ifndef INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_H
#define INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_H

#include <message_tools/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace message_tools {

    /*!
     * \brief <+description of block+>
     * \ingroup message_tools
     *
     */
    class MESSAGE_TOOLS_API pdu_file_source : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<pdu_file_source> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of message_tools::pdu_file_source.
       *
       * To avoid accidental use of raw pointers, message_tools::pdu_file_source's
       * constructor is in a private implementation
       * class. message_tools::pdu_file_source::make is the public interface for
       * creating new instances.
       */
      static sptr make(const char* filename, int fileStruct=0, int dataType=0, float delay=1000., int maxSend=0, long itemCount=0);
    };

  } // namespace message_tools
} // namespace gr

#endif /* INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_H */

