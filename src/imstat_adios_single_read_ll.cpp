#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <adios2.h>

#include "helper.hpp"

// This will work only on 4d images with dimension of polarisation axis 1
int main(void)
{
  int64_t naxis;
  int64_t naxes[4];
  adios2::ADIOS adios;

  adios2::IO io = adios.DeclareIO("imstat_adios_reader");
  // all ranks opening the bp file have access to the entire metadata
  adios2::Engine reader = io.Open("casa.bp", adios2::Mode::ReadRandomAccess);

  getImageDimensions_ll(io, reader, naxis, naxes);

  size_t spat_size = naxes[0] * naxes[1];

  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
         "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
         "Max");
  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
         "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
         "mJy/beam", "mJy/beam");

  adios2::Variable<float> varData = io.InquireVariable<float>("data");
  std::vector<float> data;
  reader.Get(varData, data, adios2::Mode::Sync);

  /* process image one channel at a time; increment channel # in each loop */
  for (int64_t channel = 0; channel < naxes[2]; channel++)
  {
    printImageStats(data, channel * spat_size, (channel + 1) * spat_size, channel + 1);
  }

  reader.Close();
}
