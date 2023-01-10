#include "fitsio.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>

int main(int argc, char *argv[])
{
  fitsfile *fptr; /* FITS file pointer */
  int status = 0; /* CFITSIO status value MUST be initialized to zero! */
  int hdutype, naxis;
  long naxes[4], fpixel[4];
  double *pix;

  MPI_Init(&argc, &argv);

  int rank, size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc != 2)
  {
    printf("Usage: imstat image \n");
  }

  if (fits_open_image(&fptr, argv[1], READONLY, &status))
  {
    printf("Error: can not open fits file\n");
    return (1);
  }

  if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU)
  {
    printf("Error: this program only works on images, not tables\n");
    return (1);
  }

  fits_get_img_dim(fptr, &naxis, &status);
  fits_get_img_size(fptr, 4, naxes, &status);

  if (status || naxis != 4)
  {
    naxis = 4;
    naxes[3] = 1;
  }
  else if (naxes[2] == 1 && naxes[3] != 1)
  {
    long temp = naxes[2];
    naxes[2] = naxes[3];
    naxes[3] = temp;
  }

  size_t spat_size = naxes[0] * naxes[1];
  pix = (double *)malloc(spat_size *
                         sizeof(double)); /* memory for 1 spatial channel */

  if (pix == NULL)
  {
    printf("Memory allocation error\n");
    return (1);
  }

  fpixel[0] = 1; /* read starting with first pixel in each row */
  fpixel[1] = 1;
  fpixel[3] = 1; /* Polarisation axis */

  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
         "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
         "Max");
  printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
         "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
         "mJy/beam", "mJy/beam");

  /* process image one row at a time; increment row # in each loop */
  for (int channel = 1; channel <= naxes[2]; channel++)
  {
    if (channel % size == rank)
    {
      fpixel[2] = channel;
      /* give starting pixel coordinate and number of pixels to read */
      if (fits_read_pix(fptr, TDOUBLE, fpixel, spat_size, 0, pix, 0, &status))
        break; /* jump out of loop on error */

      double sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
      double valid_pix = 0;
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
    }
  }

  free(pix);
  fits_close_file(fptr, &status);

  if (status)
  {
    fits_report_error(stderr, status); /* print any error message */
  }

  MPI_Finalize();

  return 0;
}
