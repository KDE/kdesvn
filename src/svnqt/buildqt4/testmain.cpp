#include "client.hpp"
#include "repository.hpp"

int main(int,char**)
{
  svn::Client::getobject(0,0);
  svn::repository::Repository rep(0L);
  return 1;
}
