#include "client.hpp"
#include "repository.hpp"
#include "context.hpp"

int main(int,char**)
{
  svn::Client::getobject(0,0);
  svn::repository::Repository rep(0L);
  svn::ContextP myContext = new svn::Context();
  return 1;
}
