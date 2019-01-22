<template>
   
<div>
    <h2>Propose Monetary Policy Change</h2>
    <v-flex >
                      
          <v-text-field
            v-model="explanation"
            label="Proposal Explanation"
            required
          ></v-text-field>
       </v-form>
    </v-flex>
    
    <br/>
    <br/>
     <v-btn color="primary" @click.prevent="propose">Propose</v-btn>
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

  let eos
  const rpc = new JsonRpc(network_config)
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        proposer: "",
        //tokengen: "gygenesisgen",
        explanation: "back to genesis calculation"
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
      async propose () {
        //console.log (this.tokengen);
        console.log (this.explanation);
        console.log (this.account.name);
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
              account: gyftiecontract,
              name: 'propose',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                proposer: this.account.name,
                //token_gen: this.tokengen,
                notes: this.explanation
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