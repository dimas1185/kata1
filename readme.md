# Accounting Kata

## Phase 1
For this phase, we want to support a basic sub-category balances of a user's SYS token balance. For instance, one category could be "savings" and another could be "vacation". By default, all SYS tokens are categorized as a default category and this is the only category that should allow deposits and withdraws of tokens from outside parties.

This phase should be implemented with a focus on correctness and user experience. However, it is always beneficial to consider how the data model may need to change to support future enhancements and to keep track of the resources that the user will need in order to operate the contracts core feature set.

## Prerequisites
This kata assumes knowledge of setting up a basic single node test net and deploying contracts to it. If those concepts need refreshment refer to the Quick Start Guide

## User Stories
As a user, I can define the categories used to sub-divide my token balance.

## Acceptance Criteria 1
A user must be able to create new categories where each category:
is defined by a unique human readable string
is associated with an automatically generated numeric unique ID
A user must be able to retrieve a listing of existing categories
the listing must display both the ID and human readable string
There is no restriction placed on how the listing is assembled or displayed
An action that attempts to duplicate a category must fail objectively
As a user, I can re-categorize any quantity of SYS tokens in my account from one category to another.

## Acceptance Criteria 2
As a user i can issue an action that categorizes un-categorized or "default" tokens into a certain category bucket provided:
that quantity of SYS tokens exists in an uncategorized or default state on the account
As a user i can issue an action that re-categorizes tokens from one category to another provided:
the current category's balance is greater than or equal to the quantity of SYS tokens being re-categorized
A user must be able to retrieve a listing of existing category balances, including a default or un-categorized entry
the listing must display the human readable category string
There is no restriction placed on how the listing is assembled or displayed
As a user, I am protected from eosio.token::transfer actions which send more SYS tokens than my un-categorized or "default" balance

## Acceptance Criteria 3
A user must be able to successfully transfer a quantity of SYS tokens less than or equal their uncategorized balance
A user must not be able to successfully transfer a quantity of SYS tokens greater than their uncategorized balance
Notes
For the purposes of this kata, we can assume that the user (and owner of the contract) is capable of covering all resource costs.


# Demo:

compile:
```
$ eosio-cpp kata1.cpp -o kata1.wasm -abigen -I=../eosio.contracts/contracts/eosio.token/include
```

create account:
```
$ cleos create account han cats4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
$ cleos set account permission cats4 active --add-code
```

set contract:
```
$ cleos set contract cats4 $(pwd) --abi kata1.abi -p cats4@active
```

trunsfer money to cats4 account:
```
$ cleos push action eosio.token transfer '[ "alice", "cats4", "25.0000 SYS", "m" ]' -p alice@active
```

print balance:
```
$ cleos -v push action cats4 printbal '[ "" ]' -p cats4@active
```

add new category [Acceptance Criteria 1](#Acceptance-Criteria-1):
```
$ cleos -v push action cats4 addtype '[ "checking" ]' -p cats4@active
```

try to duplicate and there will be an error:
```
$ cleos -v push action cats4 addtype '[ "checking" ]' -p cats4@active
```

print categiries
```
$ cleos -v push action cats4 listtypes '[ "" ]' -p cats4@active
```

transfer between categories [Acceptance Criteria 2](#Acceptance-Criteria-2):
```
$ cleos -v push action cats4 transfer '[ "cats4", "default", "cats4", "checking", "5.0000 SYS", "m" ]' -p cats4@active
$ cleos -v push action cats4 transfer '[ "cats4", "checking", "cats4", "default", "1.0000 SYS", "m" ]' -p cats4@active
```

print balance:
```
$ cleos -v push action cats4 printbal '[ "" ]' -p cats4@active
```

transfer from default account [Acceptance Criteria 3](#Acceptance-Criteria-3):
```
$ cleos -v push action cats4 transfer '[ "cats4", "default", "alice", "", "5.0000 SYS", "m" ]' -p cats4@active
```

print balance:
```
$ cleos -v push action cats4 printbal '[ "" ]' -p cats4@active
```

print balance:
```
$ cleos -v push action cats4 printbal '[ "" ]' -p cats4@active
```

trying to overdraft:
```
$ cleos push action eosio.token transfer '[ "cats4", "alice", "20.0000 SYS", "m" ]' -p cats4@active
```