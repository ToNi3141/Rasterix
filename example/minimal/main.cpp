#include "Runner.hpp"
#include "Minimal.hpp"

int main()
{
    static Runner<Minimal> r;
    r.execute();
    return 0;
}