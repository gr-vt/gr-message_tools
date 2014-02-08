/* -*- c++ -*- */

#define MESSAGE_FILE_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "message_file_swig_doc.i"

%{
#include "message_file/message_file_sink.h"
#include "message_file/message_strobe_source.h"
%}


%include "message_file/message_file_sink.h"
GR_SWIG_BLOCK_MAGIC2(message_file, message_file_sink);
%include "message_file/message_strobe_source.h"
GR_SWIG_BLOCK_MAGIC2(message_file, message_strobe_source);
