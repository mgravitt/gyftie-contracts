<template>
   <v-app id="inspire">
    <v-navigation-drawer
      v-model="drawer"
      fixed
      app
    >
      <v-list >

         <v-list-tile :to= "{ name: 'viewbal'}" @click="">
          <v-list-tile-action>
            <v-icon>attach_money</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Balance</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>
<!-- 
  collaborate
  coin creation rate
  value
  70%, -->

        <v-list-tile :to= "{ name: 'transfer'}" @click="">
          <v-list-tile-action>
            <v-icon>send</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Transfer GFT</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>

         <v-list-tile :to= "{ name: 'gyft'}" @click="">
          <v-list-tile-action>
            <v-icon>person_add</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Gyft / Earn</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>

         <v-list-tile :to= "{ name: 'collaborate'}" @click="">
          <v-list-tile-action>
            <v-icon>group</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Collaborate</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile> 

        <v-list-tile :to= "{ name: 'buygft'}" @click="">
          <v-list-tile-action>
            <v-icon>local_grocery_store</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Buy GFT</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile> 

        <v-list-tile :to= "{ name: 'sellgft'}" @click="">
          <v-list-tile-action>
            <v-icon>local_offer</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Sell GFT</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile> 

      </v-list>
    </v-navigation-drawer>
    <v-toolbar color="indigo" dark fixed app>
      <v-toolbar-side-icon @click.stop="drawer = !drawer"></v-toolbar-side-icon>
      <v-toolbar-title></v-toolbar-title>
    
    </v-toolbar>
    <v-content>
      <v-container fluid fill-height>
        <v-layout>

          <router-view> </router-view>

        </v-layout>
      </v-container>
    </v-content>
    <v-footer color="indigo" app>
      <span class="white--text">&copy; 2018</span>
    </v-footer>
  </v-app>
</template>

<script>
import Landing from './components/Landing'

export default {
  name: 'App',
  data() { 
    return {
      drawer: true,
    };
  },
  components: {
    Landing
  },
  data () {
    return {
      //
    }
  }
}
</script>
