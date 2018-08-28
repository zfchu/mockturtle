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
  \file coi_view.hpp
  \brief Implements an isolated view on a cone-of-influence

  \author Heinz Riener
*/

#pragma once

#include "../traits.hpp"
#include "immutable_view.hpp"

#include <sparsepp/spp.h>

namespace mockturtle
{

/*! \brief Implements an isolated view on a cone-of-influence (COI) of a network.
 *
 * Cone-of-influence reductions structurally narrow the view on a
 * logic network to a certain subset of nodes indentified by a set of
 * pivot nodes.  Given the pivot nodes, the COI grows towards the CIs.
 * In constrast to windows, COIs do not have any restriction on the
 * maxmimum number of CIs or nodes to be added.
 *
 * **Required network functions:**
 *  ...
 */
template<typename Ntk>
class coi_view : public immutable_view<Ntk>
{
public:
  using storage = typename Ntk::storage;
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

public:
  explicit coi_view( Ntk const& ntk, std::vector<node> const& pivots, bool sequential_wrap_around = false )
    : immutable_view<Ntk>( ntk )
    , _pivots( pivots )
    , _sequential_wrap_around( sequential_wrap_around )
  {
    const auto c0 = this->get_node( this->get_constant( false ) );
    _constants.push_back( c0 );
    _node_to_index.emplace( c0, _node_to_index.size() );

    const auto c1 = this->get_node( this->get_constant( true ) );
    if ( c1 != c0 )
    {
      _constants.push_back( c1 );
      _node_to_index.emplace( c1, _node_to_index.size() );
      ++_num_constants;

    }

    update();
  }

  inline auto size() const { return _num_constants + _leaves.size() + _inner.size(); }
  inline auto num_cis() const { return _leaves.size(); }
  inline auto num_cos() const { return _pivots.size(); }
  inline auto num_pis() const { return _leaves.size(); }
  inline auto num_pos() const { return _pivots.size(); }
  inline auto num_gates() const { return _inner.size(); }

  inline bool is_pi( node const& pi ) const
  {
    return std::find( _leaves.begin(), _leaves.end(), pi ) != _leaves.end();
  }

  template<typename Fn>
  void foreach_pi( Fn&& fn ) const
  {
    detail::foreach_element( _leaves.begin(), _leaves.end(), fn );
  }

  template<typename Fn>
  void foreach_ci( Fn&& fn ) const
  {
    detail::foreach_element( _leaves.begin(), _leaves.end(), fn );
  }

  template<typename Fn>
  void foreach_po( Fn&& fn ) const
  {
    std::vector<signal> signals;
    for ( const auto& p : _pivots )
    {
      signals.emplace_back( this->make_signal( p ) );
    }
    detail::foreach_element( signals.begin(), signals.end(), fn );
  }

  template<typename Fn>
  void foreach_co( Fn&& fn ) const
  {
    std::vector<signal> signals;
    for ( const auto& p : _pivots )
    {
      signals.emplace_back( this->make_signal( p ) );
    }
    detail::foreach_element( signals.begin(), signals.end(), fn );
  }

  template<typename Fn>
  void foreach_gate( Fn&& fn ) const
  {
    detail::foreach_element( _inner.begin(), _inner.end(), fn );
  }

  template<typename Fn>
  void foreach_node( Fn&& fn ) const
  {
    detail::foreach_element( _constants.begin(), _constants.end(), fn );
    detail::foreach_element( _leaves.begin(), _leaves.end(), fn, _num_constants );
    detail::foreach_element( _inner.begin(), _inner.end(), fn, _num_constants + _leaves.size() );
  }

  inline auto index_to_node( uint32_t index ) const
  {
    if ( index < _num_constants )
    {
      return _constants[index];
    }
    if ( index < _num_constants + _leaves.size() )
    {
      return _leaves[index - _num_constants];
    }
    return _inner[index - _num_constants - _leaves.size()];
  }

  inline auto node_to_index( node const& n ) const { return _node_to_index.at( n ); }

  void update()
  {
    _leaves.clear();
    _inner.clear();

    for ( auto const& p : _pivots )
    {
      collect_recurse( p );
    }

    compute_sets();
  }

  void collect_recurse( node const& n )
  {
    if ( Ntk::is_constant( n ) )
      return;

    if ( Ntk::is_pi( n ) )
    {
      if ( std::find( _nodes.begin(), _nodes.end(), n ) == _nodes.end() )
        _nodes.push_back( n );
      return;
    }

    this->foreach_fanin( n, [&]( auto const& f ){
        if ( std::find( _nodes.begin(), _nodes.end(), this->get_node( f ) ) != _nodes.end() )
          return false;
        _nodes.emplace_back( this->get_node( f ) );
        collect_recurse( this->get_node( f ) );
        return true;
      } );
  }

  void compute_sets()
  {
    std::sort( _nodes.begin(), _nodes.end() );

    for ( auto const& n : _nodes )
    {
      if ( Ntk::is_constant( n ) ) continue;

      if ( Ntk::is_ci( n ) )
      {
        if ( _leaves.empty() || _leaves.back() != n )
        {
          _leaves.push_back( n );
        }
      }
      else
      {
        if ( _inner.empty() || _inner.back() != n )
        {
          _inner.push_back( n );
        }
      }
    }

    for ( auto const& n : _leaves )
    {
      _node_to_index.emplace( n, _node_to_index.size() );
    }
    for ( auto const& n : _inner )
    {
      _node_to_index.emplace( n, _node_to_index.size() );
    }
    for ( auto const& p : _pivots )
    {
      _inner.emplace_back( p );
      _node_to_index.emplace( p, _node_to_index.size() );
    }

    /* sort topologically */
    _topo.clear();
    _colors.clear();
    const auto _size = _num_constants + _inner.size() + _leaves.size();
    _colors.resize( _size, 0 );
    std::for_each( _leaves.begin(), _leaves.end(), [&]( auto& l ) { _colors[_node_to_index[l]] = 2u; } );
    for ( auto i = 0u; i < _num_constants; ++i ) { _colors[i] = 2u; }

    for ( auto const &p : _pivots )
      topo_sort_rec( p );

    assert( _inner.size() == _topo.size() );
    _inner = _topo;
  }

  void topo_sort_rec( node const& n )
  {
    const auto idx = _node_to_index[n];

    /* is permanently marked? */
    if ( _colors[idx] == 2u )
      return;

    /* mark node temporarily */
    _colors[idx] = 1u;

    /* mark children */
    Ntk::foreach_fanin( n, [&]( auto const& f ) {
      topo_sort_rec( Ntk::get_node( f ) );
    } );

    /* mark node n permanently */
    _colors[idx] = 2u;

    _topo.push_back( n );
  }

protected:
  std::vector<node> _constants, _nodes, _leaves, _pivots, _inner, _topo;
  std::vector<uint8_t> _colors;
  spp::sparse_hash_map<node, uint32_t> _node_to_index;
  uint32_t _num_constants{1};
  bool _sequential_wrap_around;
};

} // namespace mockturtle
