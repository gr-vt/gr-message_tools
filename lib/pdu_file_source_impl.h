/* -*- c++ -*- */
/* 
 * Copyright 2015 Bill Clark.
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

#ifndef INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_IMPL_H
#define INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_IMPL_H

#include <message_tools/pdu_file_source.h>
#include <boost/thread/mutex.hpp>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

namespace gr {
  namespace message_tools {

    class pdu_file_source_impl : public pdu_file_source
    {
     private:
      size_t d_itemsize;
      FILE *d_fp;
      FILE *d_new_fp;
      bool d_repeat;
      bool d_updated;
      boost::mutex fp_mutex;
      fpos_t d_pos_start;
      fpos_t d_pos;
      fpos_t d_pos_end;

      void do_update();

      boost::shared_ptr<boost::thread> d_thread;
      bool d_finished;
      float d_delay_ms;
      int d_maxCount;
      pmt::pmt_t d_msg;

      void run();
      void get_msg();

      int d_fileType;
      int d_dataType;

      std::vector<std::string> split(const std::string &s, char delim);
      std::vector<std::string> &split_helper(const std::string &s, char delim, std::vector<std::string> &elems);

      void parse_line(std::string line);
      void parse(std::string chunk, pmt::pmt_t &holder);
      int identifyS(std::string chunk);
      int identifyC(char start);
      void nullS(std::string chunk, pmt::pmt_t &holder);
      void boolS(std::string chunk, pmt::pmt_t &holder);
      void stringS(std::string chunk, pmt::pmt_t &holder);
      void longS(std::string chunk, bool pos, pmt::pmt_t &holder);
      void u64S(std::string chunk, pmt::pmt_t &holder);
      void doubleS(std::string chunk, bool pos, pmt::pmt_t &holder);
      void complexS(std::string real, bool rpos, std::string imag, bool ipos, pmt::pmt_t &holder);
      void pairS(std::string chunk, pmt::pmt_t &holder);
      void tupleS(std::string chunk, pmt::pmt_t &holder);
      void vectorS(std::string chunk, pmt::pmt_t &holder);
      void dictS(std::string chunk, pmt::pmt_t &holder);
      void uniformS(std::string chunk, pmt::pmt_t &holder);
      uint8_t bitVal(std::string chunk);

      std::vector<pmt::pmt_t> d_msg_list;
      size_t d_ml_pointer;
      bool d_loading;

      long d_items_per_pdu;

     public:
      pdu_file_source_impl(const char* filename, int fileStruct=0, int dataType=0, float delay=1000., int maxSend=0, long itemCount=0);
      ~pdu_file_source_impl();

      bool seek(long seek_point, int whence);
      void open(const char *filename, bool repeat);
      void close();
      
      void set_delay(float delay_ms) { d_delay_ms = delay_ms; }
      float delay() const { return d_delay_ms; }
      void set_maxCount(int mC) { d_maxCount = mC; }
      int maxCount() const { return d_maxCount; }
    };

  } // namespace message_tools
} // namespace gr

#endif /* INCLUDED_MESSAGE_TOOLS_PDU_FILE_SOURCE_IMPL_H */

