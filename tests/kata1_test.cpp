#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/testing/tester.hpp>
#include <contracts.hpp>

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;

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
        ("memo", "")
    );

    //transfer some to kata1
    t.push_action(
        N(eosio.token), N(transfer), N(eosio.token),
        mutable_variant_object
        ("from", "eosio.token")
        ("to", "kata1")
        ("quantity", "100.0000 SYS")
        ("memo", "")
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

    //transfer some to kata1
    t.push_action(
        N(eosio.token), N(transfer), N(eosio.token),
        mutable_variant_object
        ("from", "eosio.token")
        ("to", "kata1")
        ("quantity", "1.0000 SYS")
        ("memo", "checking")
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "checking")
        ("balance", "6.0000 SYS")
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

    uint32_t sec_since_epoch = t.control->pending_block_time().time_since_epoch().count() / 1000000;

    //deffered transfer between types
    t.push_action(
        N(kata1), N(deffered), N(kata1),
        mutable_variant_object
        ("idx", 1)
        ("from", "kata1")
        ("from_type", "default")
        ("to", "eosio.token")
        ("to_type", "")
        ("balance", "10.0000 SYS")
        ("when", sec_since_epoch)
        ("till", 0)
        ("reccuring", false)
        ("period", 0)
    );

    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 1)
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "75.0000 SYS")
    );

    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", false)
    );

    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 0)
    );

    //deferred in past should fail
    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                N(kata1), N(deffered), N(kata1),
                mutable_variant_object
                ("idx", 1)
                ("from", "kata1")
                ("from_type", "default")
                ("to", "eosio.token")
                ("to_type", "")
                ("balance", "10.0000 SYS")
                ("when", sec_since_epoch - 1)
                ("till", 0)
                ("reccuring", false)
                ("period", 0)
            );
        }(),
    fc::exception);

    t.produce_blocks();
    sec_since_epoch = t.control->pending_block_time().time_since_epoch().count() / 1000000;

        //transfer between types
    t.push_action(
        N(kata1), N(deffered), N(kata1),
        mutable_variant_object
        ("idx", 1)
        ("from", "kata1")
        ("from_type", "default")
        ("to", "eosio.token")
        ("to_type", "")
        ("balance", "1.0000 SYS")
        ("when", sec_since_epoch)
        ("till", sec_since_epoch + 90)
        ("reccuring", true)
        ("period", 10)
    );
    t.produce_blocks();
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "74.0000 SYS")
    );

    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 1)
    );
    t.produce_blocks();

    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", false)
    );
    t.produce_blocks();
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "73.0000 SYS")
    );
    t.produce_blocks();

    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 1)
    );
    t.produce_blocks();

    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", true)
    );
    t.produce_blocks();
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "65.0000 SYS")
    );
    t.produce_blocks();

    sec_since_epoch = t.control->pending_block_time().time_since_epoch().count() / 1000000;
    //try to deposit to invalid account
    t.push_action(
        N(kata1), N(deffered), N(kata1),
        mutable_variant_object
        ("idx", 1)
        ("from", "kata1")
        ("from_type", "default")
        ("to", "invalid")
        ("to_type", "")
        ("balance", "10.0000 SYS")
        ("when", sec_since_epoch + 1)
        ("till", 0)
        ("reccuring", false)
        ("period", 0)
    );
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 1)
    );
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "55.0000 SYS")
    );
    t.produce_blocks();
    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                N(kata1), N(executedef), N(kata1),
                mutable_variant_object
                ("force", true)
            );
        }(),
    fc::exception);

    t.push_action(
        N(kata1), N(canceltrn), N(kata1),
        mutable_variant_object
        ("idx", 1)
    );
    t.produce_blocks();

    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 0)
    );

    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "65.0000 SYS")
    );
    t.produce_blocks();

    sec_since_epoch = t.control->pending_block_time().time_since_epoch().count() / 1000000;
    for (uint32_t i = 0; i < 10; i++ )
    {
        //deffered transfer between types
        t.push_action(
            N(kata1), N(deffered), N(kata1),
            mutable_variant_object
            ("idx", i)
            ("from", "kata1")
            ("from_type", "default")
            ("to", "eosio.token")
            ("to_type", "")
            ("balance", "1.0000 SYS")
            ("when", sec_since_epoch + i)
            ("till", 0)
            ("reccuring", false)
            ("period", 0)
        );
    }
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 10)
    );
    t.push_action(
        N(kata1), N(printdef), N(kata1),
        mutable_variant_object()
    );
    t.produce_blocks();

    BOOST_CHECK_THROW(
        [&] {
            t.push_action(
                N(kata1), N(deffered), N(kata1),
                mutable_variant_object
                ("idx", 11)
                ("from", "kata1")
                ("from_type", "default")
                ("to", "eosio.token")
                ("to_type", "")
                ("balance", "10.0000 SYS")
                ("when", sec_since_epoch + 10)
                ("till", 0)
                ("reccuring", false)
                ("period", 0)
            );
        }(),
    fc::exception);
    t.produce_blocks();

    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", false)
    );
    //3 blocks passed so 10-2 is 8
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 8)
    );
    t.produce_blocks();
    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", true)
    );
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 0)
    );
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "55.0000 SYS")
    );
    t.produce_blocks();

    sec_since_epoch = t.control->pending_block_time().time_since_epoch().count() / 1000000;
    //deffered transfer between types
    t.push_action(
        N(kata1), N(deffered), N(kata1),
        mutable_variant_object
        ("idx", 1)
        ("from", "kata1")
        ("from_type", "default")
        ("to", "kata1")
        ("to_type", "checking")
        ("balance", "1.0000 SYS")
        ("when", sec_since_epoch)
        ("till", 0)
        ("reccuring", false)
        ("period", 0)
    );
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 1)
    );
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "default")
        ("balance", "54.0000 SYS")
    );
    t.produce_blocks();
    t.push_action(
        N(kata1), N(executedef), N(kata1),
        mutable_variant_object
        ("force", false)
    );
    t.push_action(
        N(kata1), N(verifydef), N(kata1),
        mutable_variant_object
        ("number", 0)
    );
    t.push_action(
        N(kata1), N(verify), N(kata1),
        mutable_variant_object
        ("type", "checking")
        ("balance", "7.0000 SYS")
    );
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()