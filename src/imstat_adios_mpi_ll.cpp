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

    adios2::ADIOS adios("config.xml", MPI_COMM_WORLD);

    adios2::IO io = adios.DeclareIO("imstat_adios_reader");
    // all ranks opening the bp file have access to the entire metadata
    adios2::Engine reader = io.Open("casa.bp", adios2::Mode::ReadRandomAccess);

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

            adios2::Variable<float> varData = io.InquireVariable<float>("data");
            std::vector<float> data;
            if (varData)
            {
                adios2::Box<adios2::Dims> selection = {start, count};
                varData.SetSelection(selection);
                reader.Get(varData, data, adios2::Mode::Sync);
            }

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

    reader.Close();

    MPI_Finalize();
    return 0;
}
