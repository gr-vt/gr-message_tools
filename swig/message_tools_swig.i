/* -*- c++ -*- */

#define MESSAGE_TOOLS_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "message_tools_swig_doc.i"

%{
#include "message_tools/message_file_sink.h"
#include "message_tools/message_strobe_source.h"
#include "message_tools/msg_vector_strobe.h"
#include "message_tools/msg_vector_sink.h"
%}


%include "message_tools/message_file_sink.h"
GR_SWIG_BLOCK_MAGIC2(message_tools, message_file_sink);
%include "message_tools/message_strobe_source.h"
GR_SWIG_BLOCK_MAGIC2(message_tools, message_strobe_source);
%include "message_tools/msg_vector_strobe.h"
GR_SWIG_BLOCK_MAGIC2(message_tools, msg_vector_strobe);
%include "message_tools/msg_vector_sink.h"
GR_SWIG_BLOCK_MAGIC2(message_tools, msg_vector_sink);
