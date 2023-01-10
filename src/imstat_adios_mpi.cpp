#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <adios2.h>
#include <mpi.h>

#include "helper.hpp"

// This will work only on 4d images with dimension of polarisation axis 1
int main(int argc, char *argv[])
{
    int64_t naxis;
    int64_t naxes[4];

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    adios2::fstream inStream("casa.bp", adios2::fstream::in_random_access,
                             MPI_COMM_WORLD);

    getImageDimensions(inStream, naxis, naxes);

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
    for (int64_t channel = 0; channel < naxes[2]; channel++)
    {
        if (channel % size == rank)
        {
            const adios2::Dims start = {static_cast<std::size_t>(0), static_cast<std::size_t>(channel), static_cast<std::size_t>(0), static_cast<std::size_t>(0)};

            const adios2::Dims count = {static_cast<std::size_t>(1), static_cast<std::size_t>(1), static_cast<std::size_t>(naxes[0]), static_cast<std::size_t>(naxes[1])};

            std::vector<float> data = inStream.read<float>("data", start, count);

            printImageStats(data, 0, spat_size, channel + 1);
        }
    }

    inStream.close();

    MPI_Finalize();
    return 0;
}
