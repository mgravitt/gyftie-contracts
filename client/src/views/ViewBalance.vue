<template>
<div>
    <h2>Balances</h2>

    <h3>gyftieuser11</h3>
    <ul>
        <li v-for="balance in this.gyfuser11bal" :key="balance">{{balance.balance}} </li>
    </ul>

    <h3>gyftieuser12</h3>
    <ul>
        <li v-for="balance in this.gyfuser12bal" :key="balance">{{balance.balance}} </li>
    </ul>

    <h3>gyftieuser13</h3>
    <ul>
        <li v-for="balance in this.gyfuser13bal" :key="balance">{{balance.balance}} </li>
    </ul>

    <br/><br/>

     <v-btn @click="login" v-if="scatter && !account">Login with Scatter</v-btn>
     <section v-if="scatter && account">
          Logged in with: {{account.name}}
          <v-btn @click="logout">Logout</v-btn>
     </section>
</div>
</template>

<script>
  import ScatterJS, {Network} from 'scatterjs-core'
  import ScatterEOS from 'scatterjs-plugin-eosjs2'
  import { Api, JsonRpc, RpcError, JsSignatureProvider } from 'eosjs'

  ScatterJS.plugins( new ScatterEOS() )

  const network = Network.fromJson({
    blockchain:'eos',
    host:'jungle.eosio.cr',
    port:443,
    protocol:'https',
    chainId:'e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473' 
  })

  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        gyfuser11bal: [],
        gyfuser12bal: [],
        gyfuser13bal: []
      }
    },

    mounted(){
      this.setEosInstance()

      ScatterJS.scatter.connect('Gyftie').then(connected => {
        if(!connected){
          console.error('Could not connect to Scatter.')
          return
        }
        this.scatter = ScatterJS.scatter
      })
    
        rpc.get_table_rows({
            "json": true,
            "code": "gyftietoken1",   // contract who owns the table
            "scope": "gyftieuser11",//this.scatter.identity.accounts[0],  // scope of the table
            "table": "accounts",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.gyfuser11bal = result.rows;
        });

        rpc.get_table_rows({
            "json": true,
            "code": "gyftietoken1",   // contract who owns the table
            "scope": "gyftieuser12",//this.scatter.identity.accounts[0],  // scope of the table
            "table": "accounts",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.gyfuser12bal = result.rows;
        });

        rpc.get_table_rows({
            "json": true,
            "code": "gyftietoken1",   // contract who owns the table
            "scope": "gyftieuser13",//this.scatter.identity.accounts[0],  // scope of the table
            "table": "accounts",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.gyfuser13bal = result.rows;
        });
    },

    computed:{
      account(){
        if(!this.scatter || !this.scatter.identity) return null
        return this.scatter.identity.accounts[0]
      },
    },


    methods: {
      login(){
        this.scatter.getIdentity({accounts:[network]})
      },
      logout(){
        this.scatter.forgetIdentity()
      },
     
    setEosInstance(){
      if(this.account){
        eos = this.scatter.eos(network, Api, {rpc})
      } else {
        eos = new Api({ rpc })
      }
    },
  },
  watch:{
    ['account'](){
      this.setEosInstance()
    }
  }
}
</script>

<style lang="css">

</style>