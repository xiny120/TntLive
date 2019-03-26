import Vue from 'vue'
import App from './App'

import store from './store'

Vue.config.productionTip = false

Vue.prototype.$store = store

Vue.prototype.$serverUrl = 'http://192.168.18.101:8091';

App.mpType = 'app'


const app = new Vue({
	store,
    ...App
})
app.$mount()