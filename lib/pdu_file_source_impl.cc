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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/thread/thread.h>
#include <gnuradio/io_signature.h>
#include "pdu_file_source_impl.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <errno.h>
#include <iostream>
#include <stdio.h>

// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define  OUR_O_BINARY O_BINARY
#else
#define  OUR_O_BINARY 0
#endif
// should be handled via configure
#ifdef O_LARGEFILE
#define  OUR_O_LARGEFILE  O_LARGEFILE
#else
#define  OUR_O_LARGEFILE 0
#endif

#define PMT_GEN_TYPE

namespace gr {
  namespace message_tools {

    pdu_file_source::sptr
    pdu_file_source::make(const char* filename, int fileStruct, int dataType, float delay, int maxSend, long itemCount)
    {
      return gnuradio::get_initial_sptr
        (new pdu_file_source_impl(filename, fileStruct, dataType, delay, maxSend, itemCount));
    }

    /*
     * The private constructor
     */
    pdu_file_source_impl::pdu_file_source_impl(const char* filename, int fileStruct, int dataType, float delay, int maxSend, long itemCount)
      : gr::block("pdu_file_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
        d_fp(0),
        d_new_fp(0),
        d_updated(false),
        d_delay_ms(delay),
        d_maxCount(maxSend),
        d_fileType(fileStruct),
        d_items_per_pdu(itemCount),
        d_dataType(dataType),
        d_finished(false),
        d_loading(true),
        d_ml_pointer(0)
    {
      d_itemsize = 1; //THIS WILL CHANGE BASED ON FILETYPE----------------------------
      bool repeat = false;
      if(maxSend < 0) repeat = true;
      open(filename, repeat);
      do_update();
      
      message_port_register_out(pmt::mp("msg"));
      d_thread = boost::shared_ptr<boost::thread>
        (new boost::thread(boost::bind(&pdu_file_source_impl::run, this)));
    }

    /*
     * Our virtual destructor.
     */
    pdu_file_source_impl::~pdu_file_source_impl()
    {
      if(d_fp)
        fclose ((FILE*)d_fp);
      if(d_new_fp)
        fclose ((FILE*)d_new_fp);
        
      d_finished = true;
      d_thread->interrupt();
      d_thread->join();
    }
    
    void
    pdu_file_source_impl::run()
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1));
      while(!d_finished) {
        do_update();       // update d_fp is reqd
        if(d_fp == NULL)
          throw std::runtime_error("work with file not open");
        
        get_msg();
//          printf("The message is:\n");
//          print(d_msg);
        message_port_pub(pmt::mp("msg"), d_msg);
        boost::this_thread::sleep(boost::posix_time::milliseconds(d_delay_ms));
      }
    }
    
    void
    pdu_file_source_impl::get_msg()
    {
//      printf("GETTING MESSAGE\n");
      switch(d_fileType){
        case 2:{
          long byte_count = sizeof(gr_complex)*d_items_per_pdu;
//          printf("I want %ld items (%ld bytes)\n",d_items_per_pdu, byte_count);
          char line[byte_count];
          size_t i = fread(line,sizeof(gr_complex),d_items_per_pdu, (FILE*)d_fp);
//          printf("I read %ld items (%ld bytes)\n",i,i*sizeof(gr_complex));
          if( i == 0 ){
            //end of file or error?
            if(d_repeat){
              fsetpos(d_fp, &d_pos);
            }
            else if(d_maxCount > 0){
              fsetpos(d_fp, &d_pos);
            }
            else{
              d_finished = true;
            }
          }
          else{
            std::string pdu_line(line,i*sizeof(gr_complex));
//            printf("The string is");
//            for(int ii = 0; ii < pdu_line.length(); ii++){
//              printf(" %02x",(unsigned int)pdu_line[ii]);
//            }
//            printf("\n");
            parse_line(pdu_line);
            if(d_maxCount > 0) d_maxCount--;
          }
          break;
        }
        case 0:
        case 1:
        default:{
          char line[5000];
          char* line_start;
          line_start = fgets(line, 5000, (FILE*)d_fp);//THIS WILL CHANGE BASED ON FILETYPE----------------------------
          if((line_start == NULL) && (d_repeat)){fsetpos(d_fp, &d_pos); d_loading=false; get_msg();}
          else if((line_start == NULL) && (d_maxCount>0)) {fsetpos(d_fp, &d_pos); d_loading=false; get_msg();}
          else if(line_start == NULL) {d_finished = true;}
          else {
            std::string pdu_line(line);
            parse_line(pdu_line);
            if(d_maxCount > 0) d_maxCount--;
          }
          break;
        }
      }
    }

    std::vector<std::string> &pdu_file_source_impl::split_helper(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
      //stackoverflow.com User: Evan Teran
    }

    std::vector<std::string> pdu_file_source_impl::split(const std::string &s, char delim) {
      std::vector<std::string> elems;
      split_helper(s, delim, elems);
      return elems;
      //stackoverflow.com User: Evan Teran
    }

    bool
    pdu_file_source_impl::seek(long seek_point, int whence)
    {
      return fseek((FILE*)d_fp, seek_point *d_itemsize, whence) == 0;
    }


    void
    pdu_file_source_impl::open(const char *filename, bool repeat)
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
        ::close(fd);  // don't leak file descriptor if fdopen fails
        throw std::runtime_error("can't open file");
      }
      d_updated = true;
      d_repeat = repeat;
    }

    void
    pdu_file_source_impl::close()
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
    pdu_file_source_impl::do_update()
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
        d_pos_start = d_pos;
      }
    }
    
    void
    pdu_file_source_impl::parse_line(std::string line)
    {
      //printf("parse starting, filetype = %d\n", d_fileType);
      switch(d_fileType){
        case 1://File is a comma seperated file
        {
          if(d_loading){
            //printf("Line = [%s]\n", line.c_str());
            pmt::pmt_t holder;
            parse(line, holder);
            d_msg = holder;
            d_msg_list.push_back(holder);
          }
          else
          {
            d_msg = d_msg_list[d_ml_pointer++];
            d_ml_pointer = d_ml_pointer % d_msg_list.size();
            //pmt::print(d_msg);
          }
          break;
          
        }
        case 2://Complex data
        {
          long byte_count = sizeof(gr_complex)*d_items_per_pdu;
//          printf("Should be receiving %ld items (%ld bytes)\n",d_items_per_pdu,byte_count);
          long read_count = line.length();
//          printf("Received %ld items (%ld bytes)\n",read_count/sizeof(gr_complex),read_count);
          gr_complex* data = (gr_complex*)line.c_str();
//          printf("Complex data is:");
//          for(int ii = 0; ii < read_count/sizeof(gr_complex); ii++){
//            printf(" (%f,%f)",data[ii].real(),data[ii].imag());
//          }
//          printf("\n");
          //pmt::pmt_t meta = pmt::cons(pmt::intern("packet_len"),pmt::from_long(read_count/sizeof(gr_complex)));
          pmt::pmt_t meta = pmt::PMT_NIL;
          pmt::pmt_t data_vec = pmt::init_c32vector(read_count/sizeof(gr_complex),&data[0]);
          d_msg = pmt::cons(meta,data_vec);
//          printf("The message is:\n");
//          print(d_msg);
          break;
        }
        case 0://This is the default, PMT Generic
        default:
        {
          std::vector<std::string> chunkedLine;
          char delim1,delim2;
          delim1 = ',';
          delim2 = '.';
          chunkedLine = split(line,delim1);
          pmt::pmt_t meta = pmt::make_dict();
          pmt::pmt_t vecData = pmt::PMT_NIL;
          for(int ii = 0; ii < chunkedLine.size(); ii++){
            std::vector<std::string> pdu_pair;
            pdu_pair = split(chunkedLine[ii],delim2);
            if(pdu_pair[0].compare("Count") == 0){
              if(pdu_pair.size() <= 1)
                meta = pmt::dict_add(meta,pmt::intern(pdu_pair[0]),pmt::PMT_NIL);
              else{
                if(pdu_pair[1][0] == '\n')
                  meta = pmt::dict_add(meta,pmt::intern(pdu_pair[0]),pmt::PMT_NIL);
                else{
                  std::stringstream convert(pdu_pair[1]);
                  int val;
                  val = ((convert >> val) ? val : -1);
                //cplusplus.com User: Bazzy
                  meta = pmt::dict_add(meta,pmt::intern(pdu_pair[0]),(val >= 0) ? pmt::from_long(val) : pmt::PMT_NIL);
                }
              }
            }
            else if(pdu_pair[0].compare("Data") == 0){
              if((pdu_pair.size() == 2)&&(pdu_pair[1][0] != '\n')){
                switch(d_dataType){
                  case 0:
                  default:
                  {
                    size_t count(0);
                    std::vector<uint8_t> vec;
                    for(std::string::iterator it = pdu_pair[1].begin(); it!=pdu_pair[1].end(); ++it){
                      if((*it) == '1'){ vec.push_back(1); count++; }
                      if((*it) == '0'){ vec.push_back(0); count++; }
                    }
                    vecData = pmt::init_u8vector(count,&vec[0]);
                  }
                  break;
                }
              }
            }
          }
          d_msg = pmt::cons(meta, vecData);
          break;
        }
      } 
    }
    
    
    void
    pdu_file_source_impl::parse(std::string chunk, pmt::pmt_t &holder)
    {
#ifdef PMT_GEN_TYPE
      int type = identifyC(chunk[0]);
      //printf("Type = %d\n",type);
      switch(type){
        case 0:
        {
          //handle the # case;
          char sub = chunk[1];
          if(sub == '(')//) vector
          {
            //printf("VECTOR\n");
            int oP(1),cP(0),count(1);
            for(std::string::iterator it = chunk.begin()+2; cP != oP, it < chunk.end(); it++){
              if(*it == '(')
              {
                if(*(it-1) != '\\') oP++;
              }
              if(*it == ')')
              {
                if(*(it-1) != '\\') cP++;
              }
              count++;
            }
            std::string new_chunk(chunk, 1, count);
            vectorS(new_chunk,holder);
          }
          else if(sub == '[')//] uniform
          {
            //printf("UNIFORM\n");
            int oB(1),cB(0),count(1);
            for(std::string::iterator it = chunk.begin()+2; cB != oB, it < chunk.end(); it++){
              if(*it == '[')
              {
                if(*(it-1) != '\\') oB++;
              }
              if(*it == ']')
              {
                if(*(it-1) != '\\') cB++;
              }
              count++;
            }
            std::string new_chunk(chunk, 1, count);
            uniformS(new_chunk,holder);
          }
          else if(sub == '{')// } tuple
          {
            //printf("TUPLE\n");
            int oS(1),cS(0),count(1);
            for(std::string::iterator it = chunk.begin()+2; cS != oS, it < chunk.end(); it++){
              if(*it == '{')
              {
                if(*(it-1) != '\\') oS++;
              }
              if(*it == '}')
              {
                if(*(it-1) != '\\') cS++;
              }
              count++;
            }
            std::string new_chunk(chunk, 1, count);
            tupleS(new_chunk,holder);
          }
          else if(sub == '<')//> dict
          {
            //printf("DICTIONARY\n");
            int oA(1),cA(0),count(1);
            for(std::string::iterator it = chunk.begin()+2; cA != oA, it < chunk.end(); it++){
              if(*it == '<')
              {
                if(*(it-1) != '\\') oA++;
              }
              if(*it == '>')
              {
                if(*(it-1) != '\\') cA++;
              }
              count++;
            }
            std::string new_chunk(chunk, 1, count);
            dictS(new_chunk,holder);
          }
          else if((sub == 'F')||(sub == 'f')||(sub == 'T')||(sub == 't'))//bool
          {
            //printf("BOOL\n");
            std::string new_chunk(chunk,1,1);
            boolS(new_chunk, holder);
          }
          else
          {
            //error
            printf("# ERROR\n");
          }
          break;
        }
        case 1:
        {
          //pair or null
          char sub = chunk[1];
          if(sub == ')') //null
          {
            //printf("NULL\n");
            std::string new_chunk(chunk, 0, 2);
            nullS(new_chunk, holder);
          }
          else //pair
          {
            //printf("PAIR\n");
            int oP(1),cP(0),count(1);
            for(std::string::iterator it = chunk.begin()+1; (cP != oP)&&(it < chunk.end()); it++){
              if(*it == '(')
              {
                if(*(it-1) != '\\') oP++;
              }
              if(*it == ')')
              {
                if(*(it-1) != '\\') cP++;
              }
              count++;
            }
            std::string new_chunk(chunk, 0, count);
            pairS(new_chunk,holder);
          }
          break;
        }
        case 2:
        {
          //escaped fs
          stringS(chunk,holder);
          break;
        }
        case 3:
        {
          //this is numeric / it better not be a string damnit!
          //printf("NUMBER\n");
          //printf("number is [%s]\n",chunk.c_str());
          int nd(0),ps(0),ms(0),ji(0);
          for(std::string::iterator it = chunk.begin(); it < chunk.end(); it++){
            if(*it == '.') nd++;
            if(*it == '-') ms++;
            if(*it == '+') ps++;
            if(*it == 'j') ji++;
            if(*it == 'i') ji++;
          }
          if(nd == 0)
          {
            //long, uint64
            if((ps==1)||(ms==1))
            {
              if(ms==1)
              {
                std::string c(chunk,1);
                longS(c,false,holder);
              }
              else
              {
                longS(chunk,true,holder);
              }
            }
            else
            {
              u64S(chunk,holder);
            }
          }
          else if(nd == 1)
          {
            //double, complex?
            bool rs = true;
            if(((int)chunk[0] == 45)) rs = false;
            if(ji == 0)
            {
              if((chunk[0] == '-')||(chunk[0] == '+'))
              { 
                std::string c(chunk,1);
                doubleS(c,rs,holder);
              }
              else
              {
                doubleS(chunk,rs,holder);
              }
            }
            else
            {//just the imag part
              int count(0),start(0);
              std::string::iterator it = chunk.begin();
              if((*it == '-')||(*it == '+'))
              {
                start++;
                it++;
              }
              for(; it < chunk.end(); it++){
                count++;
                if((*it == 'j')||(*it == 'i'))
                {
                  count--;
                }
              }
              std::string real = "0.0";
              std::string imag(chunk,start,count);
              //printf("complex pieces = [%s,%s]\n",real.c_str(),imag.c_str());
              complexS(real,true,imag,rs,holder);
            }
            
          }
          else if(nd == 2)
          {
            //complex
            bool rs = true;
            bool is = true;
            if(((int)chunk[0] == 45)) rs = false;
            int count(0),start(0),icount(0);
            std::string::iterator it = chunk.begin();
            if((*it == '-')||(*it == '+'))
            {
              start++;
              it++;
            }
            bool RealSearch = true;
            for(; it < chunk.end(); it++){
              if(RealSearch)
              {
                count++;
                if((*it == '-')||(*it == '+'))
                {
                  RealSearch = false;
                  count--;
                  if(*it == '-') is = false;
                }
              }
              else
              {
                icount++;
                if((*it == 'j')||(*it == 'i'))
                {
                  icount--;
                }
              }
            }
            std::string real(chunk,start,count);
            std::string imag(chunk,start+count+1,icount);
            //printf("complex pieces = [%s,%s]\n",real.c_str(),imag.c_str());
            complexS(real,rs,imag,is,holder);
          }
          else
          {
            holder = pmt::from_long(0);
          }
          break;
        }
        case 4:
        {
          //this is either a double or a complex
          //printf("DOUBLE/COMP\n");
          //printf("number is [%s]\n",chunk.c_str());
          int nd(0),ps(0),ms(0),ji(0);
          for(std::string::iterator it = chunk.begin(); it < chunk.end(); it++){
            if(*it == '.') nd++;
            if(*it == '-') ms++;
            if(*it == '+') ps++;
            if(*it == 'j') ji++;
            if(*it == 'i') ji++;
          }
          if(nd == 1)
          {
            //double, complex?
            bool rs = true;
            if(((int)chunk[0] == 45)) rs = false;
            if(ji == 0)
            {
              if((chunk[0] == '-')||(chunk[0] == '+'))
              { 
                std::string c(chunk,1);
                doubleS(c,rs,holder);
              }
              else
              {
                doubleS(chunk,rs,holder);
              }
            }
            else
            {//just the imag part
              int count(0),start(0);
              std::string::iterator it = chunk.begin();
              if((*it == '-')||(*it == '+'))
              {
                start++;
                it++;
              }
              for(; it < chunk.end(); it++){
                count++;
                if((*it == 'j')||(*it == 'i'))
                {
                  count--;
                }
              }
              std::string real = "0.0";
              std::string imag(chunk,start,count);
              //printf("complex pieces = [%s,%s]\n",real.c_str(),imag.c_str());
              complexS(real,true,imag,rs,holder);
            }
            
          }
          else if(nd == 2)
          {
            //complex
            bool rs = true;
            bool is = true;
            if(((int)chunk[0] == 45)) rs = false;
            int count(0),start(0),icount(0);
            std::string::iterator it = chunk.begin();
            if((*it == '-')||(*it == '+'))
            {
              start++;
              it++;
            }
            bool RealSearch = true;
            for(; it < chunk.end(); it++){
              if(RealSearch)
              {
                count++;
                if((*it == '-')||(*it == '+'))
                {
                  RealSearch = false;
                  count--;
                  if(*it == '-') is = false;
                }
              }
              else
              {
                icount++;
                if((*it == 'j')||(*it == 'i'))
                {
                  icount--;
                }
              }
            }
            std::string real(chunk,start,count);
            std::string imag(chunk,start+count+1,icount);
            //printf("complex pieces = [%s,%s]\n",real.c_str(),imag.c_str());
            complexS(real,rs,imag,is,holder);
          }
          else
          {
            holder = pmt::from_long(0);
          }
        }
        case 5://this is a string, and all of it better be a string!!
        {
          stringS(chunk,holder);
          break;
        }
        default:
        {//non-human readable wtf!
          std::string n(chunk,1);
          if(n.size() > 0)
            parse(n, holder);
          break;
        }
      }
#endif
#ifndef PMT_GEN_TYPE
      holder = pmt::intern(chunk);
#endif
    }
    
    int
    pdu_file_source_impl::identifyC(char start)
    {
      int val = (int) start;
      if(val == 35)//#
        return 0;
      else if(val == 40)//( )
        return 1;
      else if(val == 92)// fs
        return 2;
      else if(((val <= 57)&&(val >= 48))||(val == 45)||(val == 43))//number-+
        return 3;
      else if(val == 46)//.
        return 4;
      else if((val <= 126)&&(val >= 32))//string
        return 5;
      else//shouldn't be in a human readable text file
        return -1;
    }
    
    int
    pdu_file_source_impl::identifyS(std::string chunk)
    {
      if(*chunk.begin() == '#') return 0;//error
      else if(*chunk.begin() == '(') return 1;//vector)
      else if(*chunk.begin() == '[') return 2;//uniform
      else if(*chunk.begin() == '{') return 3;//tuple
      else if(*chunk.begin() == '<') return 4;//dict or unknown/blob
      else if(*chunk.begin() == '\\') return 5;//number
      else if(*chunk.begin() == '\n') return 6;//error
      else if(*chunk.begin() == 0) return 7;//error
      else return -1;//string
    }
    
    void
    pdu_file_source_impl::nullS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("null = [%s]\n",chunk.c_str());
      holder = pmt::PMT_NIL;
    }
    
    void
    pdu_file_source_impl::boolS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("bool = [%s]\n",chunk.c_str());
      if((chunk[0] == 'T')||(chunk[0] == 't'))
        holder = pmt::from_bool(true);
      else
        holder = pmt::from_bool(false);
    }
    
    void
    pdu_file_source_impl::stringS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("string = [%s]\n",chunk.c_str());
      holder = pmt::intern(chunk);
    }
    
    void
    pdu_file_source_impl::longS(std::string chunk, bool pos, pmt::pmt_t &holder)
    {
      //printf("long = [%s]\n",chunk.c_str());
      std::stringstream convert(chunk);
      long val;
      val = ((convert >> val) ? val : -1);
    //cplusplus.com User: Bazzy
      if(pos)
        holder = ((val >= 0) ? pmt::from_long(val) : pmt::PMT_NIL);
      else
        holder = ((val >= 0) ? pmt::from_long(-val) : pmt::PMT_NIL);
    }
    
    void
    pdu_file_source_impl::u64S(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("u64 = [%s]\n",chunk.c_str());
      std::stringstream convert(chunk);
      uint64_t val;
      val = ((convert >> val) ? val : -1);
    //cplusplus.com User: Bazzy
      holder = ((val >= 0) ? pmt::from_uint64(val) : pmt::PMT_NIL);
    }
    
    void
    pdu_file_source_impl::doubleS(std::string chunk, bool pos, pmt::pmt_t &holder)
    {
      //printf("double = [%s]\n",chunk.c_str());
      std::stringstream convert(chunk);
      double val;
      val = ((convert >> val) ? val : -1);
    //cplusplus.com User: Bazzy
      if(pos)
        holder = ((val >= 0) ? pmt::from_double(val) : pmt::PMT_NIL);
      else
        holder = ((val >= 0) ? pmt::from_double(-val) : pmt::PMT_NIL);
    }
    
    void
    pdu_file_source_impl::complexS(std::string real, bool rpos,std::string imag, bool ipos, pmt::pmt_t &holder)
    {
      //printf("complex = [%s , %s]\n",real.c_str(),imag.c_str());
      std::stringstream convert(real);
      double val,vas;
      val = ((convert >> val) ? val : -1);
      std::stringstream convert2(imag);
      vas = ((convert2 >> vas) ? vas : -1);
    //cplusplus.com User: Bazzy
      if(rpos){
        if(ipos){
          if((val >= 0) && (vas >= 0))
            holder = pmt::from_complex(val,vas);
          else
            holder = pmt::PMT_NIL;
        }
        else{
          if((val >= 0) && (vas >= 0))
            holder = pmt::from_complex(val,-vas);
          else
            holder = pmt::PMT_NIL;
        }
      }
      else{
        if(ipos){
          if((val >= 0) && (vas >= 0))
            holder = pmt::from_complex(-val,vas);
          else
            holder = pmt::PMT_NIL;
        }
        else{
          if((val >= 0) && (vas >= 0))
            holder = pmt::from_complex(-val,-vas);
          else
            holder = pmt::PMT_NIL;
        }
      }
    }
    
    void
    pdu_file_source_impl::pairS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("pair = [%s]\n",chunk.c_str());
      bool search = true;
      int oP(1),cP(0),count(0);
      for(std::string::iterator it = chunk.begin()+1; search&&(it < chunk.end()-1); it++){
        if(*it == '.')
        {
          if((*(it-1) == ' ')&&(*(it+1) == ' '))
          {
            if(oP == cP+1) search = false;
          }
        }
        if(*it == '(')
        {
          if(*(it-1) != '\\') oP++;
        }
        if(*it == ')')
        {
          if(*(it-1) != '\\') cP++;
        }
        //printf("%c\t#(=%d, #)=%d\n",*it,oP,cP);
        count++;
      }
      if(search)
        printf("PAIRS ERROR");
      //printf("Count = %d, Size = %lu\n",count, chunk.size());
      std::string car(chunk,1,count-2);
      //printf("Meta = %s\n",car.c_str());
      int count2(0);
      for(std::string::iterator it = chunk.begin()+count+1; (oP != cP)&&(it < chunk.end()); it++){
        if(*it == '(')
        {
          if(*(it-1) != '\\') oP++;
        }
        if(*it == ')')
        {
          if(*(it-1) != '\\') cP++;
        }
        //printf("%c, ",*it);
        count2++;
      }
      std::string test(chunk,count+2);
      //printf("%s\n",test.c_str());
      //printf("Count = %d, Size = %lu\n",count2, test.size());
      std::string cdr(chunk,count+2,count2-2);
      //printf("Vect = %s\n",cdr.c_str());
      pmt::pmt_t meta;
      pmt::pmt_t vect;
      parse(car,meta);
      parse(cdr,vect);
      holder = pmt::cons(meta,vect);
    }
    
    void
    pdu_file_source_impl::tupleS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("tuple = [%s]\n",chunk.c_str());
      bool search = true;
      int oS(0),cS(0),objs(0),count(0);
      for(std::string::iterator it = chunk.begin()+1; it < chunk.end(); it++){
        if(*it == '{')
        {
          if(*(it-1) != '\\') oS++;
        }
        if(*it == '}')
        {
          if(*(it-1) != '\\') cS++;
        }
        if(*it == ' ')
        {
          if((oS == cS)&&(*(it-1) != '\\')) objs++;
        }
      }
      objs++;
      //printf("#Objs = %d\n", objs);
      holder = pmt::make_vector(objs,pmt::PMT_NIL);
      std::string piece(chunk,1);
      oS = 1; cS = 0;
      for(int o=0; o<objs; o++){
        std::string::iterator it = piece.begin()+1;
        search = true; count = 0;
        //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        //printf("This is the string = [%s]\n",piece.c_str());
        for( ; (search)&&(it < piece.end()); it++){
          if(*it == '{')
          {
            if(*(it-1) != '\\') oS++;
          }
          if(*it == '}')
          {
            if(*(it-1) != '\\') cS++;
          }
          count++;
          if((*it == ' ')&&(*(it-1) != '\\'))
          {
            if(oS == cS+1)
            {
              search = false;
              //printf("this char = %c\n",*it);
            }
          }
          if((*it == '}')&&(*(it-1) != '\\'))
          {
            if(oS == cS)
            {
              search = false;
              //printf("this char = %c\n",*it);
            }
          }
          //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        }
        //printf("str = [%s], start = 2, count = %d\n",piece.c_str(),count);
        std::string v(piece, 0, count);
        std::string temp(piece,count+1);
        //printf("val = [%s]\nremainder = [%s]\n",v.c_str(),temp.c_str());
        piece = temp;
        it = piece.begin();
        pmt::pmt_t val;
        //printf("Parsing the value\n");
        parse(v,val);
        pmt::vector_set(holder,o,val);
      }
      holder = pmt::to_tuple(holder);
    }
    
    void
    pdu_file_source_impl::vectorS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("vector = [%s]\n",chunk.c_str());
      bool search = true;
      int oP(0),cP(0),objs(0),count(0);
      for(std::string::iterator it = chunk.begin()+1; it < chunk.end(); it++){
        if(*it == '(')
        {
          if(*(it-1) != '\\') oP++;
        }
        if(*it == ')')
        {
          if(*(it-1) != '\\') cP++;
        }
        if((*it == ' ')&&(*(it-1) != '\\'))
        {
          if(oP == cP) objs++;
        }
      }
      objs++;
      //printf("#Objs = %d\n", objs);
      holder = pmt::make_vector(objs,pmt::PMT_NIL);
      std::string piece(chunk,1);
      oP = 1; cP = 0;
      for(int o=0; o<objs; o++){
        std::string::iterator it = piece.begin()+1;
        search = true; count = 0;
        //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        //printf("This is the string = [%s]\n",piece.c_str());
        for( ; (search)&&(it < piece.end()); it++){
          if(*it == '(')
          {
            if(*(it-1) != '\\') oP++;
          }
          if(*it == ')')
          {
            if(*(it-1) != '\\') cP++;
          }
          count++;
          if((*it == ' ')&&(*(it-1) != '\\'))
          {
            if(oP == cP+1)
            {
              search = false;
              //printf("this char = %c\n",*it);
            }
          }
          if((*it == ')')&&(*(it-1) != '\\'))
          {
            if(oP == cP)
            {
              search = false;
              //printf("this char = %c\n",*it);
            }
          }
          //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        }
        //printf("str = [%s], start = 2, count = %d\n",piece.c_str(),count);
        std::string v(piece, 0, count);
        std::string temp(piece,count+1);
        //printf("val = [%s]\nremainder = [%s]\n",v.c_str(),temp.c_str());
        piece = temp;
        it = piece.begin();
        pmt::pmt_t val;
        //printf("Parsing the value\n");
        parse(v,val);
        pmt::vector_set(holder,o,val);
      }
    }
    
    void
    pdu_file_source_impl::dictS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("dict = [%s]\n",chunk.c_str());
      bool search = true;
      int oP(0),cP(0),oA(1),cA(0),objs(0),count(0);
      for(std::string::iterator it = chunk.begin()+1; it < chunk.end(); it++){
        if(*it == '(')
        {
          if(*(it-1) != '\\') oP++;
        }
        if(*it == ')')
        {
          if(*(it-1) != '\\') cP++;
        }
        if(*it == '<')
        {
          if(*(it-1) != '\\') oA++;
        }
        if(*it == '>')
        {
          if(*(it-1) != '\\') cA++;
        }
        if(*it == ' ')
        {
          if(oP == cP) objs++;
        }
      }
      objs++;
      //printf("#Objs = %d\n", objs);
      holder = pmt::make_dict();
      std::string piece(chunk,0);
      oA = 1; cA = 0;
      for(int o=0; o<objs; o++){
        std::string::iterator it = piece.begin()+1;
        oP = 0; cP = 0; search = true; count = 0;
        //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        //printf("This is the string = [%s]\n",piece.c_str());
        for( ; (search)&&(it < piece.end()); it++){
          if(*it == '(')
          {
            if(*(it-1) != '\\') oP++;
          }
          if(*it == ')')
          {
            if(*(it-1) != '\\') cP++;
          }
          if(*it == '<')
          {
            if(*(it-1) != '\\') oA++;
          }
          if(*it == '>')
          {
            if(*(it-1) != '\\') cA++;
          }
          count++;
          if(*it == '|')
          {
            if((*(it-1) == ' ')&&(*(it+1) == ' '))
            {
              if(oA == cA+1)
              {
                search = false;
                count -= 3;
                //printf("this char = %c\n",*it);
              }
            }
          }
          //printf("<=%d\t>=%d\t(=%d\t)=%d\n",oA,cA,oP,cP);
        }
        //printf("str = [%s], start = 2, count = %d\n",piece.c_str(),count);
        std::string k(piece, 2, count);
        //printf("Key = [%s]\n",k.c_str());
        //printf("now char = %c\n",*it);
        std::string temp(piece,count+5);
        piece = temp;
        it = piece.begin();
        //printf("This is the chopped = [%s]\n",piece.c_str());
        count = 0; search = true;
        //printf("start char = %c\n",*it);
        for( ; (search)&&(it < piece.end()); it++){
          count++;
          if(*it == '(')
          {
            if(*(it-1) != '\\') oP++;
          }
          if(*it == ')')
          {
            if(*(it-1) != '\\')
            {
              cP++;
              if(oP == cP)
              {
                search = false;
                count--;
              }
            }
          }
          if(*it == '<')
          {
            if(*(it-1) != '\\') oA++;
          }
          if(*it == '>')
          {
            if(*(it-1) != '\\') cA++;
          }
        }
        std::string v(piece,0,count);
        //printf("Val = [%s]\n",v.c_str());
        std::string temp2(piece,count+1);
        piece = temp2;
        pmt::pmt_t key = pmt::intern(k);
        pmt::pmt_t val;
        //printf("Parsing the value\n");
        parse(v,val);
        holder = pmt::dict_add(holder,key,val);
      }
    }
    
    void
    pdu_file_source_impl::uniformS(std::string chunk, pmt::pmt_t &holder)
    {
      //printf("uniform = [%s]\n",chunk.c_str());
      bool search = true;
      int oB(1),cB(0),objs(0),count(0);
      for(std::string::iterator it = chunk.begin()+1; (search)&&(it < chunk.end()); it++){
        if(*it == '[')
        {
          if(*(it-1) != '\\') oB++;
        }
        if(*it == ']')
        {
          if(*(it-1) != '\\') cB++;
        }
        if(*it == ' ')
        {
          objs++;
        }
        if(oB==cB) search = false;
      }
      objs++;
      //printf("#Objs = %d\n", objs);
      switch(d_dataType){
        case 0:
        default:
        {
          holder = pmt::make_u8vector(objs,255);
          break;
        }
      }
      std::string piece(chunk,0);
      oB = 1; cB = 0;
      for(int o=0; o<objs; o++){
        std::string::iterator it = piece.begin()+1;
        //printf("Start Char = [%c]\n",*it);
        count = 0;
        search = true;
        for(; (search)&&(it < piece.end()); it++)
        {
          count++;
          if((*it == ' ')||(*it == ']'))
          {
            search = false;
            count--;
            std::string val(piece,1,count++);
            //printf("The val = [%s]\n",val.c_str());
            std::string temp(piece, count);
            piece = temp;
            //printf("The remainder = [%s]\n",piece.c_str());
            switch(d_dataType){
              case 0:
              default:
              {
                pmt::u8vector_set(holder,o,bitVal(val)+48);
                break;
              }
            }
          }
        }
      }
    }
    
    uint8_t
    pdu_file_source_impl::bitVal(std::string chunk)
    {
      //printf("bit = [%s]\n",chunk.c_str());
      if(chunk[0] == '0') return 0;
      else return 1;
    }

  } /* namespace message_tools */
} /* namespace gr */

