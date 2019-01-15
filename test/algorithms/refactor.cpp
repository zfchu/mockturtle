#include <catch.hpp>

#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <mockturtle/algorithms/refactor.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/io/write_verilog.hpp>

#include <kitty/static_truth_table.hpp>

using namespace mockturtle;

TEST_CASE( "Refactor of AIG", "[refactor]" )
{
  aig_network aig;

  const auto a = aig.create_pi();
  const auto b = aig.create_pi();

  const auto f = aig.create_and( a, aig.create_and( b, a ) );
  aig.create_po( f );

  CHECK( aig.size() == 5 );
  CHECK( aig.num_pis() == 2 );
  CHECK( aig.num_pos() == 1 );
  CHECK( aig.num_gates() == 2 );

  const auto tt = simulate<kitty::static_truth_table<2>>( aig )[0];
  CHECK( tt._bits == 0x8 );

  using view_t = depth_view<fanout_view<aig_network>>;
  fanout_view<aig_network> fanout_view{aig};
  view_t refactor_view{fanout_view};
  
  refactor( refactor_view );

  aig = cleanup_dangling( aig );

  /* check equivalence */
  const auto tt_opt = simulate<kitty::static_truth_table<2>>( aig )[0];
  CHECK( tt_opt._bits == tt._bits );

  // CHECK( aig.size() == 4 );
  // CHECK( aig.num_pis() == 2 );
  // CHECK( aig.num_pos() == 1 );
  // CHECK( aig.num_gates() == 1 );
}
