#pragma once

#include "../traits.hpp"
#include "../utils/algorithm.hpp"
#include "../utils/truth_table_cache.hpp"
#include "detail/foreach.hpp"
#include "events.hpp"
#include "storage.hpp"

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operators.hpp>
#include <kitty/partial_truth_table.hpp>

#include <list>
#include <memory>
#include <optional>
#include <stack>
#include <string>

namespace mockturtle
{

/**
 * @brief strcut gtg_storage_data
 */
struct gtg_storage_data
{
  mockturtle::truth_table_cache<kitty::dynamic_truth_table> cache;
  uint32_t num_pis = 0u;
  uint32_t num_pos = 0u;
  uint32_t trav_id = 0u;
};

/*! \brief GTG storage container

  GTGs have nodes with fan-in 2.  We split of one bit of the index pointer to
  store a complemented attribute.  Every node has 64-bit of additional data
  used for the following purposes:

  `data[0].h1`: Fan-out size (we use MSB to indicate whether a node is dead)
  `data[0].h2`: Application-specific value
  `data[1].h1`: Function literal in truth table cache
  `data[1].h2`: Visited flag
*/
struct gtg_storage_node : max_fanin_node<3, 2, 1>
{
  bool operator==( gtg_storage_node const& other ) const
  {
    return data[1].h1 == other.data[1].h1 && children == other.children; // same type node and same children
  }
};

using gtg_storage = storage_no_hash<gtg_storage_node,
                                    gtg_storage_data>;

/**
 * @brief Gtech Logic network
 * @note this network is composed by the following types of logic gates:
 *      unate  :  buffer, inverter,
 *      binate :  and2, nand2, or2, nor2, xor2, xnor2,
 *      trinate:  mux, nmux, aoi3, oai3, axi3, xai3, oxi3, xoi3
 */
class gtg_network
{
public:
#pragma region Types and constructors
  static constexpr auto min_fanin_size = 2u;
  static constexpr auto max_fanin_size = 3u;

  using base_type = gtg_network;
  using storage = std::shared_ptr<gtg_storage>;
  using node = uint64_t;

  struct signal
  {
    signal() = default;

    signal( uint64_t index, uint64_t complement )
        : complement( complement ), index( index )
    {}

    explicit signal( uint64_t data )
        : data( data )
    {}

    signal( gtg_storage::node_type::pointer_type const& p )
        : complement( p.weight ), index( p.index )
    {}

    union
    {
      struct
      {
        uint64_t complement : 1;
        uint64_t index : 63;
      };
      uint64_t data;
    };

    signal operator!() const
    {
      return signal( data ^ 1 );
    }

    signal operator+() const
    {
      return { index, 0 };
    }

    signal operator-() const
    {
      return { index, 1 };
    }

    signal operator^( bool complement ) const
    {
      return signal( data ^ ( complement ? 1 : 0 ) );
    }

    bool operator==( signal const& other ) const
    {
      return data == other.data;
    }

    bool operator!=( signal const& other ) const
    {
      return data != other.data;
    }

    bool operator<( signal const& other ) const
    {
      return data < other.data;
    }

    operator gtg_storage::node_type::pointer_type() const
    {
      return { index, complement };
    }

#if __cplusplus > 201703L
    bool operator==( gtg_storage::node_type::pointer_type const& other ) const
    {
      return data == other.data;
    }
#endif
  };

  gtg_network()
      : _storage( std::make_shared<gtg_storage>() ),
        _events( std::make_shared<decltype( _events )::element_type>() )
  {
    _init();
  }

  gtg_network( std::shared_ptr<gtg_storage> storage )
      : _storage( storage ),
        _events( std::make_shared<decltype( _events )::element_type>() )
  {
    _init();
  }

  gtg_network clone() const
  {
    return { std::make_shared<gtg_storage>( *_storage ) };
  }
#pragma endregion

private:
  /**
   * @brief initialize the basic gate's function into truth table cache
   *      gate truth table cache
   *                  binary-tt   func-id
   * constant 0 ->         0           0
   * constant 1 ->         1           1
   *     buf    ->        10           2
   *     not    ->        01           3
   *---------------------------------------
   *     and    ->      1000           4
   *     nand   ->      0111           5
   *     or     ->      1110           6
   *     nor    ->      0001           7
   *     xor    ->      0110          12
   *     xnor   ->      1001          13
   *     ite    -> 1101,1000          16
   *     nand3  -> 0111,1111          21
   *     nor3   -> 0000,0001          23
   *     aoi21  -> 0001,0101          25
   *     oai21  -> 0101,0111          27
   *     axi21  -> 1001,0101          29
   *     xai21  -> 1101,0111          31
   *     oxi21  -> 1010,1001          33
   *     xoi21  -> 0100,0001          35
   */
  void _init()
  {
    // reserve truth tables for nodes
    kitty::dynamic_truth_table tt_zero( 0 );
    _storage->data.cache.insert( tt_zero ); // func-id: 0

    _storage->nodes[0].data[1].h1 = 0; // constant zero is alread insert to the storate->nodes

    static uint64_t _buf = 0x2;
    kitty::dynamic_truth_table tt_buf( 1 ); // func-id: 2
    kitty::create_from_words( tt_buf, &_buf, &_buf + 1 );
    _storage->data.cache.insert( tt_buf );

    static uint64_t _and = 0x8;
    kitty::dynamic_truth_table tt_and( 2 ); // func-id: 4
    kitty::create_from_words( tt_and, &_and, &_and + 1 );
    _storage->data.cache.insert( tt_and );

    static uint64_t _or = 0xe;
    kitty::dynamic_truth_table tt_or( 2 ); // func-id: 6
    kitty::create_from_words( tt_or, &_or, &_or + 1 );
    _storage->data.cache.insert( tt_or );

    static uint64_t _lt = 0x4;
    kitty::dynamic_truth_table tt_lt( 2 ); // func-id: 8
    kitty::create_from_words( tt_lt, &_lt, &_lt + 1 );
    _storage->data.cache.insert( tt_lt );

    static uint64_t _le = 0xd;
    kitty::dynamic_truth_table tt_le( 2 ); // func-id: 11
    kitty::create_from_words( tt_le, &_le, &_le + 1 );
    _storage->data.cache.insert( tt_le );

    static uint64_t _xor = 0x6;
    kitty::dynamic_truth_table tt_xor( 2 ); // func-id: 12
    kitty::create_from_words( tt_xor, &_xor, &_xor + 1 );
    _storage->data.cache.insert( tt_xor );

    static uint64_t _maj = 0xe8;
    kitty::dynamic_truth_table tt_maj( 3 ); // func-id: 14
    kitty::create_from_words( tt_maj, &_maj, &_maj + 1 );
    _storage->data.cache.insert( tt_maj );

    static uint64_t _ite = 0xd8;
    kitty::dynamic_truth_table tt_ite( 3 ); // func-id: 16
    kitty::create_from_words( tt_ite, &_ite, &_ite + 1 );
    _storage->data.cache.insert( tt_ite );

    static uint64_t _xor3 = 0x96;
    kitty::dynamic_truth_table tt_xor3( 3 ); // func-id: 18
    kitty::create_from_words( tt_xor3, &_xor3, &_xor3 + 1 );
    _storage->data.cache.insert( tt_xor3 );

    static uint64_t _nand3 = 0x7f;            // !(a and b and c) 0111,1111
    kitty::dynamic_truth_table tt_nand3( 3 ); // func-id: 21
    kitty::create_from_words( tt_nand3, &_nand3, &_nand3 + 1 );
    _storage->data.cache.insert( tt_nand3 );

    static uint64_t _nor3 = 0x01;            // !(a or b or c) 0000,0001
    kitty::dynamic_truth_table tt_nor3( 3 ); // func-id: 23
    kitty::create_from_words( tt_nor3, &_nor3, &_nor3 + 1 );
    _storage->data.cache.insert( tt_nor3 );

    static uint64_t _aoi21 = 0x15;            // !((a and b) or c) 0001,0101
    kitty::dynamic_truth_table tt_aoi21( 3 ); // func-id: 25
    kitty::create_from_words( tt_aoi21, &_aoi21, &_aoi21 + 1 );
    _storage->data.cache.insert( tt_aoi21 );

    static uint64_t _oai21 = 0x57;            // !((a or b) and c) 0101,0111
    kitty::dynamic_truth_table tt_oai21( 3 ); // func-id: 27
    kitty::create_from_words( tt_oai21, &_oai21, &_oai21 + 1 );
    _storage->data.cache.insert( tt_oai21 );

    static uint64_t _axi21 = 0x95;            // !((a and b) xor c) 1001,0101
    kitty::dynamic_truth_table tt_axi21( 3 ); // func-id: 29
    kitty::create_from_words( tt_axi21, &_axi21, &_axi21 + 1 );
    _storage->data.cache.insert( tt_axi21 );

    static uint64_t _xai21 = 0xd7;            // !((a xor b) and c) 1101,0111
    kitty::dynamic_truth_table tt_xai21( 3 ); // func-id: 31
    kitty::create_from_words( tt_xai21, &_xai21, &_xai21 + 1 );
    _storage->data.cache.insert( tt_xai21 );

    static uint64_t _oxi21 = 0xa9;            // !((a or b) xor c) 1010,1001
    kitty::dynamic_truth_table tt_oxi21( 3 ); // func-id: 33
    kitty::create_from_words( tt_oxi21, &_oxi21, &_oxi21 + 1 );
    _storage->data.cache.insert( tt_oxi21 );

    static uint64_t _xoi21 = 0x41;            // !((a xor b) or c) 0100,0001
    kitty::dynamic_truth_table tt_xoi21( 3 ); // func-id: 35
    kitty::create_from_words( tt_xoi21, &_xoi21, &_xoi21 + 1 );
    _storage->data.cache.insert( tt_xoi21 );
  }

  /**
   * @brief create the basic gate through literal of its truth table
   * @param children
   * @param literal
   */
  signal _create_node( std::vector<signal> const& children, uint32_t literal )
  {
    assert( children.size() <= 3u );

    const auto index = _storage->nodes.size();
    auto& node = _storage->nodes.emplace_back();
    node.data[1].h1 = literal;
    for ( auto child : children )
    {
      node.children.push_back( child );
    }

    // increase ref-count to children
    for ( auto c : children )
    {
      _storage->nodes[c.index].data[0].h1++;
    }

    for ( auto const& fn : _events->on_add )
    {
      ( *fn )( index );
    }

    return { index, 0 };
  }

public:
#pragma region Primary I / O and constants
  signal get_constant( bool value ) const
  {
    return { 0, static_cast<uint64_t>( value ? 1 : 0 ) };
  }

  signal create_pi( std::string const& name = std::string() )
  {
    (void)name;
    const auto index = _storage->nodes.size();
    auto& node = _storage->nodes.emplace_back();
    node.data[1].h1 = 1; // mark the literal as PI
    _storage->inputs.emplace_back( index );
    return { index, 0 };
  }

  uint32_t create_po( signal const& f, std::string const& name = std::string() )
  {
    (void)name;
    /* increase ref-count to children */
    _storage->nodes[f.index].data[0].h1++;
    auto const po_index = _storage->outputs.size();
    _storage->outputs.emplace_back( f.index, f.complement );
    return static_cast<uint32_t>( po_index );
  }

  bool is_combinational() const
  {
    return true;
  }

  bool is_constant( node const& n ) const
  {
    return n == 0;
  }

  bool is_ci( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 1;
  }

  bool is_pi( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 1 && !is_constant( n );
  }

  bool constant_value( node const& n ) const
  {
    (void)n;
    return false;
  }
#pragma endregion

#pragma region Create unary functions
  signal create_buf( signal const& a )
  {
    return a;
  }

  signal create_not( signal const& a )
  {
    return !a;
  }
#pragma endregion

#pragma region Create binary functions
  signal create_node( std::vector<signal> const& children, kitty::dynamic_truth_table const& function )
  {
    if ( children.size() == 0u )
    {
      assert( function.num_vars() == 0u );
      return get_constant( !kitty::is_const0( function ) );
    }
    return _create_node( children, _storage->data.cache.insert( function ) );
  }

  signal create_and( signal a, signal b )
  {
    return _create_node( { a, b }, 4 );
  }

  signal create_nand( signal a, signal b )
  {
    return _create_node( { a, b }, 5 );
  }

  signal create_or( signal a, signal b )
  {
    return _create_node( { a, b }, 6 );
  }

  signal create_nor( signal a, signal b )
  {
    return _create_node( { a, b }, 7 );
  }

  signal create_lt( signal const& a, signal const& b )
  {
    return create_and( !a, b );
  }

  signal create_le( signal const& a, signal const& b )
  {
    return !create_and( a, !b );
  }

  signal create_xor( signal a, signal b )
  {
    return _create_node( { a, b }, 12 );
  }

  signal create_xnor( signal a, signal b )
  {
    return _create_node( { a, b }, 13 );
  }
#pragma endregion

#pragma region Createy ternary functions
  signal create_maj( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 14 );
  }

  signal create_ite( signal i, signal t, signal e )
  {
    return _create_node( { i, t, e }, 16 );
  }

  signal create_xor3( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 18 );
  }

  signal create_mux21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 16 ); // same as ite
  }

  signal create_nmux21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 17 );
  }

  signal create_nand3( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 21 );
  }

  signal create_nor3( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 23 );
  }

  signal create_aoi21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 25 );
  }

  signal create_oai21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 27 );
  }

  signal create_axi21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 29 );
  }

  signal create_xai21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 31 );
  }

  signal create_oxi21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 33 );
  }

  signal create_xoi21( signal a, signal b, signal c )
  {
    return _create_node( { a, b, c }, 35 );
  }

#pragma endregion

#pragma region Create nary functions
  signal create_nary_and( std::vector<signal> const& fs )
  {
    return tree_reduce( fs.begin(), fs.end(), get_constant( true ), [this]( auto const& a, auto const& b ) { return create_and( a, b ); } );
  }

  signal create_nary_or( std::vector<signal> const& fs )
  {
    return tree_reduce( fs.begin(), fs.end(), get_constant( false ), [this]( auto const& a, auto const& b ) { return create_or( a, b ); } );
  }

  signal create_nary_xor( std::vector<signal> const& fs )
  {
    return tree_reduce( fs.begin(), fs.end(), get_constant( false ), [this]( auto const& a, auto const& b ) { return create_xor( a, b ); } );
  }
#pragma endregion

#pragma region Create arbitrary functions
  signal clone_node( gtg_network const& other, node const& source, std::vector<signal> const& children )
  {
    assert( !children.empty() );
    const auto tt = other._storage->data.cache[other._storage->nodes[source].data[1].h1];
    return create_node( children, tt );
  }
#pragma endregion

#pragma region Structural properties
  bool is_dead( node const& n ) const
  {
    return ( _storage->nodes[n].data[0].h1 >> 31 ) & 1;
  }

  auto size() const
  {
    return static_cast<uint32_t>( _storage->nodes.size() );
  }

  auto num_cis() const
  {
    return static_cast<uint32_t>( _storage->inputs.size() );
  }

  auto num_cos() const
  {
    return static_cast<uint32_t>( _storage->outputs.size() );
  }

  auto num_pis() const
  {
    return static_cast<uint32_t>( _storage->inputs.size() );
  }

  auto num_pos() const
  {
    return static_cast<uint32_t>( _storage->outputs.size() );
  }

  auto num_gates() const
  {
    auto ngates = size() - num_cis() - 1;
    return ngates;
  }

  uint32_t fanin_size( node const& n ) const
  {
    if ( is_constant( n ) || is_ci( n ) )
      return 0;
    return 2;
  }

  uint32_t fanout_size( node const& n ) const
  {
    return _storage->nodes[n].data[0].h1 & UINT32_C( 0x7FFFFFFF );
  }

  uint32_t incr_fanout_size( node const& n ) const
  {
    return _storage->nodes[n].data[0].h1++ & UINT32_C( 0x7FFFFFFF );
  }

  uint32_t decr_fanout_size( node const& n ) const
  {
    return --_storage->nodes[n].data[0].h1 & UINT32_C( 0x7FFFFFFF );
  }

  bool is_and( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 4;
  }

  bool is_nand( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 5;
  }

  bool is_or( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 6;
  }

  bool is_nor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 7;
  }

  bool is_lt( node const& n ) const
  {
    return false;
  }

  bool is_le( node const& n ) const
  {
    return false;
  }

  bool is_xor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 12;
  }

  bool is_xnor( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 13;
  }

  bool is_maj( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 14;
  }

  bool is_ite( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 16;
  }

  bool is_xor3( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 18;
  }

  bool is_mux21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 16;
  }

  bool is_nmux21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 17;
  }

  bool is_nand3( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 21;
  }

  bool is_nor3( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 23;
  }

  bool is_aoi21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 25;
  }

  bool is_oai21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 27;
  }
  bool is_axi21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 29;
  }

  bool is_xai21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 31;
  }
  bool is_oxi21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 33;
  }

  bool is_xoi21( node const& n ) const
  {
    return _storage->nodes[n].data[1].h1 == 35;
  }

  bool is_nary_and( node const& n ) const
  {
    (void)n;
    return false;
  }

  bool is_nary_or( node const& n ) const
  {
    (void)n;
    return false;
  }

  bool is_nary_xor( node const& n ) const
  {
    (void)n;
    return false;
  }

#pragma endregion

#pragma region Functional properties
  kitty::dynamic_truth_table node_function( const node& n ) const
  {
    return _storage->data.cache[_storage->nodes[n].data[1].h1];
  }
#pragma endregion

#pragma region Nodes and signals
  node get_node( signal const& f ) const
  {
    return f.index;
  }

  signal make_signal( node const& n ) const
  {
    return signal( n, 0 );
  }

  bool is_complemented( signal const& f ) const
  {
    return f.complement;
  }

  uint32_t node_to_index( node const& n ) const
  {
    return static_cast<uint32_t>( n );
  }

  node index_to_node( uint32_t index ) const
  {
    return index;
  }

  node ci_at( uint32_t index ) const
  {
    assert( index < _storage->inputs.size() );
    return *( _storage->inputs.begin() + index );
  }

  signal co_at( uint32_t index ) const
  {
    assert( index < _storage->outputs.size() );
    return *( _storage->outputs.begin() + index );
  }

  node pi_at( uint32_t index ) const
  {
    assert( index < _storage->inputs.size() );
    return *( _storage->inputs.begin() + index );
  }

  signal po_at( uint32_t index ) const
  {
    assert( index < _storage->outputs.size() );
    return *( _storage->outputs.begin() + index );
  }

  uint32_t ci_index( node const& n ) const
  {
    assert( _storage->nodes[n].children[0].data == _storage->nodes[n].children[1].data );
    return static_cast<uint32_t>( _storage->nodes[n].children[0].data );
  }

  uint32_t co_index( signal const& s ) const
  {
    uint32_t i = -1;
    foreach_co( [&]( const auto& x, auto index ) {
      if ( x == s )
      {
        i = index;
        return false;
      }
      return true;
    } );
    return i;
  }

  uint32_t pi_index( node const& n ) const
  {
    assert( _storage->nodes[n].children[0].data == _storage->nodes[n].children[1].data );
    return static_cast<uint32_t>( _storage->nodes[n].children[0].data );
  }

  uint32_t po_index( signal const& s ) const
  {
    uint32_t i = -1;
    foreach_po( [&]( const auto& x, auto index ) {
      if ( x == s )
      {
        i = index;
        return false;
      }
      return true;
    } );
    return i;
  }
#pragma endregion

#pragma region Node and signal iterators
  template<typename Fn>
  void foreach_node( Fn&& fn ) const
  {
    auto r = range<uint64_t>( _storage->nodes.size() );
    detail::foreach_element_if(
        r.begin(), r.end(),
        [this]( auto n ) { return !is_dead( n ); },
        fn );
  }

  template<typename Fn>
  void foreach_ci( Fn&& fn ) const
  {
    detail::foreach_element( _storage->inputs.begin(), _storage->inputs.end(), fn );
  }

  template<typename Fn>
  void foreach_co( Fn&& fn ) const
  {
    detail::foreach_element( _storage->outputs.begin(), _storage->outputs.end(), fn );
  }

  template<typename Fn>
  void foreach_pi( Fn&& fn ) const
  {
    detail::foreach_element( _storage->inputs.begin(), _storage->inputs.end(), fn );
  }

  template<typename Fn>
  void foreach_po( Fn&& fn ) const
  {
    detail::foreach_element( _storage->outputs.begin(), _storage->outputs.end(), fn );
  }

  template<typename Fn>
  void foreach_gate( Fn&& fn ) const
  {
    auto r = range<uint64_t>( 1u, _storage->nodes.size() ); /* start from 1 to avoid constant */
    detail::foreach_element_if(
        r.begin(), r.end(),
        [this]( auto n ) { return !is_ci( n ) && !is_dead( n ); },
        fn );
  }

  template<typename Fn>
  void foreach_fanin( node const& n, Fn&& fn ) const
  {
    if ( n == 0 || is_ci( n ) )
      return;
    static_assert( detail::is_callable_without_index_v<Fn, signal, bool> ||
                   detail::is_callable_with_index_v<Fn, signal, bool> ||
                   detail::is_callable_without_index_v<Fn, signal, void> ||
                   detail::is_callable_with_index_v<Fn, signal, void> );
    int i = 0;
    /* we don't use foreach_element here to have better performance */
    if constexpr ( detail::is_callable_without_index_v<Fn, signal, bool> )
    {
      for ( i = 0; i < _storage->nodes[n].children.size(); ++i )
      {
        if ( !fn( signal{ _storage->nodes[n].children[i] } ) )
          return;
      }
    }
    else if constexpr ( detail::is_callable_with_index_v<Fn, signal, bool> )
    {
      for ( i = 0; i < _storage->nodes[n].children.size(); ++i )
      {
        if ( !fn( signal{ _storage->nodes[n].children[i] }, i ) )
          return;
      }
    }
    else if constexpr ( detail::is_callable_without_index_v<Fn, signal, void> )
    {
      for ( i = 0; i < _storage->nodes[n].children.size(); ++i )
      {
        fn( signal{ _storage->nodes[n].children[i] } );
      }
    }
    else if constexpr ( detail::is_callable_with_index_v<Fn, signal, void> )
    {
      for ( i = 0; i < _storage->nodes[n].children.size(); ++i )
      {
        fn( signal{ _storage->nodes[n].children[i] }, i );
      }
    }
  }
#pragma endregion

#pragma region Value simulation

  template<typename Iterator>
  mockturtle::iterates_over_t<Iterator, bool>
  compute( node const& n, Iterator begin, Iterator end ) const
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
  compute( node const& n, Iterator begin, Iterator end ) const
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
#pragma endregion

#pragma region Custom node values
  void clear_values() const
  {
    std::for_each( _storage->nodes.begin(), _storage->nodes.end(), []( auto& n ) { n.data[0].h2 = 0; } );
  }

  auto value( node const& n ) const
  {
    return _storage->nodes[n].data[0].h2;
  }

  void set_value( node const& n, uint32_t v ) const
  {
    _storage->nodes[n].data[0].h2 = v;
  }

  auto incr_value( node const& n ) const
  {
    return _storage->nodes[n].data[0].h2++;
  }

  auto decr_value( node const& n ) const
  {
    return --_storage->nodes[n].data[0].h2;
  }
#pragma endregion

#pragma region Visited flags
  void clear_visited() const
  {
    std::for_each( _storage->nodes.begin(), _storage->nodes.end(), []( auto& n ) { n.data[1].h2 = 0; } );
  }

  auto visited( node const& n ) const
  {
    return _storage->nodes[n].data[1].h2;
  }

  void set_visited( node const& n, uint32_t v ) const
  {
    _storage->nodes[n].data[1].h2 = v;
  }

  uint32_t trav_id() const
  {
    return _storage->trav_id;
  }

  void incr_trav_id() const
  {
    ++_storage->trav_id;
  }
#pragma endregion

#pragma region General methods
  auto& events() const
  {
    return *_events;
  }
#pragma endregion

public:
  std::shared_ptr<gtg_storage> _storage;
  std::shared_ptr<network_events<base_type>> _events;
};

} // namespace mockturtle

namespace std
{

template<>
struct hash<mockturtle::gtg_network::signal>
{
  uint64_t operator()( mockturtle::gtg_network::signal const& s ) const noexcept
  {
    uint64_t k = s.data;
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;
    return k;
  }
}; /* hash */

} // namespace std