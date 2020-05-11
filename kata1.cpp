#include <eosio/eosio.hpp>
#include <eosio.token/eosio.token.hpp>

using namespace eosio;

class [[eosio::contract("kata1")]] kata1 : public eosio::contract {
private:
    struct [[eosio::table]] acc_type_index
    {
        name type;
        //std::map<symbol, asset> balance;
        asset balance;
        uint64_t primary_key() const { return type.value; }
    };

    using acc_table = eosio::multi_index<"acc.type.ind"_n, acc_type_index>;

    static name def_account()
    {
        static name def ("default");
        return def;
    }

    void updateBalance(name acc, name type, asset balance)
    {
        acc_table types(get_self(), acc.value);

        auto it = types.find(type.value);
        if(it == types.end())
        {
            print("inserting ", type, "\n");
            check(balance > asset(0, balance.symbol), "trying to create negative balance!");
            types.emplace(get_self(), [&](auto &row) {
                row.type = def_account();
                row.balance = balance;
            });
        }
        else
        {
            print("updating ", type, "\n");
            auto& cur_balance = it->balance;
            check(cur_balance + balance >= asset(0, balance.symbol), "trying to create negative balance!");
                types.modify(it, acc, [&](auto& row){
                    row.type = type;
                    row.balance += balance;
                });
        }
    }

public:
    using contract::contract;

    kata1(name receiver, name code, datastream<const char *> ds) 
    : contract(receiver, code, ds)
    {
        print("kata1::contract(", get_self(), "): ", receiver, ", ", code, ", ds<>\n");
    }

    [[eosio::action]]
    void init()
    {
        acc_table types(get_self(), get_self().value);
        auto it = types.find(def_account().value);
        if (it == types.end())
        {
            types.emplace(get_self(), [&](auto &row) {
                row.type = def_account();
                row.balance = eosio::token::get_balance("eosio.token"_n, get_self(), symbol_code("SYS"));
            });
        }
    }

    [[eosio::action]]
    void addtype(name type)
    {
        require_auth(get_self());

        acc_table types(get_self(), get_self().value);
        
        check(types.find(type.value) == types.end(), "Account type with such name already exists");

        types.emplace(get_self(), [&](auto &row) {
            row.type = type;
            row.balance = asset(0, symbol("SYS", 4));
        });
    }

    [[eosio::action]]
    void listtypes()
    {
        require_auth(get_self());
        acc_table types(get_self(), get_self().value);

        for (auto it = types.begin(); it != types.end(); ++it)
        {
            print(it->type, "=", it->type.value, "\n");
        }
    }

    [[eosio::action]]
    void printbal()
    {
        require_auth(get_self());
        acc_table types(get_self(), get_self().value);

        for (auto it = types.begin(); it != types.end(); ++it)
        {
            print(it->type, ":\n");
            print(it->balance.to_string(), "\n");
        }
    }

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name from, name to, eosio::asset balance, std::string memo) 
    {
        print("deposit called(", get_self(), "): ", from, ", ", to, ", balance=", balance, ", ", memo, "\n");
        if (to != get_self() && from != get_self())
            return;
        
        //check(to == get_self(), "use kata1::deposit instead");

        check(balance.amount > 0, "zero quantity, aborting");

        if (to == get_self())
            updateBalance(to, def_account(), balance);
        else
            updateBalance(from, def_account(), -balance);
    }

    [[eosio::action]]
    void deposit(name from, name to, name type, asset balance)
    {
        require_auth(from);
        check(get_self() == to || get_self() == from, "transfer should be either from this account or to this account");

        action{
            permission_level{get_self(), "active"_n},
            "eosio.token"_n,
            "transfer"_n,
            std::make_tuple(get_self(), to, balance, std::string("kata1 deposit"))
        }.send();

        updateBalance(get_self(), type, balance);
    }

    [[eosio::action]]
    void transfer(name from, name from_type, name to, name to_type, asset balance)
    {
        require_auth(from);
        //transfer between owner accounts
        if (from == to)
        {
            //throws if balance greater than we have
            updateBalance(from, from_type, -balance);
            updateBalance(to, to_type, balance);
        }
        //from owner outside
        else
        {
            action{
                permission_level{get_self(), "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(get_self(), to, balance, std::string("kata1 transfer"))
            }.send();
        }
    }
};