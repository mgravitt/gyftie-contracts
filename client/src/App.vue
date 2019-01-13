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
            <v-list-tile-title>View Balance</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>
<!-- 
        <v-list-tile :to= "{ name: 'issue'}" @click="">
          <v-list-tile-action>
            <v-icon>flash_on</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Issue</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile> -->

        <v-list-tile :to= "{ name: 'transfer'}" @click="">
          <v-list-tile-action>
            <v-icon>send</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Transfer</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>

         <v-list-tile :to= "{ name: 'gyft'}" @click="">
          <v-list-tile-action>
            <v-icon>plus_one</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Gyft</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>

<!-- 
         <v-list-tile :to= "{ name: 'propose'}" @click="">
          <v-list-tile-action>
            <v-icon>edit</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Propose Calculation</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>

        <v-list-tile :to= "{ name: 'listproposals'}" @click="">
          <v-list-tile-action>
            <v-icon>list</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>List Proposals</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile>


        <v-list-tile :to= "{ name: 'vote'}" @click="">
          <v-list-tile-action>
            <v-icon>thumb_up_alt</v-icon>
          </v-list-tile-action>
          <v-list-tile-content>
            <v-list-tile-title>Vote</v-list-tile-title>
          </v-list-tile-content>
        </v-list-tile> -->


      </v-list>
    </v-navigation-drawer>
    <v-toolbar color="indigo" dark fixed app>
      <v-toolbar-side-icon @click.stop="drawer = !drawer"></v-toolbar-side-icon>
      <v-toolbar-title>Gyftie</v-toolbar-title>
    
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
