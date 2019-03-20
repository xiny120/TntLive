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
			this.websockopened = false;
			this.wsuriidx = 0;
			//this.sign_up_status = -1;
			this.initWebSocket();
            console.log('App Launch')
        },
        onShow: function() {
            console.log('App Show')
        },
        onHide: function() {
            console.log('App Hide')
        },
		computed: mapState(['signupStatus']),
　　　　methods: { 
			...mapMutations(['register00','login']),
　　　　　　initWebSocket(){ //初始化weosocket 
				const wsuris = ["ws://localhost:8090/ws","ws://localhost:8091/ws","ws://localhost:8092/ws"]
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
