#ifndef HELPER_HPP
#define HELPER_HPP

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include <vector>

#include <adios2.h>
#include <mpi.h>

void getImageDimensions(adios2::fstream &inStream, int64_t &naxis, int64_t *naxes)
{
  // get image count of dimensions by reading the BP file variables
  const std::vector<int64_t> s_numAxis = inStream.read<int64_t>("NAXIS");
  naxis = s_numAxis.front();

  // get image dimensions by reading the BP file variables
  for (int i = 1; i <= naxis; i++)
  {
    std::string search_var = "NAXIS" + std::to_string(i);
    const std::vector<int64_t> s_axis = inStream.read<int64_t>(search_var);
    naxes[i - 1] = s_axis.front();
  }
}

void getImageDimensions_ll(adios2::IO io, adios2::Engine reader, int64_t &naxis, int64_t *naxes)
{
  // get image count of dimensions by reading the BP file variables
  adios2::Variable<int64_t> s_numAxis =
      io.InquireVariable<int64_t>("NAXIS");

  reader.Get(s_numAxis, naxis, adios2::Mode::Sync);

  // get image dimensions by reading the BP file variables
  for (int i = 1; i <= naxis; i++)
  {
    std::string search_var = "NAXIS" + std::to_string(i);

    adios2::Variable<int64_t> s_axis =
        io.InquireVariable<int64_t>(search_var);

    reader.Get(s_axis, naxes[i - 1], adios2::Mode::Sync);
  }
}

void printImageStats(const std::vector<float> &data, size_t start, size_t end, int64_t channelID)
{
  float sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
  float valid_pix = 0;

  for (size_t ii = start; ii < end; ii++)
  {
    float val = data[ii];
    valid_pix += isnan(val) ? 0 : 1;
    val = isnan(val) ? 0.0 : val;

    sum += val; /* accumlate sum */
    if (val < minval)
      minval = val; /* find min and  */
    if (val > maxval)
      maxval = val; /* max values    */
  }
  meanval = sum / valid_pix;

  meanval *= 1000.0;
  minval *= 1000.0;
  maxval *= 1000.0;

  printf("%8lld %15.6f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
         channelID, 1.0f, meanval, 0.0f, 0.0f, 0.0f, 0.0f, minval, maxval);
}

#endif