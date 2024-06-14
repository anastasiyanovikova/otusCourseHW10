#include "bulk_server.h"


int main(int argc, char const *argv[])
{
  if(argc <= 2)
  {
    std::cout << "port bulkNumber" <<std::endl;
    return 0;
  } 
  unsigned short port = atoi (argv [1]);
  std::size_t N = atoi (argv [2]);

  asio::io_context context;
  Server server {context, port, N};
  server.accept();
  context.run();
}