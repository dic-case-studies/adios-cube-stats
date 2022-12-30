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

os.system("rm -rf casa.hdf5")

with adios2.open("casa.hdf5", "w", config_file="config.xml", io_in_config_file="HDF5IO") as fh:
    
    # Convert the FITS into a BP file
    with fits.open(filename) as hdul:

        for hdu in hdul:
            print(hdu)
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
