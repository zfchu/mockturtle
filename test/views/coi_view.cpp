#include <catch.hpp>

#include <mockturtle/networks/aig.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/views/coi_view.hpp>

using namespace mockturtle;

TEST_CASE( "create a COI view", "[coi_view]" )
{
  aig_network aig;
  const auto a = aig.create_pi();
  const auto b = aig.create_pi();
  const auto c = aig.create_pi();
  const auto d = aig.create_pi();
  const auto e = aig.create_pi();

  const auto f1 = aig.create_and( a, b );
  const auto f2 = aig.create_and( c, d );
  const auto f3 = aig.create_and( f1, f2 );
  const auto f4 = aig.create_and( e, f2 );
  const auto f5 = aig.create_and( f1, f3 );
  const auto f6 = aig.create_and( f2, f3 );
  const auto f7 = aig.create_and( f5, f6 );
  const auto f8 = aig.create_and( f4, f7 );
  aig.create_po( f8 );

  CHECK( aig.size() == 14 );
  CHECK( aig.num_pis() == 5 );
  CHECK( aig.num_pos() == 1 );
  CHECK( aig.num_gates() == 8 );

  CHECK( aig.get_node( f1 ) == 6 );
  CHECK( aig.get_node( f2 ) == 7 );
  CHECK( aig.get_node( f3 ) == 8 );
  CHECK( aig.get_node( f4 ) == 9 );
  CHECK( aig.get_node( f5 ) == 10 );
  CHECK( aig.get_node( f6 ) == 11 );
  CHECK( aig.get_node( f7 ) == 12 );
  CHECK( aig.get_node( f8 ) == 13 );

  coi_view coi1{aig, { aig.get_node( f3 ), aig.get_node( f5 ) }};
  CHECK( coi1.size() == 9 );
  CHECK( coi1.num_cis() == 4 );
  CHECK( coi1.num_cos() == 2 );
  CHECK( coi1.num_pis() == 4 );
  CHECK( coi1.num_pos() == 2 );
  CHECK( coi1.num_gates() == 4 );

  coi1.foreach_ci( [&]( auto const& n, auto i ) {
    CHECK( coi1.node_to_index( n ) == i + 1 );
    switch ( i )
    {
    case 0:
      CHECK( n == aig.get_node( a ) );
      break;
    case 1:
      CHECK( n == aig.get_node( b ) );
      break;
    case 2:
      CHECK( n == aig.get_node( c ) );
      break;
    case 3:
      CHECK( n == aig.get_node( d ) );
      break;
    case 4:
      CHECK( n == aig.get_node( f1 ) );
      break;
    case 5:
      CHECK( n == aig.get_node( f2 ) );
      break;
    case 6:
      CHECK( n == aig.get_node( f3 ) );
      break;
    case 7:
      CHECK( n == aig.get_node( f5 ) );
      break;
    default:
      CHECK( false );
      break;
    }
  });

  coi1.foreach_gate( [&]( auto const& n, auto i ) {
    CHECK( coi1.node_to_index( n ) == i + 1 + coi1.num_cis() );
    switch ( i )
    {
    case 0:
      CHECK( n == aig.get_node( f1 ) );
      break;
    case 1:
      CHECK( n == aig.get_node( f2 ) );
      break;
    case 2:
      CHECK( n == aig.get_node( f3 ) );
      break;
    case 3:
      CHECK( n == aig.get_node( f5 ) );
      break;
    default:
      CHECK( false );
      break;
    }
  } );

  coi1.foreach_node( [&]( auto const& n, auto i ) {
    CHECK( coi1.node_to_index( n ) == i );
    switch ( i )
    {
    case 0:
      CHECK( n == aig.get_node( aig.get_constant( false ) ) );
      break;
    case 1:
      CHECK( n == aig.get_node( a ) );
      break;
    case 2:
      CHECK( n == aig.get_node( b ) );
      break;
    case 3:
      CHECK( n == aig.get_node( c ) );
      break;
    case 4:
      CHECK( n == aig.get_node( d ) );
      break;
    case 5:
      CHECK( n == aig.get_node( f1 ) );
      break;
    case 6:
      CHECK( n == aig.get_node( f2 ) );
      break;
    case 7:
      CHECK( n == aig.get_node( f3 ) );
      break;
    case 8:
      CHECK( n == aig.get_node( f5 ) );
      break;
    default:
      CHECK( false );
      break;
    }
  } );

  coi1.foreach_co( [&]( auto const& f, auto i ) {
    switch ( i )
    {
    case 0:
      CHECK( coi1.get_node( f ) == aig.get_node( f3 ) );
      break;
    case 1:
      CHECK( coi1.get_node( f ) == aig.get_node( f5 ) );
      break;
    default:
      CHECK( false );
      break;
    }
  } );

  coi1.foreach_po( [&]( auto const& f, auto i ) {
    switch ( i )
    {
    case 0:
      CHECK( coi1.get_node( f ) == aig.get_node( f3 ) );
      break;
    case 1:
      CHECK( coi1.get_node( f ) == aig.get_node( f5 ) );
      break;
    default:
      CHECK( false );
      break;
    }
  } );
}
