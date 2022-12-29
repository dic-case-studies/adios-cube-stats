#include "fitsio.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  fitsfile *fptr; /* FITS file pointer */
  int status = 0; /* CFITSIO status value MUST be initialized to zero! */
  int hdutype, naxis, ii;
  long naxes[4], totpix, fpixel[4];
  double *pix;

  if (argc != 2) {
    printf("Usage: imstat image \n");
    printf("\n");
    printf("Compute statistics of pixels in the input image\n");
    printf("\n");
    printf("Examples: \n");
    printf("  imstat  image.fits                    - the whole image\n");
    printf("  imarith 'image.fits[200:210,300:310]' - image section\n");
    printf("  imarith 'table.fits+1[bin (X,Y) = 4]' - image constructed\n");
    printf("     from X and Y columns of a table, with 4-pixel bin size\n");
    return (0);
  }

  if (!fits_open_image(&fptr, argv[1], READONLY, &status)) {
    if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU) {
      printf("Error: this program only works on images, not tables\n");
      return (1);
    }

    fits_get_img_dim(fptr, &naxis, &status);
    fits_get_img_size(fptr, 4, naxes, &status);

    if (status || naxis != 4) {
      printf("Error: NAXIS = %d.  Only 4-D images are supported.\n", naxis);
      return (1);
    }

    if (naxes[2] == 1) {
      long temp = naxes[2];
      temp = naxes[3];
      naxes[3] = temp;
    }

    if (naxes[3] != 1) {
      printf("Error: Polarisation axis must be 1\n");
      return 1;
    }

    size_t spat_size = naxes[0] * naxes[1];
    pix = (double *)malloc(spat_size *
                           sizeof(double)); /* memory for 1 spatial channel */

    if (pix == NULL) {
      printf("Memory allocation error\n");
      return (1);
    }

    totpix = naxes[0] * naxes[1] * naxes[2] * naxes[3];
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
    for (int channel = 1; channel <= naxes[2]; channel++) {
      fpixel[2] = channel;
      /* give starting pixel coordinate and number of pixels to read */
      if (fits_read_pix(fptr, TDOUBLE, fpixel, spat_size, 0, pix, 0, &status))
        break; /* jump out of loop on error */
      
      // if (channel == 1 || channel == 2) {
      //   int valid_pix = 0;
      //   for (ii = 0; ii < spat_size && valid_pix < 100; ii++) {
      //     float val = pix[ii];
      //     valid_pix += isnan(val) ? 0 : 1;
      //     if (isnan(val)) {
      //       continue;
      //     } else {
      //       printf("%d %f\n", ii, val);
      //     }
      //   }
      // }

      double sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
      double valid_pix = 0;
      for (ii = 0; ii < spat_size; ii++) {
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

    free(pix);
    fits_close_file(fptr, &status);
  }

  if (status) {
    fits_report_error(stderr, status); /* print any error message */
  } else {
    fprintf(stderr, "Statistics of %ld x %ld x %ld x %ld  image\n", naxes[0], naxes[1],
           naxes[2], naxes[3]);
  }

  return (status);
}
