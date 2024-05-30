#pragma once

#include "kitty/constructors.hpp"
#include "kitty/dynamic_truth_table.hpp"
#include "kitty/operators.hpp"
#include "mockturtle/mockturtle.hpp"
#include "mockturtle/networks/events.hpp"
#include "mockturtle/networks/storage.hpp"

#include <assert.h>

namespace mockturtle
{

struct gtech_storage_data
{
  mockturtle::truth_table_cache<kitty::dynamic_truth_table> cache;
  uint32_t num_pis{ 0u };
  uint32_t num_pos{ 0u };
  uint32_t trav_id{ 0u };
};

/*! \brief gates_network node
 *
 * `data[0].h1`: Fan-out size
 * `data[0].h2`: Application-specific value
 * `data[1].h1`: Function literal in truth table cache
 * `data[2].h2`: Visited flags
 */
struct gtech_node : mockturtle::mixed_fanin_node<2>
{
  bool operator==( gtech_node const& other ) const
  {
    // function ID and children are same, the 2 gates are seemed to be the same
    return data[1].h1 == other.data[1].h1 && children == other.children;
  }
};

using gtech_storage = mockturtle::storage<gtech_node, gtech_storage_data>;

enum class gtech_gate_type
{
  gconst0 = '0',
  gconst1 = '1',
  gpi = '2',
  gnot = '3',
  gbuf = '4',
  gand = '5',
  gnand = '6',
  gor = '7',
  gnor = '8',
  gxor = '9',
  gxnor = 'a',
  gite = 'b',
  gmaj = 'c',
};

/**
 * @brief primitive gate network
 *  unate:  not, buf
 *  binate: and, nand, or, nor, xor, xnor
 */
class gtech_network
{
public:
  using base_type = gtech_network;
  using node = uint32_t;
  using signal = uint32_t;
  using storage = std::shared_ptr<gtech_storage>;

  static constexpr auto min_fanin_size = 1u; // support not
  static constexpr auto max_fanin_size = 2u;

private:
  std::shared_ptr<gtech_storage> _storage;
  std::shared_ptr<mockturtle::network_events<base_type>> _events;
  std::vector<gtech_gate_type> _gates_type;

public:
  gtech_network();
  ~gtech_network();

public:
  inline signal get_constant( bool value ) const
  {
    return ( value ? 1u : 0u );
  }
  signal create_pi( std::string const& name = std::string() );
  uint32_t create_po( signal const& f, std::string const& name = std::string() );
  signal create_node( std::vector<signal> const& children, kitty::dynamic_truth_table const& function );
  signal clone_node( gtech_network const& other, node const& source, std::vector<signal> const& children );

  signal create_buf( signal const& a );              // literal: 2
  signal create_not( signal const& a );              // literal: 3
  signal create_and( signal a, signal b );           // literal: 4
  signal create_nand( signal a, signal b );          // literal: 5
  signal create_or( signal a, signal b );            // literal: 6
  signal create_nor( signal a, signal b );           // literal: 7
  signal create_xor( signal a, signal b );           // literal: 12
  signal create_xnor( signal a, signal b );          // literal: 13
  signal create_maj( signal a, signal b, signal c ); // literal: 14
  signal create_ite( signal i, signal t, signal e ); // literal: 16

  signal create_nary_and( std::vector<signal> const& fs );
  signal create_nary_or( std::vector<signal> const& fs );
  signal create_nary_xor( std::vector<signal> const& fs );

  uint32_t fanin_size( node const& n ) const;
  uint32_t fanout_size( node const& n ) const;
  uint32_t incr_fanout_size( node const& n ) const;
  uint32_t decr_fanout_size( node const& n ) const;

  kitty::dynamic_truth_table node_function( const node& n ) const;
  uint32_t node_literal( const node& n ) const;

  node get_node( signal const& f ) const;
  gtech_gate_type get_nodetype( node const& n ) const;
  signal make_signal( node const& n ) const;
  uint32_t node_to_index( node const& n ) const;
  node index_to_node( uint32_t index ) const;
  node pi_at( uint32_t index ) const;
  signal po_at( uint32_t index ) const;
  uint32_t pi_index( node const& n ) const;
  uint32_t po_index( signal const& s ) const;

  template<typename Fn>
  void foreach_pi( Fn&& fn ) const;
  template<typename Fn>
  void foreach_po( Fn&& fn ) const;
  template<typename Fn>
  void foreach_node( Fn&& fn ) const;
  template<typename Fn>
  void foreach_gate( Fn&& fn ) const;
  template<typename Fn>
  void foreach_fanin( node const& n, Fn&& fn ) const;

  template<typename Iterator>
  mockturtle::iterates_over_t<Iterator, bool>
  compute( node const& n, Iterator begin, Iterator end ) const;

  template<typename Iterator>
  mockturtle::iterates_over_truth_table_t<Iterator>
  compute( node const& n, Iterator begin, Iterator end ) const;

  inline void clear_values() const
  {
    std::for_each( _storage->nodes.begin(), _storage->nodes.end(), []( auto& n ) { n.data[0].h2 = 0; } );
  }
  inline auto value( node const& n ) const
  {
    return _storage->nodes[n].data[0].h2;
  }
  inline void set_value( node const& n, uint32_t v ) const
  {
    _storage->nodes[n].data[0].h2 = v;
  }
  inline auto incr_value( node const& n ) const
  {
    return _storage->nodes[n].data[0].h2++;
  }
  inline auto decr_value( node const& n ) const
  {
    return static_cast<uint32_t>( --_storage->nodes[n].data[0].h2 );
  }
  inline void clear_visited() const
  {
    std::for_each( _storage->nodes.begin(), _storage->nodes.end(), []( auto& n ) { n.data[1].h2 = 0; } );
  }
  inline auto visited( node const& n ) const
  {
    return _storage->nodes[n].data[1].h2;
  }
  inline void set_visited( node const& n, uint32_t v ) const
  {
    _storage->nodes[n].data[1].h2 = v;
  }
  inline uint32_t trav_id() const
  {
    return _storage->data.trav_id;
  }
  inline void incr_trav_id() const
  {
    ++_storage->data.trav_id;
  }

  inline bool is_constant( node const& n ) const
  {
    return n <= 1;
  }

  inline bool is_constant_zero( node const& n ) const
  {
    return n == 0;
  }

  inline bool is_constant_one( node const& n ) const
  {
    return n == 1;
  }

  inline bool constant_value( node const& n ) const
  {
    return n <= 1;
  }

  bool is_complemented( signal const& f ) const
  {
    (void)f;
    return false;
  }

  inline bool is_pi( node const& n ) const
  {
    const auto end = _storage->inputs.begin() + _storage->data.num_pis;
    return std::find( _storage->inputs.begin(), end, n ) != end;
  }
  inline bool is_function( node const& n ) const
  {
    return n > 1 && !is_pi( n );
  }

  inline bool is_not( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 3;
  }

  inline bool is_and( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 4;
  }

  inline bool is_nand( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 5;
  }
  inline bool is_or( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 6;
  }
  inline bool is_nor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 7;
  }

  inline bool is_xor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 12;
  }

  inline bool is_xnor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 13;
  }

  inline bool is_lt( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 8;
  }

  inline bool is_le( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 11;
  }

  inline bool is_maj( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 14;
  }

  inline bool is_ite( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 16;
  }

  inline bool is_xor3( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 18;
  }

  inline uint32_t size() const
  {
    return static_cast<uint32_t>( _storage->nodes.size() );
  }
  inline uint32_t num_pis() const
  {
    return _storage->data.num_pis;
  }
  inline uint32_t num_pos() const
  {
    return _storage->data.num_pos;
  }
  inline uint32_t num_gates() const
  {
    return static_cast<uint32_t>( _storage->nodes.size() - _storage->inputs.size() - 2 );
  }
  inline auto& events() const
  {
    return *_events;
  }

  /**
   * @brief verify the gate type for each node
   */
  bool verify();

private:
  signal _create_node( std::vector<signal> const& children, uint32_t literal );
  void init();
};

gtech_network::gtech_network()
    : _storage( std::make_shared<gtech_storage>() ),
      _events( std::make_shared<decltype( _events )::element_type>() )
{
  init();
}

gtech_network::~gtech_network()
{
}

gtech_network::signal gtech_network::create_pi( std::string const& name )
{
  (void)name;
  const auto index = _storage->nodes.size();
  _storage->nodes.emplace_back();
  _storage->inputs.emplace_back( index );
  _storage->nodes[index].data[1].h1 = 2; // 2 means a buf

  ++_storage->data.num_pis;

  _gates_type.emplace_back( gtech_gate_type::gpi );
  return index;
}

uint32_t gtech_network::create_po( gtech_network::signal const& f, std::string const& name )
{
  (void)name;

  /* increase ref-count to children */
  _storage->nodes[f].data[0].h1++;

  auto const po_index = static_cast<uint32_t>( _storage->outputs.size() );
  _storage->outputs.emplace_back( f );
  ++_storage->data.num_pos;
  return po_index;
}

gtech_network::signal gtech_network::create_node( std::vector<gtech_network::signal> const& children, kitty::dynamic_truth_table const& function )
{
  if ( children.size() == 0u )
  {
    assert( function.num_vars() == 0u );
    return get_constant( !kitty::is_const0( function ) );
  }
  return _create_node( children, _storage->data.cache.insert( function ) );
}

gtech_network::signal gtech_network::clone_node( gtech_network const& other, gtech_network::node const& source, std::vector<gtech_network::signal> const& children )
{
  assert( !children.empty() );
  const auto tt = other._storage->data.cache[other._storage->nodes[source].data[1].h1];
  return create_node( children, tt );
}

gtech_network::signal gtech_network::create_buf( gtech_network::signal const& a )
{
  return a;
}

gtech_network::signal gtech_network::create_not( gtech_network::signal const& a )
{
  _gates_type.emplace_back( gtech_gate_type::gnot );
  auto res = _create_node( { a }, 3 ); // 0x1
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gnot );
  return res;
}

gtech_network::signal gtech_network::create_and( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gand );
  auto res = _create_node( { a, b }, 4 ); // 0x1
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gand );
  return res;
}
gtech_network::signal gtech_network::create_nand( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gnand );
  auto res = _create_node( { a, b }, 5 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gnand );
  return res;
}
gtech_network::signal gtech_network::create_or( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gor );
  auto res = _create_node( { a, b }, 6 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gor );
  return res;
  // return _create_node({a, b}, 6);
}
gtech_network::signal gtech_network::create_nor( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gnor );
  auto res = _create_node( { a, b }, 7 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gnor );
  return res;
}
gtech_network::signal gtech_network::create_xor( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gxor );
  auto res = _create_node( { a, b }, 12 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gxor );
  return res;
}
gtech_network::signal gtech_network::create_xnor( gtech_network::signal a, gtech_network::signal b )
{
  _gates_type.emplace_back( gtech_gate_type::gxnor );
  auto res = _create_node( { a, b }, 13 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gxnor );
  return res;
}

gtech_network::signal gtech_network::create_maj( gtech_network::signal a, gtech_network::signal b, gtech_network::signal c )
{
  _gates_type.emplace_back( gtech_gate_type::gmaj );
  auto res = _create_node( { a, b, c }, 14 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gmaj );
  return res;
}

gtech_network::signal gtech_network::create_ite( gtech_network::signal i, gtech_network::signal t, gtech_network::signal e )
{
  _gates_type.emplace_back( gtech_gate_type::gite );
  auto res = _create_node( { i, t, e }, 16 );
  assert( _storage->nodes.size() == _gates_type.size() );
  assert( get_nodetype( get_node( res ) ) == gtech_gate_type::gite );
  return res;
}

gtech_network::signal gtech_network::create_nary_and( std::vector<gtech_network::signal> const& fs )
{
  return mockturtle::tree_reduce( fs.begin(), fs.end(), get_constant( true ), [this]( auto const& a, auto const& b ) { return create_and( a, b ); } );
}

gtech_network::signal gtech_network::create_nary_or( std::vector<gtech_network::signal> const& fs )
{
  return mockturtle::tree_reduce( fs.begin(), fs.end(), get_constant( false ), [this]( auto const& a, auto const& b ) { return create_or( a, b ); } );
}

gtech_network::signal gtech_network::create_nary_xor( std::vector<gtech_network::signal> const& fs )
{
  return mockturtle::tree_reduce( fs.begin(), fs.end(), get_constant( false ), [this]( auto const& a, auto const& b ) { return create_xor( a, b ); } );
}

uint32_t gtech_network::fanin_size( gtech_network::node const& n ) const
{
  return static_cast<uint32_t>( _storage->nodes[n].children.size() );
}

uint32_t gtech_network::fanout_size( gtech_network::node const& n ) const
{
  return _storage->nodes[n].data[0].h1;
}

uint32_t gtech_network::incr_fanout_size( gtech_network::node const& n ) const
{
  return _storage->nodes[n].data[0].h1++;
}

uint32_t gtech_network::decr_fanout_size( gtech_network::node const& n ) const
{
  return --_storage->nodes[n].data[0].h1;
}

kitty::dynamic_truth_table gtech_network::node_function( const gtech_network::node& n ) const
{
  return _storage->data.cache[_storage->nodes[n].data[1].h1];
}

uint32_t gtech_network::node_literal( const gtech_network::node& n ) const
{
  return _storage->nodes[n].data[1].h1;
}

gtech_network::node gtech_network::get_node( gtech_network::signal const& f ) const
{
  return f;
}

gtech_gate_type gtech_network::get_nodetype( gtech_network::node const& n ) const
{
  return _gates_type[n];
}

gtech_network::signal gtech_network::make_signal( gtech_network::node const& n ) const
{
  return n;
}

uint32_t gtech_network::node_to_index( gtech_network::node const& n ) const
{
  return n;
}

gtech_network::node gtech_network::index_to_node( uint32_t index ) const
{
  return index;
}

gtech_network::node gtech_network::pi_at( uint32_t index ) const
{
  assert( index < _storage->data.num_pis );
  return *( _storage->inputs.begin() + index );
}

gtech_network::signal gtech_network::po_at( uint32_t index ) const
{
  assert( index < _storage->data.num_pos );
  return ( _storage->outputs.begin() + index )->index;
}

uint32_t gtech_network::pi_index( gtech_network::node const& n ) const
{
  assert( _storage->nodes[n].children[0].data == _storage->nodes[n].children[1].data );
  return static_cast<uint32_t>( _storage->nodes[n].children[0].data );
}

uint32_t gtech_network::po_index( gtech_network::signal const& s ) const
{
  uint32_t i = -1;
  foreach_po( [&]( const auto& x, auto index ) {
      if ( x == s )
      {
        i = index;
        return false;
      }
      return true; } );
  return i;
}

template<typename Fn>
void gtech_network::foreach_pi( Fn&& fn ) const
{
  mockturtle::detail::foreach_element( _storage->inputs.begin(), _storage->inputs.end(), fn );
}

template<typename Fn>
void gtech_network::foreach_po( Fn&& fn ) const
{
  using IteratorType = decltype( _storage->outputs.begin() );
  mockturtle::detail::foreach_element_transform<IteratorType, uint32_t>(
      _storage->outputs.begin(), _storage->outputs.begin() + _storage->data.num_pos, []( auto o ) { return o.index; },
      fn );
}

template<typename Fn>
void gtech_network::foreach_node( Fn&& fn ) const
{
  auto r = mockturtle::range<uint64_t>( _storage->nodes.size() );
  mockturtle::detail::foreach_element( r.begin(), r.end(), fn );
}

template<typename Fn>
void gtech_network::foreach_gate( Fn&& fn ) const
{
  auto r = mockturtle::range<uint64_t>( 2u, _storage->nodes.size() ); /* start from 2 to avoid constant */
  mockturtle::detail::foreach_element_if(
      r.begin(), r.end(),
      [this]( auto n ) { return !is_pi( n ); },
      fn );
}

template<typename Fn>
void gtech_network::foreach_fanin( gtech_network::node const& n, Fn&& fn ) const
{
  if ( n <= 1 || is_pi( n ) ) // skip constant 0, constant 1 and PI s
    return;

  using IteratorType = decltype( _storage->outputs.begin() );
  mockturtle::detail::foreach_element_transform<IteratorType, uint32_t>(
      _storage->nodes[n].children.begin(), _storage->nodes[n].children.end(), []( auto f ) { return f.index; },
      fn );
}

template<typename Iterator>
mockturtle::iterates_over_t<Iterator, bool>
gtech_network::compute( gtech_network::node const& n, Iterator begin, Iterator end ) const
{
  assert( n > 1 && !is_pi( n ) );

  uint32_t index{ 0 };
  while ( begin != end )
  {
    index <<= 1;
    index ^= *begin++ ? 1 : 0;
  }
  return kitty::get_bit( _storage->data.cache[_storage->nodes[n].data[1].h1], index );
}

template<typename Iterator>
mockturtle::iterates_over_truth_table_t<Iterator>
gtech_network::compute( gtech_network::node const& n, Iterator begin, Iterator end ) const
{
  const auto nfanin = _storage->nodes[n].children.size();

  std::vector<typename Iterator::value_type> tts( begin, end );

  assert( nfanin != 0 );
  assert( tts.size() == nfanin );

  /* resulting truth table has the same size as any of the children */
  auto result = tts.front().construct();
  const auto gate_tt = _storage->data.cache[_storage->nodes[n].data[1].h1];

  for ( uint32_t i = 0u; i < static_cast<uint32_t>( result.num_bits() ); ++i )
  {
    uint32_t pattern = 0u;
    for ( auto j = 0u; j < nfanin; ++j )
    {
      pattern |= kitty::get_bit( tts[j], i ) << j;
    }
    if ( kitty::get_bit( gate_tt, pattern ) )
    {
      kitty::set_bit( result, i );
    }
  }
  return result;
}

gtech_network::signal gtech_network::_create_node( std::vector<gtech_network::signal> const& children, uint32_t literal )
{
  storage::element_type::node_type tmp_node;
  std::copy( children.begin(), children.end(), std::back_inserter( tmp_node.children ) );
  tmp_node.data[1].h1 = literal;

  // const auto it = _storage->hash.find(tmp_node);
  // if (it != _storage->hash.end())
  // {
  //     return it->second;
  // }

  const auto index = _storage->nodes.size();
  _storage->nodes.emplace_back( tmp_node );
  _storage->hash[tmp_node] = index;

  // increase ref-count to children
  for ( auto c : children )
  {
    _storage->nodes[c].data[0].h1++;
  }

  for ( auto const& fn : _events->on_add )
  {
    ( *fn )( index );
  }

  return index;
}

/**
 * @brief inite the basic gate's function into truth table cache
 *
 *      gate truth table cache
 *                  binary-tt   func-id
 * constant 0 ->         0           0
 * constant 1 ->         1           1
 *
 *     buf    ->        10           2
 *     not    ->        01           3
 *
 *     and    ->      1000           4
 *     nand   ->      0111           5
 *
 *     or     ->      1110           6
 *     nor    ->      0001           7
 *
 *     lt     ->      0100           8
 *
 *
 *     le     ->      1101          11
 *
 *     xor    ->      0110          12
 *     xnor   ->      1001          13
 *
 *     maj    -> 1110,1000          14
 *     ite    -> 1101,1000          16
 *     xor3   -> 1001,0110          18
 *
 */
void gtech_network::init()
{
  /* reserve the second node for constant 1 */
  _storage->nodes.emplace_back();

  /* reserve some truth tables for nodes */

  kitty::dynamic_truth_table tt_zero( 0 ); // func-id 0
  _storage->data.cache.insert( tt_zero );

  static uint64_t _not = 0x1; // func-id 3
  kitty::dynamic_truth_table tt_not( 1 );
  kitty::create_from_words( tt_not, &_not, &_not + 1 );
  _storage->data.cache.insert( tt_not );

  static uint64_t _and = 0x8; // func-id 4
  kitty::dynamic_truth_table tt_and( 2 );
  kitty::create_from_words( tt_and, &_and, &_and + 1 );
  _storage->data.cache.insert( tt_and );

  static uint64_t _or = 0xe; // func-id 6
  kitty::dynamic_truth_table tt_or( 2 );
  kitty::create_from_words( tt_or, &_or, &_or + 1 );
  _storage->data.cache.insert( tt_or );

  static uint64_t _lt = 0x4; // func-id 8
  kitty::dynamic_truth_table tt_lt( 2 );
  kitty::create_from_words( tt_lt, &_lt, &_lt + 1 );
  _storage->data.cache.insert( tt_lt );

  static uint64_t _le = 0xd; // func-id 11
  kitty::dynamic_truth_table tt_le( 2 );
  kitty::create_from_words( tt_le, &_le, &_le + 1 );
  _storage->data.cache.insert( tt_le );

  static uint64_t _xor = 0x6; // func-id 12
  kitty::dynamic_truth_table tt_xor( 2 );
  kitty::create_from_words( tt_xor, &_xor, &_xor + 1 );
  _storage->data.cache.insert( tt_xor );

  static uint64_t _maj = 0xe8; // func-id 14
  kitty::dynamic_truth_table tt_maj( 3 );
  kitty::create_from_words( tt_maj, &_maj, &_maj + 1 );
  _storage->data.cache.insert( tt_maj );

  static uint64_t _ite = 0xd8; // func-id 16
  kitty::dynamic_truth_table tt_ite( 3 );
  kitty::create_from_words( tt_ite, &_ite, &_ite + 1 );
  _storage->data.cache.insert( tt_ite );

  static uint64_t _xor3 = 0x96; // func-id 18
  kitty::dynamic_truth_table tt_xor3( 3 );
  kitty::create_from_words( tt_xor3, &_xor3, &_xor3 + 1 );
  _storage->data.cache.insert( tt_xor3 );

  _gates_type.reserve( 20000u );
  _gates_type.emplace_back( gtech_gate_type::gconst0 );
  _gates_type.emplace_back( gtech_gate_type::gconst1 );
}

bool gtech_network::verify()
{
  this->foreach_node( [&]( auto const& node ) {
        if(is_constant(node))
            assert(get_nodetype(node) == gtech_gate_type::gconst0 || get_nodetype(node) == gtech_gate_type::gconst1);
        if(is_pi(node))
            assert(get_nodetype(node) == gtech_gate_type::gpi);
        if(is_not(node))
            assert(get_nodetype(node) == gtech_gate_type::gnot);
        if(is_and(node))
            assert(get_nodetype(node) == gtech_gate_type::gand);
        if(is_nand(node))
            assert(get_nodetype(node) == gtech_gate_type::gnand);
        if(is_or(node))
            assert(get_nodetype(node) == gtech_gate_type::gor);
        if(is_nor(node))
            assert(get_nodetype(node) == gtech_gate_type::gnor);
        if(is_xor(node))
            assert(get_nodetype(node) == gtech_gate_type::gxor);
        if(is_xnor(node))
            assert(get_nodetype(node) == gtech_gate_type::gxnor); } );
  return true;
}
} // end namespace mockturtle