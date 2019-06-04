import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

const store = new Vuex.Store({
    state: {
		signupStatus:-1,
        forcedLogin: false,
        hasLogin: -1,
        userName: "",
		userInfo:{},
		roomid:0,
    },
    mutations: {
		getroomid(state){
			try {
				const value = uni.getStorageSync('roomid');
				if (value) {
					state.roomid = value;
				}
			} catch (e) {
			}				
		},
		setroomid(state,roomid_){
			state.roomid = roomid_;
			uni.setStorage({key: 'roomid', data: roomid_});	
		},
        login(state, ui) {
			state.userInfo = ui;
            state.hasLogin = 1;
			uni.setStorage({key: 'userinfo', data: ui});			
        },
        logout(state,surl) {
			const data = {
				action:"authout",
			}
			uni.request({
				url: surl + '/api/1.00/private', //仅为示例，并非真实接口地址。
				method: 'POST',
				data:data,
				dataType:'json',  
				header:{  
					'content-type':'application/json',
					'mster-token':state.userInfo.SessionId,
				}, 
				success: (res) => {
				},

			});		
            state.userInfo = {};
            state.hasLogin = -1;
            uni.removeStorage({  
                key: 'userinfo'  
            });											
											
        },
		loginFail(state){
			state.userInfo = {};
			state.hasLogin = 0;
		},
		register00(state, signupStatus){
			state.signupStatus = signupStatus || -1;
		}
    }
})

export default store
