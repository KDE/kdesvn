#include "client.hpp"
#include "repository.hpp"
#include "context.hpp"
#include "datetime.hpp"

int main(int,char**)
{
  svn::Client::getobject(0,0);
  svn::repository::Repository rep(0L);
  svn::ContextP myContext = new svn::Context();

  svn::DateTime d1(9),d2(20);

  if (d1<d2) {

  }
  if (d1>d2) {
  }
  return 1;
}
