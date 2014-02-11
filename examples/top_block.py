#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Tue Feb 11 12:45:08 2014
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import message_file
import pmt
import wx

class top_block(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Top Block")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000

        ##################################################
        # Blocks
        ##################################################
        self.message_file_message_strobe_source_0 = message_file.message_strobe_source(10, 50, "/home/clark/workspace/2014-02-10/pybombs/src/gr-message_file/examples/msg_file_format.dat", False)
        self.message_file_message_file_sink_0 = message_file.message_file_sink("/home/clark/workspace/2014-02-10/pybombs/src/gr-message_file/examples/msg_file_out.dat",False)
        self.message_file_message_file_sink_0.set_unbuffered(False)

        ##################################################
        # Asynch Message Connections
        ##################################################
        self.msg_connect(self.message_file_message_strobe_source_0, "strobe", self.message_file_message_file_sink_0, "print_pdu")

# QT sink close method reimplementation

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

if __name__ == '__main__':
    import ctypes
    import os
    if os.name == 'posix':
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    tb = top_block()
    tb.Start(True)
    tb.Wait()

