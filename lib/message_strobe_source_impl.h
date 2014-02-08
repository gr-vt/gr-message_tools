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

#ifndef INCLUDED_MESSAGE_FILE_MESSAGE_STROBE_SOURCE_IMPL_H
#define INCLUDED_MESSAGE_FILE_MESSAGE_STROBE_SOURCE_IMPL_H

#include <message_file/message_strobe_source.h>
#include <boost/thread/mutex.hpp>
#include <stdio.h>

namespace gr {
  namespace message_file {

    class message_strobe_source_impl : public message_strobe_source
    {
     private:
      boost::shared_ptr<boost::thread> d_thread;
      bool d_finished;
      float d_high_ms;
      float d_low_ms;
      float d_period_ms;
      pmt::pmt_t d_msg;

      void get_msg_strobe();
      void print_msg_strobe();

      void run();

      size_t d_itemsize;
      FILE *d_fp;
      FILE *d_new_fp;
      bool d_repeat;
      bool d_updated;
      boost::mutex fp_mutex;
      fpos_t d_pos;
      fpos_t d_pos_end;
      
      void do_update();

      void seed();
      void set_period();

     public:
      message_strobe_source_impl(float high_ms, float low_ms, const char* filename, bool repeat);
      ~message_strobe_source_impl();

      void set_highP(float high_ms) { d_high_ms = high_ms; }
      void set_lowP(float low_ms) { d_low_ms = low_ms; }
      float period() const { return d_period_ms; }

      bool seek(long seek_point, int whence);
      void open(const char *filename, bool repeat);
      void close();
    };

  } // namespace message_file
} // namespace gr

#endif /* INCLUDED_MESSAGE_FILE_MESSAGE_STROBE_SOURCE_IMPL_H */

