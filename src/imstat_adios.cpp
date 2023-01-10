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

  adios2::fstream inStream("casa.bp", adios2::fstream::in_random_access);

  getImageDimensions(inStream, naxis, naxes);

  size_t spat_size = naxes[0] * naxes[1];

  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
         "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
         "Max");
  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
         "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
         "mJy/beam", "mJy/beam");

  /* process image one channel at a time; increment channel # in each loop */
  for (int64_t channel = 0; channel < naxes[2]; channel++)
  {
    const adios2::Dims start = {static_cast<std::size_t>(0), static_cast<std::size_t>(channel), static_cast<std::size_t>(0), static_cast<std::size_t>(0)};

    const adios2::Dims count = {static_cast<std::size_t>(1), static_cast<std::size_t>(1), static_cast<std::size_t>(naxes[0]), static_cast<std::size_t>(naxes[1])};

    const std::vector<float> data = inStream.read<float>("data", start, count);

    printImageStats(data, 0, spat_size, channel + 1);
  }

  inStream.close();
}
