<script>
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
			this.initWebSocket();
            console.log('App Launch')
        },
        onShow: function() {
            console.log('App Show')
        },
        onHide: function() {
            console.log('App Hide')
        },
　　　　methods: { 
　　　　　　initWebSocket(){ //初始化weosocket 
				const wsuris = ["ws://localhost:8080/ws","ws://localhost:8081/ws","ws://localhost:8082/ws"]
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
　　　　　　　　　//注意：长连接我们是后台直接1秒推送一条数据， 
          //但是点击某个列表时，会发送给后台一个标识，后台根据此标识返回相对应的数据，
      //这个时候数据就只能从一个出口出，所以让后台加了一个键，例如键为1时，是每隔1秒推送的数据，为2时是发送标识后再推送的数据，以作区分
　　　　　　　　console.log(redata.value); 
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
				setTimeout(function(){
					getApp().initWebSocket();
				},2000);
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
