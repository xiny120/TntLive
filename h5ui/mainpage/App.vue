<script>
    import {
        mapState,
        mapMutations
    } from 'vuex'
	
    export default {
		data() {
			return {
				websock: null,
				wsuriidx:0,
				websockopened:false,
				
			};
		},		
		
		
		
        onLaunch: function() {
			
		uni.getStorage({//获得保存在本地的用户信息  
			key: 'userinfo',  
			success:(res) => {  
				this.login(res.data);  
				/*
				uni.request({// 再次校验并刷新token的有效时间  
					url: `${this.$serverUrl}/auth.php`,  
					header: {  
						"Content-Type": "application/x-www-form-urlencoded",  
						"Token":res.data.token  
					},  
					data: {  
						"username":res.data.user_name  
					},  
					method: "POST",  
					success: (e) => {  
						if (e.statusCode === 200 && e.data.code === 0) {  
							this.login(e.data);  
						}  
					}  
				}) 
				 */
			}  
		});  			
			
			/*
			try {
				//sessionid = uni.getStorageSync('sessionid');
				var userinfo = uni.getStorageSync('userinfo');
				if(userinfo){
					this.login(JSON.parse(userinfo));
				}
			} catch (e) {
				console.log(e);
				// error
			}				
			*/
			this.websockopened = false;
			this.wsuriidx = 0;
			//this.sign_up_status = -1;
			//this.initWebSocket();
            console.log('App Launch')
        },
        onShow: function() {
            console.log('App Show')
        },
        onHide: function() {
            console.log('App Hide')
        },
		computed: mapState(['signupStatus']),
		mounted() {
					
				},		
　　　　methods: { 
			...mapMutations(['register00','login','loginFail']),
	
			
　　　　　　initWebSocket(){ //初始化weosocket 
return;
				//const wsuris = ["ws://localhost:8090/ws","ws://localhost:8091/ws","ws://localhost:8092/ws"]
				const wsuris = ["ws://localhost:8091/ws"]
				this.wsuriidx = this.wsuriidx + 1;
				if(this.wsuriidx >= wsuris.length){
					this.wsuriidx = 0;
				}
	　　　　　　　const wsuri = wsuris[this.wsuriidx];//ws地址		
				console.log("initWebSocket " + wsuri);
	　　　　　　　this.websock = new WebSocket(wsuri); 
	　　　　　　　this.websock.onopen = this.websocketonopen;
	　　　　　　　this.websock.onerror = this.websocketonerror;
	　　　　　　　this.websock.onmessage = this.websocketonmessage; 
	　　　　　　　this.websock.onclose = this.websocketclose;
　　　　   }, 

　　　　　　websocketonopen() {
				this.websockopened = true;
　　　　　　　　console.log("WebSocket连接成功");
　　　　　　},
　　　　　　websocketonerror(e) { //错误
 　　　　　　 console.log("WebSocket连接发生错误");
			this.websockopened = false;
　　　　　　},
　　　　　　websocketonmessage(e){ //数据接收 
　　　　　　　　const redata = JSON.parse(e.data);
　　　　　　　　console.log(redata); 
				switch(redata.t){
					case "sign up":
						//getApp().signupStatus = redata.status;
						//this.login("hello");
						this.register00(redata.status);
						
					break;
					case "sign in":
						if(redata.userinfo.UserId != 0){
							this.login(redata.userinfo)
						}else{
							this.loginFail();
						}
						
					break;
					case "sessionid":
						var sessionid;
						var userinfo;
						try {
							sessionid = uni.getStorageSync('sessionid');
							userinfo = uni.getStorageSync('userinfo');
							if(sessionid && userinfo){
								var checkin ={
									t:'checkin',
									sessionid:sessionid,
									userinfo:JSON.parse( userinfo)
								}
								websocketsend(JSON.stringify(checkin))
								return;
							}
						} catch (e) {
							// error
						}					
						try {
							uni.setStorageSync('sessionid', redata.sessionid);
						} catch (e) {
							// error
						}						
					break;
				}
　　　　　　}, 

　　　　　　websocketsend(agentData){//数据发送 
				if(this.websockopened == true){
					this.websock.send(agentData);
					return true;
				}
				return false;
　　　　　　}, 

　　　　　 websocketclose(e){ //关闭 
				this.websockopened = false; 
　　　　　　　　console.log("connection closed (" + e.code + ")");
				uni.showToast({
					icon: 'none',
					title: '网络连接断开，准备重连...'
				});
				setTimeout(function(){
					getApp().initWebSocket();
				},8000);
　　　　　},
　　　}, 		
		
    }
</script>

<style>
    @import './common/common.css';

    page,
    view {
        display: flex;
    }

    page {
        display: flex;
        min-height: 100%;
        background-color: #EFEFEF;
    }

    template {
        display: flex;
        flex: 1;
    }
</style>
