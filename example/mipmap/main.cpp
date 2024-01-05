#include "Runner.hpp"
#include "Mipmap.hpp"

int main()
{
    static Runner<Mipmap> r;
    r.execute();
    return 0;
}