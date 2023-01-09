#ifndef HELPER_HPP
#define HELPER_HPP

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include <vector>

#include <adios2.h>
#include <mpi.h>

void broadcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator, int tag) {
  int world_rank;
  MPI_Comm_rank(communicator, &world_rank);
  int world_size;
  MPI_Comm_size(communicator, &world_size);

  if (world_rank == root) {
    // If we are the root process, send our data to everyone
    int i;
    for (i = 0; i < world_size; i++) {
      if (i != world_rank) {
        MPI_Send(data, count, datatype, i, tag, communicator);
      }
    }
  } else {
    // If we are a receiver process, receive the data from the root
    MPI_Recv(data, count, datatype, root, tag, communicator, MPI_STATUS_IGNORE);
  }
}


void getImageDimensions(adios2::fstream &inStream, int &naxis, int* naxes)
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

void getImageDimensions_ll(adios2::IO io, adios2::Engine reader, int &naxis, int* naxes)
{
    // get image count of dimensions by reading the BP file variables
    adios2::Variable<int64_t> s_numAxis =
        io.InquireVariable<int64_t>("NAXIS");

    int64_t naxis_t;
    reader.Get(s_numAxis, naxis_t, adios2::Mode::Sync);
    naxis = static_cast<int>(naxis_t);

    // get image dimensions by reading the BP file variables
    for (int i = 1; i <= naxis; i++)
    {
        std::string search_var = "NAXIS" + std::to_string(i);

        adios2::Variable<int64_t> s_axis =
            io.InquireVariable<int64_t>(search_var);

        int64_t axis_t;    
        reader.Get(s_axis, axis_t, adios2::Mode::Sync);

        naxes[i-1] = static_cast<int>(axis_t);
    }
}



void printImageStats(const std::vector<float> &data, size_t spat_size, int channel)
{
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

#endif