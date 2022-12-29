#!/usr/bin/env python
# coding: utf-8
from astropy.io import fits
import adios2
import numpy as np
import os
import sys

if (len(sys.argv) < 2):
  raise Exception("Usage : python fitsToBp.py <filename> ")

filename = sys.argv[1]

os.system("rm -rf casa.bp")

with adios2.open("casa.bp", "w") as fh:
    
    # Convert the FITS into a BP file
    with fits.open(filename) as hdul:
        print(hdul)
        for hdu in hdul:
            hdr = hdu.header
            for key in hdr.keys():
                fh.write(key, str(hdr[key]))


            data = hdu.data
            # Endian conversion
            # print(data.dtype.name + "printing data type")
            data = np.ascontiguousarray(hdu.data, dtype=data.dtype.name)
            size = list(data.shape)

            start = list(np.zeros(data.ndim, dtype=np.int32))
            # print(data.size)

            fh.write('data', data, size, start, size, end_step=True)
