/* mockturtle: C++ logic network library
 * Copyright (C) 2018  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
  \file refactor.hpp
  \brief Refactor

  \author Heinz Riener
*/

#pragma once

#include "../traits.hpp"
#include "../utils/progress_bar.hpp"
#include "../utils/stopwatch.hpp"
#include "reconv_cut2.hpp"

namespace mockturtle
{

struct refactor_params
{
  /*! \brief Maximum number of PIs of reconvergence-driven cuts. */
  uint32_t max_pis{8};

  /*! \brief Maximum fanout of a node to be considered as root. */
  uint32_t skip_fanout_limit_for_roots{1000};

  /*! \brief Show progress. */
  bool progress{false};
  
  /*! \brief Be verbose. */
  bool verbose{true};
};

struct refactor_stats
{
  /*! \brief Total runtime. */
  stopwatch<>::duration time_total{0};

  /*! \brief Accumulated runtime for cut computation. */
  stopwatch<>::duration time_cuts{0};

  /*! \brief Estimated total gain. */
  uint64_t estimated_gain{0};
  
  void report() const
  {
    std::cout << fmt::format( "[i] total time              ({:>5.2f} secs)\n", to_seconds( time_total ) );
  }
};

namespace detail
{

template<class Ntk>
class refactor_impl
{
public:
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

  explicit refactor_impl( Ntk& ntk, refactor_params const& ps, refactor_stats& st )
    : ntk( ntk ), ps( ps ), st( st )
  {
  }

  void run()
  {
    stopwatch t( st.time_total );

    /* start the managers */
    cut_manager<Ntk> mgr( ps.max_pis );
    
    const auto size = ntk.size();
    progress_bar pbar{ntk.size(), "refactor |{0}| node = {1:>4}   cand = {2:>4}   est. gain = {3:>5}", ps.progress};

    ntk.foreach_gate( [&]( auto const& n, auto i ){
        if ( i >= size )
          return false; /* terminate */

        pbar( i, i, candidates, st.estimated_gain );

        if ( ntk.is_dead( n ) )
          return true; /* next */

        /* skip nodes with many fanouts */
        if ( ntk.fanout_size( n ) > ps.skip_fanout_limit_for_roots )
          return true; /* next */

        /* compute reconvergence-driven cut */
        auto const leaves = call_with_stopwatch( st.time_cuts, [&]() {
            return reconv_driven_cut( mgr, ntk, n );
          });

        /* evaluate this cut */
        node_refactor( n, leaves );
        
        /* acceptable replacement found, update the network */

        return true;
      } );
  }

  void node_refactor( node const& r, std::vector<node> const& leaves )
  {
    /* not implemented */
  }
  
private:
  Ntk& ntk;

  refactor_params const& ps;
  refactor_stats& st;

  uint32_t candidates{0};
}; /* refactor_impl */

} /* detail */

/*! \brief Refactor
 *
 * **Required network functions:**
 *
 * \param ntk Input network (will be changed in-place)
 * \param ps Refactor params
 * \param pst Refactor statistics
 */
template<class Ntk>
void refactor( Ntk& ntk, refactor_params const& ps = {}, refactor_stats* pst = nullptr )
{
  refactor_stats st;

  detail::refactor_impl p( ntk, ps, st );
  p.run();
  if ( ps.verbose )
  {
    st.report();
  }

  if ( pst )
  {
    *pst = st;
  }
}

} /* namespace mockturtle */
