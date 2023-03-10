#!/usr/bin/env python
#
# cubestatsHelpers.py
#
# Python functions and classes to help with the measurement and handling of cube statistics
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
#############

import numpy as np


def madfmToSigma(madfm):
    return madfm/0.6744888


def findSpectralIndex(coords):
    return coords.get_coordinate('spectral').get_image_axis()


def findSpectralSize(shape, coords):
    specIndex = findSpectralIndex(coords)
    return shape[specIndex]


def findSpatialSize(coords):

    for i in range(3):
        if f'direction{i}' in coords.dict():
            label = f'direction{i}'
    return coords.dict()[label]['_axes_sizes']


def getFreqAxis(cube):
    coords = cube.coordinates()
    shape = cube.shape()
    specDim = shape[findSpectralIndex(coords)]
    specCoo = coords.get_coordinate('spectral')
    freq = (np.arange(specDim)-specCoo.get_referencepixel()) * \
        specCoo.get_increment() + specCoo.get_referencevalue()
    return freq


def getGoodCells(arr):
    med = np.nanmedian(arr)
    q75, q25 = np.nanpercentile(arr, [75, 25])
    iqr = q75-q25
    isvalid = (~np.isnan(arr)) * (~np.isinf(arr))
    issmall = (abs(arr) < 1000.)
    isgood = (abs(arr-med) < 5.*iqr)
    return isvalid*isgood*issmall


##############

class stat:
    def __init__(self, size, comm):
        self.rank = comm.Get_rank()
        self.nranks = comm.Get_size()
        self.size = size
        self.stat = np.zeros(self.size, dtype='f')
        self.fullStat = None
        self.finalStat = None
        if self.rank == 0:
            self.fullStat = np.empty([self.nranks, self.size], dtype='f')

    def assign(self, chan, statsDict, label):
        self.stat[chan] = statsDict[label]

    def set(self, chan, value):
        self.stat[chan] = value

    def makeFinal(self):
        self.finalStat = self.stat

    def gather(self, comm):
        comm.Gather(self.stat, self.fullStat)
        if self.rank == 0:
            self.finalStat = np.zeros(self.size)
            for i in range(self.size):
                self.finalStat[i] = self.fullStat[:, i].sum()

    def __getitem__(self, i):
        return self.finalStat[i]


class statsCollection:
    def __init__(self, freq, comm, useCasacoreStats, robust):
        self.rank = comm.Get_rank()
        self.freq = freq
        self.useCasa = useCasacoreStats
        self.robust = robust
        self.scaleFactor = 1.
        self.units = 'Jy/beam'
        size = freq.size
        self.rms = stat(size, comm)
        self.std = stat(size, comm)
        self.mean = stat(size, comm)
        self.onepc = stat(size, comm)
        self.median = stat(size, comm)
        self.madfm = stat(size, comm)
        self.maxval = stat(size, comm)
        self.minval = stat(size, comm)

    def calculate(self, cube, chan, blc, trc):
        if self.useCasa:
            sub = cube.subimage(blc=blc, trc=trc)
            subStats = sub.statistics(robust=self.robust)
            arr = sub.getdata()
            msk = ~sub.getmask()
            if self.robust:
                subStats['onepc'] = np.percentile(arr[msk], 1)
        else:
            arr = cube.getdata(blc=blc, trc=trc)
            msk = ~cube.getmask(blc=blc, trc=trc)
            subStats = {}
            subStats['mean'] = arr[msk].mean()
            subStats['sigma'] = arr[msk].std()
            subStats['rms'] = np.sqrt(np.mean(arr[msk]**2))
            subStats['min'] = arr[msk].min()
            subStats['max'] = arr[msk].max()
            if self.robust:
                percentiles = np.percentile(arr[msk], [50, 1])
                print(percentiles)
                print(percentiles[0])
                print(percentiles[1])
                subStats['median'] = percentiles[0]
                subStats['medabsdevmed'] = np.median(abs(arr[msk]-med))
                subStats['onepc'] = percentiles[1]

        self.assign(chan, subStats)

    def assign(self, chan, stats):
        self.rms.assign(chan, stats, 'rms')
        self.std.assign(chan, stats, 'sigma')
        self.mean.assign(chan, stats, 'mean')
        self.maxval.assign(chan, stats, 'max')
        self.minval.assign(chan, stats, 'min')
        if self.robust:
            self.onepc.assign(chan, stats, 'onepc')
            self.median.assign(chan, stats, 'median')
            self.madfm.assign(chan, stats, 'medabsdevmed')

    def gather(self, comm):
        # Gather everything to the master
        self.rms.gather(comm)
        self.std.gather(comm)
        self.mean.gather(comm)
        self.onepc.gather(comm)
        self.median.gather(comm)
        self.madfm.gather(comm)
        self.maxval.gather(comm)
        self.minval.gather(comm)

    def getRMS(self): return self.rms.finalStat
    def getStd(self): return self.std.finalStat
    def getMean(self): return self.mean.finalStat
    def getOnepc(self): return self.onepc.finalStat
    def getMedian(self): return self.median.finalStat
    def getMadfm(self): return self.madfm.finalStat
    def getMaxval(self): return self.maxval.finalStat
    def getMinval(self): return self.minval.finalStat

    def setScaleFactor(self, scalefactor):
        self.scaleFactor = scalefactor

    def setUnits(self, unit):
        self.unit = unit

    def scale(self):
        self.rms.finalStat = self.rms.finalStat * self.scaleFactor
        self.std.finalStat = self.std.finalStat * self.scaleFactor
        self.mean.finalStat = self.mean.finalStat * self.scaleFactor
        self.onepc.finalStat = self.onepc.finalStat * self.scaleFactor
        self.median.finalStat = self.median.finalStat * self.scaleFactor
        self.madfm.finalStat = self.madfm.finalStat * self.scaleFactor
        self.maxval.finalStat = self.maxval.finalStat * self.scaleFactor
        self.minval.finalStat = self.minval.finalStat * self.scaleFactor

    def write(self, catalogue):
        fout = open(catalogue, 'w')
        if self.robust:
            fout.write('#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n' % (
                'Channel', 'Frequency', 'Mean', 'Std', 'Median', 'MADFM', '1%ile', 'Min', 'Max'))
            fout.write('#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n' % (
                ' ', 'MHz', self.unit, self.unit, self.unit, self.unit, self.unit, self.unit, self.unit))
            for i in range(self.freq.size):
                fout.write('%8d %15.6f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n' % (
                    i, self.freq[i], self.mean[i], self.std[i], self.median[i], self.madfm[i], self.onepc[i], self.minval[i], self.maxval[i]))

        else:
            fout.write('#%7s %15s %10s %10s %10s %10s\n' %
                       ('Channel', 'Frequency', 'Mean', 'Std', 'Min', 'Max'))
            fout.write('#%7s %15s %10s %10s %10s %10s\n' %
                       (' ', 'MHz', self.unit, self.unit, self.unit, self.unit))
            for i in range(self.freq.size):
                fout.write('%8d %15.6f %10.3f %10.3f %10.3f %10.3f\n' % (
                    i, self.freq[i], self.mean[i], self.std[i], self.minval[i], self.maxval[i]))

        fout.close()

    def read(self, catalogue):
        if self.rank == 0:
            fin = open(catalogue, 'r')
            for line in fin:
                if line[0] != '#':
                    chan = int(line.split()[0])
                    # self.freq.set(chan,float(line.split()[1]))
                    self.mean.set(chan, float(line.split()[2]))
                    self.std.set(chan, float(line.split()[3]))
                    if self.robust:
                        self.median.set(chan, float(line.split()[4]))
                        self.madfm.set(chan, float(line.split()[5]))
                        self.onepc.set(chan, float(line.split()[6]))
                        self.minval.set(chan, float(line.split()[7]))
                        self.maxval.set(chan, float(line.split()[8]))
                    else:
                        self.minval.set(chan, float(line.split()[4]))
                        self.maxval.set(chan, float(line.split()[5]))
            self.mean.makeFinal()
            self.std.makeFinal()
            self.median.makeFinal()
            self.madfm.makeFinal()
            self.onepc.makeFinal()
            self.minval.makeFinal()
            self.maxval.makeFinal()
