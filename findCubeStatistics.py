#!/usr/bin/env python
#
# findCubeStatistics.py
#
# A python script to measure various statistics as a function of
# channel for a given spectral cube.
#
# @copyright (c) 2018 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Matthew Whiting <Matthew.Whiting@csiro.au>
#

import include.cubestatsHelpers as cs
from argparse import ArgumentParser
import pylab as plt
from mpi4py import MPI
import os
import numpy as np
import casacore.images.image as im
import matplotlib
matplotlib.use('Agg')

#############
if __name__ == '__main__':

    parser = ArgumentParser(description="An MPI-distributed program to find the statistics of a spectral cube as a function of channel. The channels of the cube are distributed evenly across all available ranks, and their statistics are calculated independently, then sent to the rank-0 process for output as a catalogue and a graphical plot.")
#    parser.add_argument("-c","--cube", dest="cube", type="string", default="", help="Input spectral cube or image [default: %(default)s]")
    parser.add_argument("-c", "--cube", dest="cube")
#                            help="Input spectral cube or image")
    parser.add_argument("-n", "--nocalc", dest="nocalc", action="store_true")
    parser.add_argument("-m", "--maxclip", dest="maxclip",
                        type=float, default=1.e5)
    parser.add_argument("--norobust", dest="norobust", action="store_true")

    args = parser.parse_args()

    if args.cube == '':
        print("Spectral cube not given - you need to use the -c option")
        exit(0)

    if not os.access(args.cube, os.F_OK):
        print("Could not access cube %s. Exiting." % args.cube)
        exit(1)

    robust = not args.norobust

    # Define the output filenames, based on the cube name.
    # cubeTag is just the filename, without any extension (like .fits)
    # or leading path
    cubeNoFITS = args.cube
    if cubeNoFITS[-5:] == '.fits':
        cubeNoFITS = cubeNoFITS[:-5]
    cubeTag = os.path.basename(cubeNoFITS)
    cubeDir = os.path.dirname(args.cube)
    if cubeDir == '':
        cubeDir = '.'
    catalogue = '%s/cubeStats-%s.txt' % (cubeDir, cubeTag)
    graph = '%s/cubePlot-%s.png' % (cubeDir, cubeTag)

    # get the MPI rank
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    size = comm.Get_size()

    cube = im(args.cube)
    unit = cube.unit()
    scale = 1
    if unit[:2] == 'Jy':
        scale = 1000.
        unit = 'm'+unit
    shape = cube.shape()
    blc = np.zeros_like(shape)
    trc = np.array(shape)-1
    coords = cube.coordinates()
    spInd = cs.findSpectralIndex(coords)
    specSize = cs.findSpectralSize(shape, coords)
    spatSize = cs.findSpatialSize(coords)
    freq = cs.getFreqAxis(cube)/1.e6

    useCasacoreStats = True
    stats = cs.statsCollection(freq, comm, useCasacoreStats, robust)
    stats.setScaleFactor(scale)
    stats.setUnits(unit)

    if not args.nocalc:

        for i in range(specSize):

            if i % size == rank:

                blc[spInd] = i
                trc[spInd] = i
                try:
                    stats.calculate(cube, i, blc.tolist(), trc.tolist())
                except:
                    print(
                        'findCubeStats.py: Error for channel %d - setting stats to zero' % i)

        stats.gather(comm)

    if rank == 0:

        if args.nocalc:
            stats.read(catalogue)
        else:
            stats.scale()
            stats.write(catalogue)

        std = stats.getStd()
        madfm = cs.madfmToSigma(stats.getMadfm())
        minval = stats.getMinval()
        maxval = stats.getMaxval()
        onepc = stats.getOnepc()

        fig = plt.figure(1, figsize=(8, 8))

        plt.subplot(211)
        plt.plot(freq, maxval, label='max')
        plt.plot(freq, minval, label='min')
        if robust:
            plt.plot(freq, onepc, label='1-percentile')
        if args.maxclip > 0:
            ymax = maxval[abs(maxval) < args.maxclip].max()
            ymin = minval[abs(minval) < args.maxclip].min()
        else:
            ymax = maxval.max()
            ymin = minval.min()
        width = ymax-ymin
        ymax = ymax + 0.1*width
        ymin = ymin - 0.1*width
        plt.ylim(ymin, ymax)
        plt.xlabel('Frequency [MHz]')
        plt.ylabel('Flux value [%s]' % unit)
        plt.legend(loc='center right')

        plt.subplot(212)
        plt.plot(freq, std, label='Std. Dev')
        if robust:
            plt.plot(freq, madfm, label='scaled MADFM')
        ymax = std.max()
        ymin = std.min()
        goodCells = cs.getGoodCells(std)
        if std[goodCells].size > 0:
            ymax = std[goodCells].max()
            ymin = std[goodCells].min()
        if robust:
            goodCells = cs.getGoodCells(madfm)
            if madfm[goodCells].size > 0:
                ymax = np.max([ymax, madfm[goodCells].max()])
                ymin = np.min([ymin, madfm[goodCells].min()])
        width = ymax-ymin
        ymax = ymax + 0.1*width
        ymin = ymin - 0.1*width
        plt.ylim(ymin, ymax)
        plt.xlabel('Frequency [MHz]')
        plt.ylabel('Flux value [%s]' % unit)
        plt.legend(loc='lower right')

        fig.suptitle(args.cube)
        fig.savefig(graph)