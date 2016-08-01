extern "C" {
#include "shared.h"
#include "static.h"
}
#include "exe.hxx"
#include "object.hxx"
#include "shared.hxx"
#include "static.hxx"

int main()
{
  c_shared();
  c_static();
  cxx_exe();
  cxx_shared();
  cxx_static();
  cxx_object();
}
