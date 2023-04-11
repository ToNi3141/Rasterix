#include "Runner.hpp"
#include "StencilShadow.hpp"

int main()
{
    static Runner<StencilShadow> r;
    r.execute();
    return 0;
}