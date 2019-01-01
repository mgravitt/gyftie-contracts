<template>
   
<div>
    <h2>Transfer Tokens</h2>
    <v-flex >
       <v-form v-model="valid">
          <v-text-field
            v-model="to"
            label="To"
            required
          ></v-text-field>
          
          <v-text-field
            v-model="quantity"
            label="Quantity"
            required
          ></v-text-field>

          
          <v-text-field
            v-model="memo"
            label="Memo"
            required
          ></v-text-field>
       </v-form>
    </v-flex>
    
    <br/>
    <br/>
     <v-btn color="primary" @click.prevent="sendTokens">Send Tokens</v-btn>
      <v-btn @click="login" v-if="scatter && !account">Login with Scatter</v-btn>
     <section v-if="scatter && account">
          Logged in with: {{account.name}}
          <v-btn @click="logout">Logout</v-btn>
     </section>
      <section v-if="result">
            <br>
            {{result}}
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
    host:'jungle2.cryptolions.io',
    port:80,
    protocol:'http',
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
        to: "gyftieuser11",
        quantity: "100.00 GFT",
        memo: "issue of GFT tokens"
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
      async sendTokens () {

        //await alert('Your data: ' + JSON.stringify(this.user))
        if(this.sending) return
        this.sending = true
        const options = { authorization:[`${this.account.name}@${this.account.authority}`] }

        const completed = res => {
          this.result = res
          this.sending = false
        }

        try {
          const result = await eos.transact({
            actions: [{
              account: 'gyftietoken1',
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: this.to,
                quantity: this.quantity,
                memo: this.memo
              },
            }]
          }, {
              blocksBehind: 3,
              expireSeconds: 30,
          })
          completed(result)
        } catch (e) {
          completed(e)
        }
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