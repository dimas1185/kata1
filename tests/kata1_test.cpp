#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/testing/tester.hpp>
#include <contracts.hpp>

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

BOOST_AUTO_TEST_SUITE(kata1_tests)

BOOST_AUTO_TEST_CASE(post) try {
    tester t{setup_policy::none};

    // Load contracts
    t.create_account(N(eosio.token), config::system_account_name, false, true);
    t.set_code(N(eosio.token), eosio::testing::contracts::token_wasm());
    t.set_abi(N(eosio.token), eosio::testing::contracts::token_abi().data());

    t.create_account(N(kata1), config::system_account_name, false, true);
    t.set_code(N(kata1), eosio::testing::contracts::kata1_wasm());
    t.set_abi(N(kata1), eosio::testing::contracts::kata1_abi().data());
    t.produce_block();

    // Create users
    //t.create_account(N(eosio.token));
    //t.create_account(N(bob));

    // create SYS token
    t.push_action(
        N(eosio.token), N(create), N(eosio.token),
        mutable_variant_object
        ("issuer", "eosio.token")
        ("maximum_supply", "1000000000.0000 SYS")
    );

    //issue SYS tokens
    t.push_action(
        N(eosio.token), N(issue), N(eosio.token),
        mutable_variant_object
        ("to", "eosio.token")
        ("quantity", "1000.0000 SYS")
        ("memo", "memo")
    );

    //transfer some to kata1
    t.push_action(
        N(eosio.token), N(transfer), N(eosio.token),
        mutable_variant_object
        ("from", "eosio.token")
        ("to", "kata1")
        ("quantity", "100.0000 SYS")
        ("memo", "memo")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "100.0000 SYS")
    );

    //test print balance
    t.push_action(
        N(kata1), N(printbal), N(kata1),
        mutable_variant_object
        ()
    );

    //teest add account type
    t.push_action(
        N(kata1), N(addtype), N(kata1),
        mutable_variant_object
        ("type", "checking")
    );

    // Can't duplicate account type
    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                N(kata1), N(addtype), N(kata1),
                mutable_variant_object
                ("type", "checking")
            );
        }(),
    fc::exception);

    //test list types
    t.push_action(
        N(kata1), N(listtypes), N(kata1),
        mutable_variant_object
        ()
    );

    //transfer between types
    t.push_action(
        N(kata1), N(transfer), N(kata1),
        mutable_variant_object
        ("from", "kata1")
        ("from_type", "default")
        ("to", "kata1")
        ("to_type", "checking")
        ("balance", "10.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "90.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "checking")
        ("balance", "10.0000 SYS")
    );

    //transfer between types
    t.push_action(
        N(kata1), N(transfer), N(kata1),
        mutable_variant_object
        ("from", "kata1")
        ("from_type", "checking")
        ("to", "kata1")
        ("to_type", "default")
        ("balance", "5.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "95.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "checking")
        ("balance", "5.0000 SYS")
    );

    //transfer between types
    t.push_action(
        N(kata1), N(transfer), N(kata1),
        mutable_variant_object
        ("from", "kata1")
        ("from_type", "default")
        ("to", "eosio.token")
        ("to_type", "")
        ("balance", "10.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "85.0000 SYS")
    );

    //try to overdraft
    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                N(kata1), N(transfer), N(kata1),
                mutable_variant_object
                ("from", "kata1")
                ("from_type", "default")
                ("to", "eosio.token")
                ("to_type", "")
                ("balance", "90.0000 SYS")
            );
        }(),
    fc::exception);
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()