#include <stdio.h>
// NOT ALL exceptions are impl'd in this toolchain
namespace std
{
  void __throw_bad_alloc()
  {
    printf("Unable to allocate memory\n");
  }
  void __throw_length_error(char const *e)
  {
    printf("Length Error :%s\n", e);
  }
  void __throw_out_of_range_fmt(char const *e, ...)
  {
    printf("Length Error :%s\n", e);
  }
}