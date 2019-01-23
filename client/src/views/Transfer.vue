<template>
   
<div>
    <h2>Transfer GFT</h2>
    <v-flex >
       <v-form v-model="valid">
          <v-text-field
            v-model="to"
            label="To"
            required
          ></v-text-field>
          
          <v-text-field
            v-model="quantity"
             :rules="[rules.required, rules.gft_asset_format]"
            label="GFT Amount"
            suffix="GFT"
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
  import { network_config, gyftiecontract } from '../config';

  ScatterJS.plugins( new ScatterEOS() )

  const network = Network.fromJson(network_config);
  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        to: "",
        quantity: "",
        memo: "",
        rules: {
          required: value => !!value || 'Required.',
          gft_asset_format: value => {
            const pattern = /^(([0-9]*)|(([0-9]*)\.([0-9]*)))$/
            return pattern.test(value) || 'GFT amount must be numeric. Do not include GFT symbol.'
          }
        }
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
              account: gyftiecontract, //'gyftietokens',
              name: 'transfer',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: this.to,
                quantity: Number.parseFloat(this.quantity).toFixed(8) + ' GFT',
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