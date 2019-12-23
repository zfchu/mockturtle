#include <catch.hpp>
#include <vector>
#include <mockturtle/networks/aoig.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/operators.hpp>

using namespace mockturtle;

TEST_CASE("1/create and use AND/OR in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_size_v<aoig_network>);
    CHECK(has_create_and_v<aoig_network>);
    CHECK(has_create_or_v<aoig_network>);
    CHECK(has_create_pi_v<aoig_network>);
    CHECK(has_create_po_v<aoig_network>);
    CHECK(has_num_pos_v<aoig_network>);
    CHECK(has_num_pis_v<aoig_network>);

    const auto a = aoig_create_pi();
    const auto b = aoig_create_pi();

    const auto c = aoig_create_and(a,b);
    const auto d = aoig_create_or(a,b);

    aoig.create_po(c);
    aoig.create_po(d);

    CHECK(aoig.size() == 6);
    CHECK(aoig.num_pis() == 2);
    CHECK(aoig.num_pos() == 2);
}

TEST_CASE("2/compute functions from AND and NOT gates in aoig networks", "[aoig]")
{
    aoig_network aoig;

    const auto a = aoig.create_pi();
    const auto b = aoig.create_pi();

    const auto f1 = aoig.create_not(a);
    const auto f2 = aoig.create_and(a, b);

    std::vector<kitty::dynamic_truth_table> xs;
    xs.emplace_back(3u);
    xs.emplace_back(3u);
    kitty::create_nth_var(xs[0], 0);
    kitty::create_nth_var(xs[1], 1);

    const auto sim_f1 = aoig.compute(aoig.get_node(f1), xs.begin(), xs.begin() + 1);
    const auto sim_f2 = aoig.compute(aoig.get_node(f2), xs.begin(), xs.end());

    CHECK(sim_f1 == ~xs[0]);
    CHECK(sim_f2 == (xs[0] & xs[1]));
}

TEST_CASE("3/create and use XOR in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_size_v<aoig_network>);
    CHECK(has_create_xor_v<aoig_network>);
    CHECK(has_create_pi_v<aoig_network>);
    CHECK(has_create_po_v<aoig_network>);
    CHECK(has_num_pos_v<aoig_network>);
    CHECK(has_num_pis_v<aoig_network>);

    const auto a = aoig_create_pi();
    const auto b = aoig_create_pi();

    const auto c = aoig_create_xor(a, b);

    aoig.create_po(c);

    CHECK(aoig.size() == 5);
    CHECK(aoig.num_pis() == 2);
    CHECK(aoig.num_pos() == 2);
}

TEST_CASE("4/create and use MUX in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_size_v<aoig_network>);
    //CHECK(has_create_mux_v<aoig_network>);
    CHECK(has_create_pi_v<aoig_network>);
    CHECK(has_create_po_v<aoig_network>);
    CHECK(has_num_pos_v<aoig_network>);
    CHECK(has_num_pis_v<aoig_network>);

    const auto a = aoig_create_pi();
    const auto b = aoig_create_pi();
    const auto c = aoig_create_pi();

    const auto d = aoig_create_mux(a, b,c);

    aoig.create_po(d);

    CHECK(aoig.size() == 6);
    CHECK(aoig.num_pis() == 3);
    CHECK(aoig.num_pos() == 1);
}


TEST_CASE("5/create and use primary inputs in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_create_pi_v<aoig_network>);
    CHECK(has_is_constant_v<aoig_network>);
    CHECK(has_is_pi_v<aoig_network>);
    CHECK(has_num_pis_v<aoig_network>);

    CHECK(aoig.num_pis() == 0);

    auto x1 = aoig.create_pi();
    auto x2 = aoig.create_pi();

    CHECK(aoig.size() == 4);
    CHECK(aoig.num_pis() == 2);
    CHECK(x1 != x2);
}

TEST_CASE("6/create and use primary outputs in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_create_po_v<aoig_network>);
    CHECK(has_num_pos_v<aoig_network>);

    auto c0 = aoig.get_constant(false);
    auto c1 = aoig.get_constant(true);
    auto x = aoig.create_pi();

    aoig.create_po(c0);
    aoig.create_po(c1);
    aoig.create_po(x);

    CHECK(aoig.size() == 3);
    CHECK(aoig.num_pis() == 1);
    CHECK(aoig.num_pos() == 3);
}

TEST_CASE("7/create unary operations in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_create_buf_v<aoig_network>);
    CHECK(has_create_not_v<aoig_network>);

    auto x1 = aoig.create_pi();

    CHECK(aoig.size() == 3);

    auto f1 = aoig.create_buf(x1);
    auto f2 = aoig.create_not(x1);

    CHECK(aoig.size() == 4);
    CHECK(f1 == x1);
    CHECK(f2 != x1);
}

TEST_CASE("8/clone a node in a aoig network", "[aoig]")
{
    aoig_network aoig1, aoig2;

    CHECK(has_clone_node_v<aoig_network>);

    auto a1 = aoig1.create_pi();
    auto b1 = aoig1.create_pi();
    auto f1 = aoig1.create_and(a1, b1);
    CHECK(aoig1.size() == 5);

    auto a2 = aoig2.create_pi();
    auto b2 = aoig2.create_pi();
    CHECK(aoig2.size() == 4);

    auto f2 = aoig2.clone_node(aoig1, aoig1.get_node(f1), { a2, b2 });
    CHECK(aoig2.size() == 5);

    aoig2.foreach_fanin(aoig2.get_node(f2), [&](auto const& s) {
        CHECK(!aoig2.is_complemented(s));
    });
}

TEST_CASE("9/hash nodes in aoig network", "[aoig]")
{
    aoig_network aoig;

    const auto a = aoig.create_pi();
    const auto b = aoig.create_pi();
    const auto c = aoig.create_pi();

    kitty::dynamic_truth_table tt_mux(3u), tt_xor(2u);
    kitty::create_from_hex_string(tt_mux, "d8");
    kitty::create_from_hex_string(tt_xor, "e");

    aoig.create_node({ a, b, c }, tt_mux);
    aoig.create_node({ a, b }, tt_xor);

    CHECK(aoig.size() == 7);

    aoig.create_node({ a, b, c }, tt_mux);

    CHECK(aoig.size() == 7);
}

TEST_CASE("10/subsitute node by another", "[aoig]")
{
    aoig_network aoig;

    const auto c0 = aoig.get_node(aoig.get_constant(false));
    const auto c1 = aoig.get_node(aoig.get_constant(true));
    const auto a = aoig.create_pi();
    const auto b = aoig.create_pi();

    kitty::dynamic_truth_table tt_nand(2u),  tt_or(2u);
    kitty::create_from_hex_string(tt_nand, "7");
    kitty::create_from_hex_string(tt_or, "e");

    // XOR with NAND
    const auto n1 = aoig.create_node({ a, b }, tt_nand);
    const auto n2 = aoig.create_node({ a, n1 }, tt_nand);
    const auto n3 = aoig.create_node({ b, n1 }, tt_nand);
    const auto n4 = aoig.create_node({ n2, n3 }, tt_nand);
    aoig.create_po(n4);

    std::vector<node<aoig_network>> nodes;
    aoig.foreach_node([&](auto node) { nodes.push_back(node); });

    CHECK(nodes == std::vector<aoig_network::node>{c0, c1, a, b, n1, n2, n3, n4});
    CHECK(aoig.fanout_size(n4) == 1);
    aoig.foreach_po([&](auto f) {
        CHECK(f == n4);
        return false;
    });
}

TEST_CASE("11/structural properties of a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_size_v<aoig_network>);
    CHECK(has_num_pis_v<aoig_network>);
    CHECK(has_num_pos_v<aoig_network>);
    CHECK(has_num_gates_v<aoig_network>);
    CHECK(has_fanin_size_v<aoig_network>);
    CHECK(has_fanout_size_v<aoig_network>);

    const auto x1 = aoig.create_pi();
    const auto x2 = aoig.create_pi();

    const auto f1 = aoig.create_and(x1, x2);
    const auto f2 = aoig.create_and(x2, x1);

    aoig.create_po(f1);
    aoig.create_po(f2);

    CHECK(aoig.size() == 6);
    CHECK(aoig.num_pis() == 2);
    CHECK(aoig.num_pos() == 2);
    CHECK(aoig.num_gates() == 2);
    CHECK(aoig.fanin_size(aoig.get_node(x1)) == 0);
    CHECK(aoig.fanin_size(aoig.get_node(x2)) == 0);
    CHECK(aoig.fanin_size(aoig.get_node(f1)) == 2);
    CHECK(aoig.fanin_size(aoig.get_node(f2)) == 2);
    CHECK(aoig.fanout_size(aoig.get_node(x1)) == 2);
    CHECK(aoig.fanout_size(aoig.get_node(x2)) == 2);
    CHECK(aoig.fanout_size(aoig.get_node(f1)) == 1);
    CHECK(aoig.fanout_size(aoig.get_node(f2)) == 1);
}

TEST_CASE("12/node and signal iteration in a aoig network", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_foreach_node_v<aoig_network>);
    CHECK(has_foreach_pi_v<aoig_network>);
    CHECK(has_foreach_po_v<aoig_network>);
    CHECK(has_foreach_fanin_v<aoig_network>);

    const auto x1 = aoig.create_pi();
    const auto x2 = aoig.create_pi();
    const auto f1 = aoig.create_and(x1, x2);
    const auto f2 = aoig.create_and(x2, x1);
    aoig.create_po(f1);
    aoig.create_po(f2);

    CHECK(aoig.size() == 6);

    /* iterate over nodes */
    uint32_t mask{ 0 }, counter{ 0 };
    aoig.foreach_node([&](auto n, auto i) { mask |= (1 << n); counter += i; });
    CHECK(mask == 63);
    CHECK(counter == 15);

    mask = 0;
    aoig.foreach_node([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 63);

    mask = counter = 0;
    aoig.foreach_node([&](auto n, auto i) { mask |= (1 << n); counter += i; return false; });
    CHECK(mask == 1);
    CHECK(counter == 0);

    mask = 0;
    aoig.foreach_node([&](auto n) { mask |= (1 << n); return false; });
    CHECK(mask == 1);

    /* iterate over PIs */
    mask = counter = 0;
    aoig.foreach_pi([&](auto n, auto i) { mask |= (1 << n); counter += i; });
    CHECK(mask == 12);
    CHECK(counter == 1);

    mask = 0;
    aoig.foreach_pi([&](auto n) { mask |= (1 << n); });
    CHECK(mask == 12);

    mask = counter = 0;
    aoig.foreach_pi([&](auto n, auto i) { mask |= (1 << n); counter += i; return false; });
    CHECK(mask == 4);
    CHECK(counter == 0);

    mask = 0;
    aoig.foreach_pi([&](auto n) { mask |= (1 << n); return false; });
    CHECK(mask == 4);

    /* iterate over POs */
    mask = counter = 0;
    aoig.foreach_po([&](auto s, auto i) { mask |= (1 << aoig.get_node(s)); counter += i; });
    CHECK(mask == 48);
    CHECK(counter == 1);

    mask = 0;
    aoig.foreach_po([&](auto s) { mask |= (1 << aoig.get_node(s)); });
    CHECK(mask == 48);

    mask = counter = 0;
    aoig.foreach_po([&](auto s, auto i) { mask |= (1 << aoig.get_node(s)); counter += i; return false; });
    CHECK(mask == 16);
    CHECK(counter == 0);

    mask = 0;
    aoig.foreach_po([&](auto s) { mask |= (1 << aoig.get_node(s)); return false; });
    CHECK(mask == 16);
}

TEST_CASE("13/custom node values in aoig networks", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_clear_values_v<aoig_network>);
    CHECK(has_value_v<aoig_network>);
    CHECK(has_set_value_v<aoig_network>);
    CHECK(has_incr_value_v<aoig_network>);
    CHECK(has_decr_value_v<aoig_network>);

    const auto x1 = aoig.create_pi();
    const auto x2 = aoig.create_pi();
    const auto f1 = aoig.create_and(x1, x2);
    const auto f2 = aoig.create_and(x2, x1);
    aoig.create_po(f1);
    aoig.create_po(f2);

    CHECK(aoig.size() == 6);

    aoig.clear_values();
    aoig.foreach_node([&](auto n) {
        CHECK(aoig.value(n) == 0);
        aoig.set_value(n, static_cast<uint32_t>(n));
        CHECK(aoig.value(n) == n);
        CHECK(aoig.incr_value(n) == n);
        CHECK(aoig.value(n) == n + 1);
        CHECK(aoig.decr_value(n) == n);
        CHECK(aoig.value(n) == n);
    });
    aoig.clear_values();
    aoig.foreach_node([&](auto n) {
        CHECK(aoig.value(n) == 0);
    });
}

TEST_CASE("14/visited values in aoig networks", "[aoig]")
{
    aoig_network aoig;

    CHECK(has_clear_visited_v<aoig_network>);
    CHECK(has_visited_v<aoig_network>);
    CHECK(has_set_visited_v<aoig_network>);

    const auto x1 = aoig.create_pi();
    const auto x2 = aoig.create_pi();
    const auto f1 = aoig.create_and(x1, x2);
    const auto f2 = aoig.create_and(x2, x1);
    aoig.create_po(f1);
    aoig.create_po(f2);

    CHECK(aoig.size() == 6);

    aoig.clear_visited();
    aoig.foreach_node([&](auto n) {
        CHECK(aoig.visited(n) == 0);
        aoig.set_visited(n, n);
        CHECK(aoig.visited(n) == n);
    });
    aoig.clear_visited();
    aoig.foreach_node([&](auto n) {
        CHECK(aoig.visited(n) == 0);
    });
}
