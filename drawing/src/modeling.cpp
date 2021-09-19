#include "modeling.h"

int main()
{
	EulerSolver s(
	  {1, 2},
	  [](std::vector<double>) {
		  return std::vector{0., 0.};
	  },
	  0.01, 100);
}
