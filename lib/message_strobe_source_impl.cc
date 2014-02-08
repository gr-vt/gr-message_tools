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
#include "message_strobe_source_impl.h"
//#include <iostream>


// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define	OUR_O_BINARY O_BINARY
#else
#define	OUR_O_BINARY 0
#endif
// should be handled via configure
#ifdef O_LARGEFILE
#define	OUR_O_LARGEFILE	O_LARGEFILE
#else
#define	OUR_O_LARGEFILE 0
#endif

namespace gr {
  namespace message_file {

    message_strobe_source::sptr
    message_strobe_source::make(float high_ms, float low_ms, const char* filename, bool repeat)
    {
      return gnuradio::get_initial_sptr
        (new message_strobe_source_impl(high_ms, low_ms, filename, repeat));
    }

    message_strobe_source_impl::message_strobe_source_impl(float high_ms, float low_ms, const char* filename, bool repeat)
      : gr::block("message_strobe_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        d_finished(false),
        d_high_ms(high_ms),
        d_low_ms(low_ms),
        d_itemsize(1),
        d_fp(0),
        d_new_fp(0),
        d_repeat(repeat),
        d_updated(false)
    {
      message_port_register_out(pmt::mp("strobe"));
      d_thread = boost::shared_ptr<boost::thread>
        (new boost::thread(boost::bind(&message_strobe_source_impl::run, this)));
      open(filename, repeat);
      seed();
      set_period();
    }

    /*
     * Our virtual destructor.
     */
    message_strobe_source_impl::~message_strobe_source_impl()
    {
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();
      if(d_fp)
        fclose ((FILE*)d_fp);
      if(d_new_fp)
        fclose ((FILE*)d_new_fp);
    }

    void message_strobe_source_impl::get_msg_strobe()
    {
      char line[4500];
      char *rst;
      rst = fgets(line,4500,d_fp);
      if((rst == NULL) && (d_repeat)) {fsetpos(d_fp, &d_pos); get_msg_strobe();}
      else if(rst == NULL) {d_finished = true;}
      else {
        //std::cout << (rst == NULL) << "\t" << line << std::endl;
        int index = 0;
        int frame_size = 0;
        //std::cout << line[index] << std::endl;
        while((int(line[index]) != 10)){
          index += 3;
          frame_size++;
        }
        int8_t framed[frame_size];
        for(int i=0; i<frame_size; i++) {
          int upper = int(line[i*3]);
          if((upper>=48)&&(upper<=57)) upper -= 48;
          else if((upper>=65)&&(upper<=70)) upper -= 55;
          else if((upper>=97)&&(upper<=102)) upper -= 87;
          else upper = 0;
          int lower = int(line[i*3+1]);
          if((lower>=48)&&(lower<=57)) lower -= 48;
          else if((lower>=65)&&(lower<=70)) lower -= 55;
          else if((lower>=97)&&(lower<=102)) lower -= 87;
          else lower = 0;
          framed[i] = (int8_t) upper<<4 | lower;
          //std::cout << (int)framed[i] << " ";
        }
        //std::cout << std::endl;
        pmt::pmt_t vec_pdu = pmt::init_s8vector((size_t)frame_size, &framed[0]);
        d_msg = pmt::cons(pmt::PMT_NIL, vec_pdu);
      }
    }

    void message_strobe_source_impl::print_msg_strobe()
    {
      if(!d_finished){
        //std::cout << "got here" << std::endl;
        message_port_pub(pmt::mp("strobe"), d_msg);
      }
    }

    void message_strobe_source_impl::run()
    {
      while(!d_finished) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(d_period_ms)); 
        if(d_finished) {
          return;
        }

        //message_port_pub(pmt::mp("strobe"), d_msg);
        do_update();
        if(d_fp == NULL)  throw std::runtime_error("work with file not open");
        get_msg_strobe();
        print_msg_strobe();
      }
      exit(0);
    }
    
    bool
    message_strobe_source_impl::seek(long seek_point, int whence)
    {
      return fseek((FILE*)d_fp, seek_point *d_itemsize, whence) == 0;
    }


    void
    message_strobe_source_impl::open(const char *filename, bool repeat)
    {
      // obtain exclusive access for duration of this function
      gr::thread::scoped_lock lock(fp_mutex);

      int fd;

      // we use "open" to use to the O_LARGEFILE flag
      if((fd = ::open(filename, O_RDONLY | OUR_O_LARGEFILE | OUR_O_BINARY)) < 0) {
        perror(filename);
        throw std::runtime_error("can't open file");
      }

      if(d_new_fp) {
        fclose(d_new_fp);
        d_new_fp = 0;
      }

      if((d_new_fp = fdopen (fd, "rb")) == NULL) {
        perror(filename);
        ::close(fd);	// don't leak file descriptor if fdopen fails
        throw std::runtime_error("can't open file");
      }

      d_updated = true;
      d_repeat = repeat;
    }

    void
    message_strobe_source_impl::close()
    {
      // obtain exclusive access for duration of this function
      gr::thread::scoped_lock lock(fp_mutex);

      if(d_new_fp != NULL) {
        fclose(d_new_fp);
        d_new_fp = NULL;
      }
      d_updated = true;
    }

    void
    message_strobe_source_impl::do_update()
    {
      if(d_updated) {
        gr::thread::scoped_lock lock(fp_mutex); // hold while in scope

        if(d_fp)
          fclose(d_fp);

        d_fp = d_new_fp;    // install new file pointer
        d_new_fp = 0;
        d_updated = false;
        fgetpos(d_fp, &d_pos);
        fseek(d_fp, 0, SEEK_END);
        fgetpos(d_fp, &d_pos_end);
        fsetpos(d_fp, &d_pos);
      }
    }

    void
    message_strobe_source_impl::seed()
    {
      srand(time(0));
    }

    void
    message_strobe_source_impl::set_period()
    {
      float temp = rand() / float(RAND_MAX);
      d_period_ms = (d_high_ms - d_low_ms) * temp + d_low_ms;
    }

  } /* namespace message_file */
} /* namespace gr */

