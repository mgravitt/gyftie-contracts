cleos create account eosio gyftietoken EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio genesisgen EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio secondgen EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio issuer EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder1 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder2 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder3 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder4 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn
cleos create account eosio holder5 EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn

cleos push action eosio updateauth '{"account":"gyftietoken","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"gyftietoken","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p gyftietoken
cleos push action eosio updateauth '{"account":"genesisgen","permission":"active","parent":"owner","auth":{"keys":[{"key":"EOS7ckzf4BMgxjgNSYV22rtTXga8R9Z4XWVhYp8TBgnBi2cErJ2hn", "weight":1}],"threshold":1,"accounts":[{"permission":{"actor":"genesisgen","permission":"eosio.code"},"weight":1}],"waits":[]}}' -p genesisgen

cleos set contract gyftietoken gyftietoken/
cleos set contract genesisgen genesisgen/
cleos set contract secondgen secondgen/

cleos push action gyftietoken setconfig '["genesisgen"]' -p gyftietoken
cleos push action gyftietoken create '["issuer"]' -p gyftietoken
cleos push action gyftietoken issue '["holder1", "100.0000 GYFTIE", "memo"]' -p issuer

cleos push action gyftietoken calcgyft '["holder1", "holder2"]' -p holder1
cleos push action gyftietoken gyft '["holder1", "holder2", "idhash12345", "memo"]' -p holder1

cleos push action gyftietoken calcgyft '["holder2", "holder3"]' -p holder2
cleos push action gyftietoken gyft '["holder2", "holder3", "idhash54321", "memo"]' -p holder2

cleos push action gyftietoken propose '["holder1", "secondgen", "Second generation calculation"]' -p holder1

cleos push action gyftietoken vote '["holder1"]' -p holder1
cleos push action gyftietoken vote '["holder2"]' -p holder2

cleos push action gyftietoken calcgyft '["holder3", "holder4"]' -p holder3
cleos push action gyftietoken gyft '["holder3", "holder4", "idhash54321", "memo"]' -p holder3
