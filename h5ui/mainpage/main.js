import Vue from 'vue'
import App from './App'

import store from './store'

Vue.config.productionTip = false

Vue.prototype.$store = store
Vue.prototype.$baseUrl = 'gpk01.gwgz.com:8091';
Vue.prototype.$serverUrl = 'http://'+Vue.prototype.$baseUrl;
Vue.prototype.$wssUrl = 'ws://'+Vue.prototype.$baseUrl;

App.mpType = 'app'


const app = new Vue({
	store,
    ...App
})
app.$mount()