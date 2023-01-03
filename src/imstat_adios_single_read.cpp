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

    adios2::fstream in("casa.bp", adios2::fstream::in);

    adios2::fstep inStep;

    while (adios2::getstep(in, inStep))
    {
        const std::size_t currentStep = inStep.current_step();
        // get image count of dimensions by reading the BP file variables
        const std::vector<int64_t> s_numAxis = inStep.read<int64_t>("NAXIS");

        std::cout << "Found numAxis:  " << s_numAxis.front() << " in currentStep "
                  << currentStep << std::endl;

        naxis = s_numAxis.front();

        // get image dimensions by reading the BP file variables
        for (int i = 1; i <= naxis; i++)
        {
            std::string search_var = "NAXIS" + std::to_string(i);
            const std::vector<int64_t> s_axis = inStep.read<int64_t>(search_var);

            std::cout << "Found NAXIS" << std::to_string(i) << " :  " << s_axis.front() << " in currentStep "
                      << currentStep << std::endl;

            naxes[i - 1] = s_axis.front();
        }
        std::cout << std::endl;

        size_t spat_size = naxes[0] * naxes[1];

        printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
               "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
               "Max");
        printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
               "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
               "mJy/beam", "mJy/beam");

        const std::vector<float> data = inStep.read<float>("data");

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
                   channel+1, 1.0f, meanval, 0.0f, 0.0f, 0.0f, 0.0f, minval, maxval);
        }
        in.end_step();
    }

    in.close();
}
