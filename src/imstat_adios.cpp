#include "fitsio.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <adios2.h>

int main(int argc, char *argv[])
{
  int status = 0; /* CFITSIO status value MUST be initialized to zero! */
  int hdutype, naxis;
  long naxes[4];
  float *pix;

  // TODO: use metadata to determine hdu type. Currently assuming that its an Image, not table
  hdutype = IMAGE_HDU;
  adios2::fstream in("casa.bp", adios2::fstream::in);

  adios2::fstep inStep;

  while (adios2::getstep(in, inStep))
  {
    const std::size_t currentStep = inStep.current_step();
    // get image count of dimensions by reading the BP file variables
    const std::vector<std::string> s_numAxis = inStep.read<std::string>("NAXIS");

    std::cout << "Found numAxis:  " << s_numAxis.front() << " in currentStep "
              << currentStep << std::endl;

    naxis = std::stoi(s_numAxis.front());

    // get image dimensions by reading the BP file variables
    for (int i = 1; i <= naxis; i++)
    {
      std::string search_var = "NAXIS" + std::to_string(i);
      const std::vector<std::string> s_axis = inStep.read<std::string>(search_var);

      std::cout << "Found NAXIS" << std::to_string(i) << " :  " << s_axis.front() << " in currentStep "
                << currentStep << std::endl;

      naxes[i - 1] = std::stoi(s_axis.front());
    }
    std::cout << std::endl;

    // TODO: Check if both 4d and 3d compatibility is needed
    // if (status || naxis != 4)
    // {
    //   printf("Error: NAXIS = %d.  Only 4-D images are supported.\n", naxis);
    //   return (1);
    // }

    if (naxes[2] == 1)
    {
      long temp = naxes[2];
      naxes[2] = naxes[3];
      naxes[3] = temp;
    }

    // if (naxes[3] != 1)
    // {
    //   printf("Error: Polarisation axis must be 1\n");
    //   return 1;
    // }

    size_t spat_size = naxes[0] * naxes[1];
    pix = (float *)malloc(spat_size *
                          sizeof(float)); /* memory for 1 spatial channel */

    if (pix == NULL)
    {
      printf("Memory allocation error\n");
      return (1);
    }

    printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
           "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
           "Max");
    printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
           "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
           "mJy/beam", "mJy/beam");

    const std::vector<float> data = inStep.read<float>("data");

    /* process image one row at a time; increment row # in each loop */
    for (int channel = 0; channel < naxes[2]; channel++)
    {
      int startIndex = channel * spat_size;

      for (size_t i = 0; i < spat_size; i++)
      {
        pix[i] = data[startIndex + i];
      }

      float sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
      float valid_pix = 0;
      for (size_t ii = 0; ii < spat_size; ii++)
      {
        float val = pix[ii];
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
             channel, 1.0f, meanval, 0.0f, 0.0f, 0.0f, 0.0f, minval, maxval);
      // printf("  sum of pixels = %g\n", sum);
      // printf("  mean value    = %g\n", meanval);
      // printf("  minimum value = %g\n", minval);
      // printf("  maximum value = %g\n", maxval);
    }
  }
}
