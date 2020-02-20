
#include "parse_input.h"

#include <symphony.h>
#include <OsiSymSolverInterface.hpp>
#include <CoinPackedVector.hpp>


#include <string_view>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <fstream>

using namespace std::string_view_literals;
using namespace std::string_literals;

/*
 *
 * Let x \in Rn, s.t. x_i is number of pizzas of type i we order,
 * and let s \in Rn, s.t. s_i is the number of slices of pizza of type i
 * we want to
 *         minimize                    x * -s = sum_i -x_i * s_i
 *         s.t.        -max_slices <= -s * x <= infty                 <--- row bounds
 *                               0 <= x_i <= 1, x_i integers       <--- col bounds
 */


int main(int argc, char** argv){

    std::string input_path(argv[1]);

    std::vector<double> slices;
    double max_slice;

    using my_clock = std::chrono::steady_clock;
    using my_dur = std::chrono::duration<double, std::ratio<1>>;

    auto b = my_clock::now();
    parse_input_threaded(input_path, slices, max_slice, 1);

    OsiSymSolverInterface si;

    for(const auto s : slices){
        CoinPackedVector col;
        col.insert(0, -s);
        si.addCol(col, 0., 1., -s);
    }

    for (int j = 0; j < si.getNumRows(); ++j) {
        si.setRowLower(j, -max_slice);
        si.setRowUpper(j, 0.);
    }

    //set everything to integer constraints
    for (int i = 0; i < si.getNumCols(); ++i)
        si.setInteger(i);

    //solve
    si.initialSolve();

    auto e = my_clock::now();
    my_dur dur = e - b;

    std::cout << dur.count() << "sec" << std::endl;

    auto obj = si.getObjValue();
    std::cout << "solution " << -obj << '/' << max_slice << std::endl;
    std::cout << "solution is proven optimal: " << (si.isProvenOptimal() ? "yes" : "no") << std::endl;

    std::vector<double> sol(si.getColSolution(),si.getColSolution() + si.getNumCols());
    int num_slices = std::accumulate(sol.begin(), sol.end(), 0);

    std::string prefix(input_path.begin(), input_path.end() - 3);
    std::ofstream file(prefix + ".out"s);
    file << num_slices << std::endl;

    for (int k = 0; k < sol.size(); ++k) {
        if(sol[k])
            file << k << ' ';
    }
}
