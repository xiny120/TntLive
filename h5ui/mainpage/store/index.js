import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

const store1 = new Vuex.Store({
    state: {
		signupStatus:-1,
        /**
         * 是否需要强制登录
         */
        forcedLogin: false,
        hasLogin: false,
        userName: ""
    },
    mutations: {
        login(state, userName) {
            state.userName = userName || '新用户';
            state.hasLogin = true;
        },
        logout(state) {
            state.userName = "";
            state.hasLogin = false;
        },
		register00(state, signupStatus){
			state.signupStatus = signupStatus || -1;
		}
    }
})

export default store1
