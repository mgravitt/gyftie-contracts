Private key: 5KSQeFjYx7D5s2iFvJNhYkwskJFeKxHsBTLwGQWE1UTp1push6X
Public key: EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj

cleos create account eosio gyftietoken  EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio genesisgen EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio secondgen EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio issuer EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder1 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder2 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder3 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder4 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj
cleos create account eosio holder5 EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj

cleos push action eosio updateauth '{"account":"gyftietoken","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"gyftietoken","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p gyftietoken
cleos push action eosio updateauth '{"account":"genesisgen","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS85AboyypCNfuENiDo8986J3tGiXMQ46m1eTimB1GDzKvVHNqWj", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"genesisgen","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p genesisgen

cleos set contract gyftietoken gyftietoken/
cleos set contract genesisgen genesisgen/
cleos set contract secondgen secondgen/

cleos push action genesisgen setconfig '["0.10000000 GFT", 1.500000000000]' -p genesisgen
cleos push action gyftietoken setconfig '["genesisgen"]' -p gyftietoken
cleos push action gyftietoken create '[]' -p gyftietoken
cleos push action gyftietoken issue '["holder1", "15.00000000 GFT", "memo"]' -p gyftietoken

cleos push action gyftietoken transfer '["holder1", "holder3", "1.00000000 GFT", "should fail"]' -p holder1

cleos push action gyftietoken calcgyft '["holder1", "holder2"]' -p holder1
cleos push action gyftietoken gyft '["holder1", "holder2", "idhash12345"]' -p holder1

cleos push action gyftietoken transfer '["holder1", "holder2", "1.00000000 GFT", "should succeed"]' -p holder1

cleos push action gyftietoken calcgyft '["holder2", "holder3"]' -p holder2
cleos push action gyftietoken gyft '["holder2", "holder3", "idhash54321", "memo"]' -p holder2

cleos push action gyftietoken propose '["holder1", "secondgen", "Second generation calculation"]' -p holder1

cleos push action gyftietoken vote '["holder1"]' -p holder1
cleos push action gyftietoken vote '["holder2"]' -p holder2

cleos push action gyftietoken calcgyft '["holder3", "holder4"]' -p holder3
cleos push action gyftietoken gyft '["holder3", "holder4", "idhash54321", "memo"]' -p holder3
