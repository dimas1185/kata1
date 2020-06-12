#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <algorithm>

using namespace eosio;

//this is copy from eos/libraries/abieos/include/eosio/chain_conversions.hpp
//it can't be used with eosio.cdt due to some redefinitions
namespace
{
using days = std::chrono::duration
    <int, std::ratio_multiply<std::ratio<24>, std::chrono::hours::period>>;

using weeks = std::chrono::duration
    <int, std::ratio_multiply<std::ratio<7>, days::period>>;

using years = std::chrono::duration
    <int, std::ratio_multiply<std::ratio<146097, 400>, days::period>>;

using months = std::chrono::duration
    <int, std::ratio_divide<years::period, std::ratio<12>>>;

template <class Duration>
using sys_time = std::chrono::time_point<std::chrono::system_clock, Duration>;

using sys_days    = sys_time<days>;
using sys_seconds = sys_time<std::chrono::seconds>;
struct day {
   inline explicit day(uint32_t d) : d(d) {}
   uint32_t d;
};
struct month {
   inline explicit month(uint32_t m) : m(m) {}
   uint32_t m;
};
struct month_day {
   inline month_day( month m, day d ) : m(m), d(d) {}
   inline auto month() const { return m; }
   inline auto day() const { return d; }
   struct month m;
   struct day d;
};
struct year {
   inline explicit year( uint32_t y )
      : y(y) {}
   uint32_t y;
};
typedef year year_t;
typedef month month_t;
typedef day day_t;
    struct year_month_day {
   inline auto from_days( days ds ) {
      const auto z = ds.count() + 719468;
      const auto era = (z >= 0 ? z : z - 146096) / 146097;
      const auto doe = static_cast<uint32_t>(z - era * 146097);
      const auto yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
      const auto y   = static_cast<days::rep>(yoe) + era * 400;
      const auto doy = doe - (365 * yoe + yoe/4 - yoe/100);
      const auto mp  = (5*doy + 2)/153;
      const auto d   = doy - (153*mp+2)/5 + 1;
      const auto m   = mp < 10 ? mp+3 : mp-9;
      return year_month_day{year_t{static_cast<uint32_t>(y + (m <= 2))}, month_t(m), day_t(d)};
   }
   inline auto to_days() const {
      const auto _y = static_cast<int>(y.y) - (m.m <= month_t{2}.m);
      const auto _m = static_cast<uint32_t>(m.m);
      const auto _d = static_cast<uint32_t>(d.d);
      const auto era = (_y >= 0 ? _y : _y-399) / 400;
      const auto yoe = static_cast<uint32_t>(_y - era * 400);
      const auto doy = (153*(_m > 2 ? _m-3 : _m+9) + 2)/5 + _d-1;
      const auto doe = yoe * 365 + yoe/4 -yoe/100 + doy;
      return days{era * 146097 + static_cast<int>(doe) - 719468};
   }
   inline year_month_day(const year_t& y, const month_t& m, const day_t& d)
      : y(y), m(m), d(d) {}
   inline year_month_day(const year_month_day&) = default;
   inline year_month_day(year_month_day&&) = default;
   inline year_month_day(sys_days ds)
      : year_month_day(from_days(ds.time_since_epoch())) {}
   inline auto year() const { return y.y; }
   inline auto month() const { return m.m; }
   inline auto day() const { return d.d; }
   year_t y;
   month_t m;
   day_t d;
};

inline std::string microseconds_to_str(uint64_t microseconds) {
   std::string result;

   auto append_uint = [&result](uint32_t value, int digits) {
      char  s[20];
      char* ch = s;
      while (digits--) {
         *ch++ = '0' + (value % 10);
         value /= 10;
      };
      std::reverse(s, ch);
      result.insert(result.end(), s, ch);
   };

   std::chrono::microseconds us{ microseconds };
   sys_days                  sd(std::chrono::floor<days>(us));
   auto                      ymd = year_month_day{ sd };
   uint32_t                  ms  = (std::chrono::floor<std::chrono::milliseconds>(us) - sd.time_since_epoch()).count();
   us -= sd.time_since_epoch();
   append_uint((int)ymd.year(), 4);
   result.push_back('-');
   append_uint((unsigned)ymd.month(), 2);
   result.push_back('-');
   append_uint((unsigned)ymd.day(), 2);
   result.push_back('T');
   append_uint(ms / 3600000 % 60, 2);
   result.push_back(':');
   append_uint(ms / 60000 % 60, 2);
   result.push_back(':');
   append_uint(ms / 1000 % 60, 2);
   result.push_back('.');
   append_uint(ms % 1000, 3);
   return result;
}
}
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
    acc_table m_types;

    struct [[eosio::table]] deffered_tx_index
    {
        uint32_t idx;
        name from;
        name from_type;
        name to;
        name to_type;
        asset balance;
        uint32_t when;
        uint32_t till;
        bool reccuring;
        uint32_t period;
        uint32_t primary_key() const { return idx; }
    };
    using tx_table = eosio::multi_index<"tx.ind"_n, deffered_tx_index>;
    tx_table m_txs;
    //deffered/reoccuring transactions limit
    static int s_trn_limit;
    static asset sys_zero;
    static name def_account;

    void updateBalance(name acc, name type, asset balance)
    {
        auto it = m_types.find(type.value);
        if(it == m_types.end())
        {
            check(balance > sys_zero, "trying to create negative balance!");
            m_types.emplace(get_self(), [&](auto &row) {
                row.type = def_account;
                row.balance = balance;
            });
        }
        else
        {
            auto& cur_balance = it->balance;
            check(cur_balance + balance >= sys_zero, "trying to create negative balance!");
            m_types.modify(it, acc, [&](auto& row){
                row.type = type;
                row.balance += balance;
            });
        }
    }

    void processReccuring(uint32_t idx, bool force)
    {
        auto it = m_txs.find(idx);
        check(it != m_txs.end(), "processReccuring: missing reccuring transaction wih given id");
        check(it->reccuring, "processReccuring: must be reccuring");

        while (it->when <= now() || force)
        {
            //moving funds back and those immediately transferred. 
            updateBalance(it->from, it->from_type, it->balance);
            transfer(it->from, it->from_type, it->to, it->to_type, it->balance);
            if (it->when + it->period > it->till)
            {
                m_txs.erase(it);
                break;
            }

            m_txs.modify(it, get_self(), [&](auto& row){
                row.when += row.period;
            });
            updateBalance(it->from, it->from_type, -it->balance);
        }
    }

    uint32_t now() {
      return current_time_point().sec_since_epoch();
    }
public:
    using contract::contract;

    kata1(name receiver, name code, datastream<const char *> ds) 
    : contract(receiver, code, ds), 
      m_types(get_self(), get_self().value),
      m_txs(get_self(), get_self().value)
    {
        //print("kata1::contract(", get_self(), "): ", receiver, ", ", code, ", ds<>\n");
    }

    [[eosio::action]]
    void addtype(name type)
    {
        require_auth(get_self());

        check(m_types.find(type.value) == m_types.end(), "Account type with such name already exists");

        m_types.emplace(get_self(), [&](auto &row) {
            row.type = type;
            row.balance = asset(0, symbol("SYS", 4));
        });
    }

    [[eosio::action]]
    void listtypes()
    {
        require_auth(get_self());
        
        for (auto it = m_types.begin(); it != m_types.end(); ++it)
        {
            print(it->type, "=", it->type.value, "\n");
        }
    }

    [[eosio::action]]
    void printbal()
    {
        require_auth(get_self());
        
        for (auto it = m_types.begin(); it != m_types.end(); ++it)
        {
            print(it->type, ":\n");
            print(it->balance.to_string(), "\n");
        }
    }

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name from, name to, eosio::asset balance, std::string memo) 
    {
        print("deposit called(", get_self(), "): ", from, ", ", to, ", balance=", balance, ", memo=", memo, "\n");
        if (to != get_self() && from != get_self())
            return;

        check(balance.amount > 0, "zero quantity, aborting");

        name account_type;
        if (memo.empty())
            account_type = def_account;
        else
            account_type = m_types.get(name(memo).value).type;
        
        if (to == get_self())
            updateBalance(to, account_type, balance);
        else if (from == get_self())
            updateBalance(from, account_type, -balance);
    }

    [[eosio::action]]
    void transfer(name from, name from_type, name to, name to_type, asset balance)
    {
        require_auth(from);

        check( balance.amount > 0, "attepmt to transfer negtive quantity, calling crypto cops..." );
        print("transfer: ", from, "[", from_type, "] -> ", to, "[", to_type, "] ", balance.amount, "\n");
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
                std::make_tuple(get_self(), to, balance, from_type.to_string())
            }.send();
        }
    }

    [[eosio::action]]
    void verify(name type, asset balance)
    {
        require_auth(get_self());

        auto it_c = m_types.find(type.value);
        check(it_c != m_types.end(), "account type not found");
        print("verifying balance: ", it_c->balance.amount, " == ", balance.amount);
        check(it_c->balance.amount == balance.amount, "wrong balance!");
    }

    [[eosio::action]]
    void executedef(bool force)
    {
        require_auth(get_self());
        
        auto it = m_txs.begin();
        check(it != m_txs.end(), "empty deffered transactions list");

        while (it != m_txs.end())
        {
            if (force || it->when <= now())
            {
                if (it->reccuring)
                {
                    uint32_t idx = it->idx;
                    it++;
                    processReccuring(idx, force);
                }
                else
                {
                    updateBalance(it->from, it->from_type, it->balance);
                    transfer(it->from, it->from_type, it->to, it->to_type, it->balance);
                    it = m_txs.erase(it);
                }
            }
            else
            {
                it++;
            }
        }
    }

    [[eosio::action]]
    void deffered(int idx,
                  name from, 
                  name from_type, 
                  name to, 
                  name to_type, 
                  asset balance, 
                  uint32_t when, 
                  bool reccuring, 
                  uint32_t till, 
                  uint32_t period)
    {
        require_auth(from);
        check( balance.amount > 0, "deffered: attepmt to transfer negtive quantity, calling crypto cops..." );
        check( when >= now(), "deffered transaction can't be in past");
        size_t count = std::count_if(m_txs.begin(), m_txs.end(), [](auto& x){return true;});
        check( count < s_trn_limit, "transactions buffer is full");
        
        updateBalance(from, from_type, -balance);

        check(m_txs.find(idx) == m_txs.end(), "transaction with such id already exists");

        m_txs.emplace(from, [&](auto& row){
            row.idx = idx;
            row.from = from;
            row.from_type = from_type;
            row.to = to;
            row.to_type = to_type;
            row.balance = balance;
            row.when = when;
            row.till = till;
            row.reccuring = reccuring;
            row.period = period;
        });
    }

    [[eosio::action]]
    void canceltrn(int idx)
    {
        require_auth(get_self());

        auto it = m_txs.find(idx);
        check(it != m_txs.end(), "no transaction with such id");
        updateBalance(it->from, it->from_type, it->balance);

        m_txs.erase(it);
    }

    [[eosio::action]]
    void verifydef(size_t number)
    {
        size_t count = std::count_if(m_txs.begin(), m_txs.end(), [](auto& x){return true;});
        check(number == count, "deffered transactions count doesn't match");
    }

    [[eosio::action]]
    void printdef()
    {
        print("deffered transfers:\n");
        for (auto it : m_txs)
        {
            print(it.from, "[", it.from_type, "] -> ", 
                  it.to, "[", it.to_type, "] ", 
                  it.balance.amount, " at ", 
                  microseconds_to_str(it.when * 1000000ull),
                  " reccuring=", (it.reccuring ? "true" : "false"),
                  "\n");
        }
        print("-------------------\n");
    }
};

int kata1::s_trn_limit = 10;
asset kata1::sys_zero = asset(0, symbol("SYS", 4));
name kata1::def_account("default");