<template>
<div>
    <h2>Wallet</h2>

    <ul>
        <li v-for="balance in this.balances" :key="balance">{{balance.balance}} </li>
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
    host:'jungle.cryptolions.io',
    port:18888,
    protocol:'http',
    chainId:'038f4b0fc8ff18a4f0842a8f0564611f6e96e8535901dd45e43ac8691a1c4dca' 
  })

  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        mybalances: []
      }
    },

    mounted(){
      this.setEosInstance()

      ScatterJS.scatter.connect('SafeTokenToolkit').then(connected => {
        if(!connected){
          console.error('Could not connect to Scatter.')
          return
        }
        this.scatter = ScatterJS.scatter
      })
    
        rpc.get_table_rows({
            "json": true,
            "code": "aaastabletok",   // contract who owns the table
            "scope": "telegramuse1",  // scope of the table
            "table": "accounts",    // name of the table as specified by the contract abi
            "limit": 100,
        }).then( result => {
            console.log (result.rows);
            this.mybalances = result.rows;
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