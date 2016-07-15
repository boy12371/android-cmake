extern "C" {
#include "shared.h"
#include "static.h"
}
#include "exe.hpp"
#include "shared.hpp"
#include "static.hpp"

int main() {
  c_shared();
  c_static();
  cpp_exe();
  cpp_shared();
  cpp_static();
}
