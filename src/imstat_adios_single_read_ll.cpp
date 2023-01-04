#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <adios2.h>

// This will work only on 4d images with dimension of polarisation axis 1
int main(int argc, char *argv[])
{
  int64_t naxis;
  int64_t naxes[4];
  adios2::ADIOS adios;

  adios2::IO io = adios.DeclareIO("imstat_adios_reader");
  // all ranks opening the bp file have access to the entire metadata
  adios2::Engine reader = io.Open("casa.bp", adios2::Mode::Read);

  // get image count of dimensions by reading the BP file variables
  adios2::Variable<int64_t> s_numAxis =
      io.InquireVariable<int64_t>("NAXIS");
  if (s_numAxis)
  {
    reader.Get(s_numAxis, naxis, adios2::Mode::Sync);
  }

  // get image dimensions by reading the BP file variables
  for (int i = 1; i <= naxis; i++)
  {
    std::string search_var = "NAXIS" + std::to_string(i);
    adios2::Variable<int64_t> s_axis =
        io.InquireVariable<int64_t>(search_var);
    if (s_axis)
    {
      reader.Get(s_axis, naxes[i - 1], adios2::Mode::Sync);
    }
  }
  std::cout << std::endl;

  size_t spat_size = naxes[0] * naxes[1];

  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
         "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
         "Max");
  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
         "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
         "mJy/beam", "mJy/beam");

  adios2::Variable<float> varData = io.InquireVariable<float>("data");
  std::vector<float> data;
  if (varData)
  {
    reader.Get(varData, data, adios2::Mode::Sync);
  }

  /* process image one channel at a time; increment channel # in each loop */
  for (int channel = 0; channel < naxes[2]; channel++)
  {
    float sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
    float valid_pix = 0;
    for (size_t ii = channel * spat_size; ii < (channel + 1) * spat_size; ii++)
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

    printf("%8d %15.6f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
           channel + 1, 1.0f, meanval, 0.0f, 0.0f, 0.0f, 0.0f, minval, maxval);
  }

  reader.Close();
}