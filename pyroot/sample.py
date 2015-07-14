#!/usr/bin/env python

import os
import sys
import time
import math
import array
import random
import subprocess
import struct

# to batch job (without displaying canvas)
# run this script with '-b' option:
# ./sample.py -b

# set PYTHONPATH in this program
# get libdir from root-config --libdir
if os.environ.get('ROOTSYS'):
    _root_config_command = os.environ.get('ROOTSYS') + '/bin/root-config'
    try:
        _libdir = subprocess.Popen([_root_config_command, '--libdir'],
                                   stdout=subprocess.PIPE).communicate()[0]
    except OSError, e:
        sys.stderr.write('tried root-config: %s --libdir\n' % _root_config_command)
        sys.exit(e)
    _libdir = _libdir.strip()
else:
    _libdir = '/usr/local/root/lib'
sys.path.insert(0, _libdir)

try:
    from ROOT import gROOT, gPad, TCanvas, TF1, TH1F, TH2F, TGraph, TLine
    import ROOT
except ImportError, e:
    sys.stderr.write('import ROOT fail')
    sys.exit(e)

c1 = TCanvas('c1', 'Example', 0, 0, 800, 600)

N_CH        = 144
HEADER_SIZE = 34
ONE_DATA_SET_SIZE = HEADER_SIZE + N_CH

def main():
    hist = TH1F('hist', 'HIST', N_CH, 0, N_CH)
    try:
        f = open(sys.argv[1], 'r')
    except IOError, e:
        sys.exit(e)

    while True:
        data = f.read(ONE_DATA_SET_SIZE)
        if data == '':
            break
        elem = struct.unpack('>8IH144B', data)
        for i in xrange(N_CH):
            ch_data = elem[9 + i] # 9: 8IH
            # print ch_data
            if (ch_data != 0): 
                hist.Fill(i)
        hist.Draw()
        c1.Update()
        sys.stderr.write('done')

if __name__ == '__main__':
    main()
