<template>
   
<div>
    <h2>Gyft Tokens</h2>
    <v-flex >
       <v-form v-model="valid">
          <v-text-field
            v-model="to"
            label="Gyftie Account Name"
            required
          ></v-text-field>
          
          <v-text-field
            v-model="idfield"
            label="Government Issued ID Number"
            required
          ></v-text-field>

       </v-form>
    </v-flex>
    
    <br/>
    <br/>
     <v-btn color="primary" @click.prevent="gyftTokens">Gyft Tokens</v-btn>
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

  async function sha256(message) {
    const msgUint8 = new TextEncoder('utf-8').encode(message);                      // encode as UTF-8
    const hashBuffer = await crypto.subtle.digest('SHA-256', msgUint8);             // hash the message
    const hashArray = Array.from(new Uint8Array(hashBuffer));                       // convert hash to byte array
    const hashHex = hashArray.map(b => ('00' + b.toString(16)).slice(-2)).join(''); // convert bytes to hex string
    return hashHex;
  }

  const network = Network.fromJson({
    blockchain:'eos',
    host:'eos.greymass.com',
    port:443,
    protocol:'https',
    chainId:'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906' 
  })

  // const network = Network.fromJson({
  //   blockchain:'eos',
  //   host:'jungle.eosio.cr',
  //   port:443,
  //   protocol:'https',
  //   chainId:'e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473' 
  // })

  let eos
  const rpc = new JsonRpc(network.fullhost())
  export default {
  
    data () {
      return {
        sending:false,
        scatter:null,
        result:null,
        to: "",
        idfield: ""
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
      async gyftTokens () {

       // await alert('Your data: ' + JSON.stringify(this.user))
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
              account: 'gyftietokens',
              name: 'calcgyft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: this.to
              },
            }
            ,{
              account: 'gyftietokens',
              name: 'gyft',
              authorization: [{
                actor: this.account.name,
                permission: this.account.authority,
              }],
              data: {
                from: this.account.name,
                to: this.to,
                idhash: await sha256(this.idfield)
              },
            }
            ]
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