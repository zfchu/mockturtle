/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2021  EPFL
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
  \file mig_enumerative.hpp
  \brief MIG enumerative resynthesis

  \author Siang-Yun (Sonia) Lee
  
  Based on previous implementation of MIG resubstitution by 
  Eleonora Testa, Heinz Riener, and Mathias Soeken
*/

#pragma once

#include "../experimental/boolean_optimization.hpp"
#include "../../utils/index_list.hpp"
#include <kitty/kitty.hpp>
#include <vector>
#include <optional>

namespace mockturtle::experimental
{

struct mig_enumerative_resyn_stats
{

  void report() const
  {
  }
}; /* mig_enumerative_resyn_stats */

template<typename TT>
struct mig_enumerative_resyn
{
public:
  using stats = mig_enumerative_resyn_stats;
  using params = null_params;
  using index_list_t = mig_index_list;
  using truth_table_t = TT;

public:
  explicit mig_enumerative_resyn( stats& st, params const& ps = {} ) noexcept
    : st( st )
  { (void)ps; }

  template<class iterator_type, class truth_table_storage_type>
  std::optional<index_list_t> operator()( TT const& target, TT const& care, iterator_type begin, iterator_type end, truth_table_storage_type const& tts, uint32_t max_size = std::numeric_limits<uint32_t>::max() )
  {
    (void)care;
    assert( is_const0( ~care ) && "enumerative resynthesis does not support don't cares" );

    index_list_t il( std::distance( begin, end ) );
    const TT ntarget = ~target;
    uint32_t i, j;
    iterator_type it = begin, jt = begin;

    /* C-resub */
    if ( kitty::is_const0( target ) )
    {
      il.add_output( 0 );
      return il;
    }
    if ( kitty::is_const0( ntarget ) ) // unreachable if normalized
    {
      il.add_output( 1 );
      return il;
    }

    /* 0-resub */
    for ( it = begin, i = 0u; it != end; ++it, ++i )
    {
      if ( target == tts[*it] )
      {
        il.add_output( make_lit( i ) );
        return il;
      }
      if ( ntarget == tts[*it] ) // unreachable if normalized
      {
        il.add_output( make_lit( i, true ) );
        return il;
      }
    }

    /* R-resub: doesn't work with this problem definition (need fanins of root) */

    if ( max_size == 0 )
    {
      return std::nullopt;
    }

    /* collect candidate pairs using MAJ filtering rule */
    std::vector<std::pair<uint32_t, uint32_t>> maj1pairs;
    for ( it = begin, i = 0u; it != end; ++it, ++i )
    {
      for ( jt = it + 1, j = i + 1; jt != end; ++jt, ++j )
      {
        if ( kitty::ternary_majority( tts[*it], tts[*jt], target ) == target )
        {
          maj1pairs.emplace_back( make_lit( i ), make_lit( j ) );
        }
        else if ( kitty::ternary_majority( ~tts[*it], tts[*jt], target ) == target )
        {
          maj1pairs.emplace_back( make_lit( i, true ), make_lit( j ) );
        }
        else if ( kitty::ternary_majority( tts[*it], ~tts[*jt], target ) == target )
        {
          maj1pairs.emplace_back( make_lit( i ), make_lit( j, true ) );
        }
        else if ( kitty::ternary_majority( ~tts[*it], ~tts[*jt], target ) == target ) // unreachable if normalized
        {
          maj1pairs.emplace_back( make_lit( i, true ), make_lit( j, true ) );
        }
      }
      if ( kitty::implies( tts[*it], target ) )
      {
        maj1pairs.emplace_back( make_lit( i ), 1 );
      }
      else if ( kitty::implies( ~tts[*it], target ) )
      {
        maj1pairs.emplace_back( make_lit( i, true ), 1 );
      }
      else if ( kitty::implies( target, tts[*it] ) )
      {
        maj1pairs.emplace_back( make_lit( i ), 0 );
      }
      else if ( kitty::implies( target, ~tts[*it] ) )
      {
        maj1pairs.emplace_back( make_lit( i, true ), 0 );
      }
    }

    /* 1-resub */
    for ( i = 0u; i < maj1pairs.size(); ++i )
    {
      TT const& x = get_tt_from_lit( maj1pairs[i].first, tts, begin );
      if ( maj1pairs[i].second < 2 )
      {
        for ( j = i + 1; j < maj1pairs.size(); ++j )
        {
          if ( maj1pairs[i].second == 0 && target == ( x & get_tt_from_lit( maj1pairs[j].first, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].first ) );
            return il;
          }
          else if ( maj1pairs[i].second == 1 && target == ( x | get_tt_from_lit( maj1pairs[j].first, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].first ) );
            return il;
          }

          if ( maj1pairs[j].second < 2 )
          {
            continue;
          }
          if ( maj1pairs[i].second == 0 && target == ( x & get_tt_from_lit( maj1pairs[j].second, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].second ) );
            return il;
          }
          else if ( maj1pairs[i].second == 1 && target == ( x | get_tt_from_lit( maj1pairs[j].second, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].second ) );
            return il;
          }
        }
      }
      else
      {
        TT const& y = get_tt_from_lit( maj1pairs[i].second, tts, begin );
        for ( j = i + 1; j < maj1pairs.size(); ++j )
        {
          if ( target == kitty::ternary_majority( x, y, get_tt_from_lit( maj1pairs[j].first, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].first ) );
            return il;
          }
          if ( maj1pairs[j].second < 2 )
          {
            if ( maj1pairs[j].second == 0 && target == ( x & y ) )
            {
              il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].second ) );
              return il;
            }
            else if ( maj1pairs[j].second == 1 && target == ( x | y ) )
            {
              il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].second ) );
              return il;
            }
          }
          else if ( target == kitty::ternary_majority( x, y, get_tt_from_lit( maj1pairs[j].second, tts, begin ) ) )
          {
            il.add_output( il.add_maj( maj1pairs[i].first, maj1pairs[i].second, maj1pairs[j].second ) );
            return il;
          }
        }
      }
    }

    if ( max_size == 1 )
    {
      return std::nullopt;
    }

    return std::nullopt;
  }

private:
  uint32_t make_lit( uint32_t const& var, bool const& inv = false )
  {
    return (var + 1) * 2 + (uint32_t)inv;
  }

  template<class truth_table_storage_type, class iterator_type>
  TT get_tt_from_lit( uint32_t const& lit, truth_table_storage_type const& tts, iterator_type const& begin )
  {
    return (lit % 2) ? ~tts[*(begin + (lit / 2) - 1)] : tts[*(begin + (lit / 2) - 1)];
  }

private:
  stats& st;
}; /* mig_enumerative_resyn */

} /* namespace mockturtle */
