#include <mpi.h>
#include <adios2.h>
#define ADIOS2_USE_MPI

int main()
{int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

size_t SelX = 0 , SelY = 0;
// Selection Window from application, std::size_t
const adios2::Dims start{0, 0};
const adios2::Dims count{SelX, SelY};

if( rank == 0)
{
   // if only one rank is active use MPI_COMM_SELF
   adios2::fstream iStream("cfd.bp", adios2::fstream::in, MPI_COMM_SELF);

   adios2::fstep iStep;
   while (adios2::getstep(iStream, iStep))
   {
       if( iStep.current_step() == 0 )
       {
           const std::size_t sizeOriginal = iStep.read<std::size_t>("size");
       }
       const double physicalTime = iStream.read<double>( "physicalTime");
       const std::vector<float> temperature = iStream.read<float>( "temperature", start, count );
       const std::vector<float> pressure = iStream.read<float>( "pressure", start, count );
   }
   // Don't forget to call close!
   iStream.close();
}}