#!/usr/bin/env python
# python script to build a display data blob

import sys
import os.path
import argparse
import subprocess
from pldata import blob
import pldata.mkwfblob
import pldata.dump


if __name__ == "__main__":

	parser = argparse.ArgumentParser(
	description="Produce PL flat display EEPROM binary waveform blob, writes it to the EEPROM and verifies it.")
	parser.add_argument('display_id', help="display identifier")
	parser.add_argument('vcom', type=int, help="VCOM voltage in mV")
	parser.add_argument('waveform', help="path to the binary waveform library")
	parser.add_argument('waveform_id', help="waveform identifier")
	parser.add_argument('max_retries', type=int,  help="max number of pgm retries")
	parser.add_argument('binary_path', help="path to the blob binary")
	args = parser.parse_args(sys.argv[1:])
	
	overallResult = 1
	currentTry = 0
	
	# ---------------------------------------------------------------------------
	# create blob
	while (currentTry <= args.max_retries) & (overallResult == 1):
		currentTry += 1
		print "Executing Try %d / %d" % (currentTry, args.max_retries)
		try:
						
			data = {
				'display_id': args.display_id,
				'display_type': open('./active_panel_type/type', 'r').read(),
				'vcom': args.vcom,
				'target': 'S1D13541',
				'wvf_id': args.waveform_id,
				'wvf': open(args.waveform, 'rb').read(),
				}
			blobTarget = pldata.mkwfblob.build_blob().pack(data)
			
			print "Length of blobTarget: %d" % len(blobTarget)				
			
			fp = open(args.binary_path, 'w')
			blobSize = len(blobTarget)
			for i in range (0, blobSize-1):
				fp.write(blobTarget[i])
				
			overallResult = fp.close			
		
		except Exception as e:
			print("Exception: {}".format(e))
			print("Start Retry...")

	if (overallResult == 1):
		sys.exit(1)
		
	sys.exit(0)