#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include <adios2.h>

// This will work only on 4d images with dimension of polarisation axis 1
int main(int argc, char *argv[])
{
    int64_t naxis;
    int64_t naxes[4];

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    adios2::fstream inStream("casa.bp", adios2::fstream::in,
                             MPI_COMM_WORLD);

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

    size_t spat_size = naxes[0] * naxes[1];

    if (rank == 0)
    {
        printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", "Channel",
               "Frequency", "Mean", "Std", "Median", "MADFM", "1%ile", "Min",
               "Max");
        printf("#%7s %15s %10s %10s %10s %10s %10s %10s %10s\n", " ", "MHz",
               "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam", "mJy/beam",
               "mJy/beam", "mJy/beam");
    }

    /* process image one channel at a time; increment channel # in each loop */
    for (int channel = 0; channel < naxes[2]; channel++)
    {
        if (channel % size == rank)
        {
            const adios2::Dims start = {static_cast<std::size_t>(0), static_cast<std::size_t>(channel), static_cast<std::size_t>(0), static_cast<std::size_t>(0)};

            const adios2::Dims count = {static_cast<std::size_t>(1), static_cast<std::size_t>(1), static_cast<std::size_t>(naxes[0]), static_cast<std::size_t>(naxes[1])};

            const std::vector<float> data = inStream.read<float>("data", start, count);

            float sum = 0., meanval = 0., minval = 1.E33, maxval = -1.E33;
            float valid_pix = 0;

            for (size_t ii = 0; ii < spat_size; ii++)
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
    }

    inStream.close();

    MPI_Finalize();
    return 0;
}
