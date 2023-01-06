#!/usr/bin/env python
# coding: utf-8
from astropy.io import fits
import adios2
import numpy as np
import os
import sys
import re


if (len(sys.argv) < 2):
    raise Exception("Usage : python fitsToBp.py <filename> ")

filename = sys.argv[1]

os.system("rm -rf casa.bp")

with adios2.open("casa.bp", "w", "BP5") as fh:

    # Convert the FITS into a BP file
    with fits.open(filename) as hdul:
        for hdu in hdul:
            hdr = hdu.header
            for key in hdr.keys():
                if(not key):
                    continue
                if(re.search("^NAXIS.*$", key)):
                    continue
                if((type(hdr[key]) is int) or (type(hdr[key]) is float)):
                    fh.write(key, np.array(hdr[key]))
                else:
                    fh.write(key, str(hdr[key]))

            data = hdu.data
            # Endian conversion
            # print(data.dtype.name + "printing data type")
            data = np.ascontiguousarray(hdu.data, dtype=data.dtype.name)

            # for 3d image cube, convert to 4d cube
            if("NAXIS4" not in hdr.keys()):
                fh.write("NAXIS",  np.array(4))
                fh.write("NAXIS1",  np.array(hdr["NAXIS1"]))
                fh.write("NAXIS2",  np.array(hdr["NAXIS2"]))
                fh.write("NAXIS3",  np.array(hdr["NAXIS3"]))
                fh.write("NAXIS4",  np.array(1))
                data = data.reshape(
                    1, hdr["NAXIS3"], hdr["NAXIS2"], hdr["NAXIS1"])

            # for 4d image, where 3rd and 4th axis needs to be swapped
            elif((hdr["NAXIS4"] != 1) and (hdr["NAXIS3"] == 1)):
                fh.write("NAXIS",  np.array(4))
                fh.write("NAXIS1",  np.array(hdr["NAXIS1"]))
                fh.write("NAXIS2",  np.array(hdr["NAXIS2"]))
                fh.write("NAXIS3",  np.array(hdr["NAXIS4"]))
                fh.write("NAXIS4",  np.array(1))
                data = data.reshape(
                    1, hdr["NAXIS4"], hdr["NAXIS2"], hdr["NAXIS1"])

            size = list(data.shape)

            start = list(np.zeros(data.ndim, dtype=np.int32))
            # print(data.size)

            fh.write("data", data, size, start, size, end_step=True)
