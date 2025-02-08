#include "Minimal.hpp"
#include "Runner.hpp"

int main()
{
    static Runner<Minimal> r;
    r.execute();
    return 0;
}
