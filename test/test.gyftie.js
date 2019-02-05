let Gyftietoken = require ('../../eos-contract-wrapper/contracts/gyftietoken');
let Gftorderbook = require ('../../eos-contract-wrapper/contracts/gftorderbook');
let Eosiotoken = require ('../../eos-contract-wrapper/contracts/eosio.token');
let Genesisgen = require ('../../eos-contract-wrapper/contracts/genesisgen');
const { assert } = require('chai')

let EosService = require("eos-service");
let Config = require("./config");
console.log (Config);
let eosService = new EosService(Config.NET_CONFIG);

let gyftietoken, gftorderbook, eosiotoken, holder1orders, holder2orders, holder1gyft, holder2gyft = {};

async function killgft (user) {
    let table=await eosService.readTable({code:"gyftietoken",scope:user,table:"accounts"});
    if (table.rows.length > 0) {
        await gyftietoken.burnall (user);
    }
}

async function getgftbalance (user) {
    let table=await eosService.readTable({code:"gyftietoken",scope:user,table:"accounts"});
    return await eosService.assetToAmount(table.rows[0].balance);
}


before (async () => {

    gyftietoken = new Gyftietoken ("gyftietoken", "active");
    gftorderbook = new Gftorderbook ("gftorderbook", "active");
    eosiotoken = new Eosiotoken ("eosio.token", "active");
    holder1eos = new Eosiotoken ("holder1", "active");
    holder1orders = new Gftorderbook ("holder1", "active");
    holder2orders = new Gftorderbook ("holder2", "active");
    holder1gyft = new Gyftietoken ("holder1", "active");
    holder2gyft = new Gyftietoken ("holder2", "active");

    genesisgen = new Genesisgen ("genesisgen", "active");

    let eosstat = await eosService.readTable({code:"eosio.token",scope:"EOS",table:"stat"});
    if (eosstat.rows.length == 0) {
        await eosiotoken.create ("eosio.token", "1000000000.0000 EOS");
    }

    await killgft ("gyftietoken");
    await killgft ("holder1");
    await killgft ("holder2");

    await gyftietoken.reset ();
    await gyftietoken.delconfig();
    await genesisgen.reset();
    await gftorderbook.removeorders();

    await genesisgen.setconfig ("0.1000000 GFT", "1.5000000000000");
    await gyftietoken.setconfig ("genesisgen", "gftorderbook", "gyftiegyftie");

    await gyftietoken.setcounter ("0");
    await gftorderbook.setconfig ("gyftietoken", "eosio.token", "EOS", "4");

    let gyftiebal = await eosService.readTable({code:"gyftietoken",scope:"GFT",table:"stat"});
    if (gyftiebal.rows.length == 0) {
        await gyftietoken.create ();
        await gyftietoken.issue ("gyftietoken", "5.00000000 GFT", "memo");
    }
    
    await eosiotoken.issue ("holder1", "10000.0000 EOS", "memo");
    await holder1eos.transfer ("holder1", "gftorderbook", "5000.0000 EOS", "memo");
  
});

describe('Test Gyftie', async () => {

    it('Gyft can occur', async () => {

        console.log ("IN first test");
        let table=await eosService.readTable({code:"gyftietoken",scope:"gyftietoken",table:"accounts"});
        console.log (table);
        console.log (table.rows[0].balance);
        console.log(await eosService.assetToAmount(table.rows[0].balance));

        let orig_balance = await eosService.assetToAmount(table.rows[0].balance);
        console.log ("Original balance: ", orig_balance);

        await gyftietoken.calcgyft ("gyftietoken", "holder1");
        await gyftietoken.gyft ("gyftietoken", "holder1", "idhash12345");
       
        table=await eosService.readTable({code:"gyftietoken",scope:"holder1",table:"accounts"});
        let gyftee_balance = await eosService.assetToAmount(table.rows[0].balance);
        console.log ("Gyftee balance: ", gyftee_balance);
        assert.equal (gyftee_balance, orig_balance);

        table=await eosService.readTable({code:"gyftietoken",scope:"gyftietoken",table:"accounts"});
        assert.equal (eosService.assetToAmount(table.rows[0].balance), orig_balance + (orig_balance * 1.1));

    });


    it ('Can place a buy order', async() => {
        await holder1orders.limitbuygft ("holder1", "6.2000 EOS", "0.20000000 GFT");

        let table=await eosService.readTable({code:"gftorderbook",scope:"gftorderbook",table:"buyorders"});
        console.log ("Buy order table   :", table);
        assert.equal (await eosService.assetToAmount (table.rows[0].price_per_gft), 62000);
    });


    it ('Second account can perform a gyft', async() => {
        let orig_balance = await getgftbalance ("holder1");

        await holder1gyft.calcgyft ("holder1", "holder2");
        await holder1gyft.gyft ("holder1", "holder2", "ABC");

        let gyftee_balance = await getgftbalance ("holder2");
        assert.equal (gyftee_balance, orig_balance);
        
        assert.equal (await getgftbalance("holder1"), orig_balance + (orig_balance * 1.1));
    });

    it ('Place a market order', async() => {
        let holder1_orig_balance = await getgftbalance ("holder1");
        console.log ("Holder 1 original: ", holder1_orig_balance);

        let holder2_orig_balance = await getgftbalance ("holder2");

        await holder2gyft.transfer ("holder2", "gftorderbook", "2.50000000 GFT", "memo");
        await holder2orders.marketsell ("holder2", "1.50000000 GFT");

        console.log ("Increased: ", holder1_orig_balance + await eosService.assetToAmount("0.20000000 GFT"));
        //console.log (holder2_orig_balance - await eosService.assetToAmount("1.50000000 GFT"));

        assert.equal (holder1_orig_balance + await eosService.assetToAmount("0.20000000 GFT"), await getgftbalance("holder1"));
        // assert.equal (holder2_orig_balance - await eosService.assetToAmount("1.50000000 GFT"), getgftbalance ("holder2"));

        let table=await eosService.readTable({code:"gftorderbook",scope:"gftorderbook",table:"buyorders"});
        assert.isUndefined (table.rows[0]);
        
    });





});