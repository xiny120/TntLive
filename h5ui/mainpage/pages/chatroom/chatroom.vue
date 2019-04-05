<template>
	<view style="margin-right: 2upx;" id="mainView">
		<view id="content" class="content" style="display: flex;flex-direction: column;margin-right: 8upx;">
			<view style="display: flex;flex-direction: column;padding: 8upx;margin-top: 4upx;" v-for="(message,index) in messages" :key="index" :message="message" :id="index">
				<view style="display: flex;flex-direction: row;">
					<view style="margin: 8upx;">
						<image style="height: 48upx; width: 48upx;" src="https://img-cdn-qiniu.dcloud.net.cn/uniapp/images/uni@2x.png"></image>					
					</view>
					<view >
						<text style="margin:4upx;color:#A0A0A0;">{{message.user}}</text>
						<text style="margin:4upx;color:#C8C8C8;margin-left:18upx;">{{message.datetime}}</text>
					</view>
				</view>
				<view><uParse :content="message.content" @preview="preview" @navigate="navigate" /></view>
				
			</view>
		</view>
		<view class="foot">
			<ly-markdown @send-message="getInputMessage" @focus="inputGetFocus"></ly-markdown>
		</view>
	</view>
</template>

<script>
	import lyMarkdown from '../../components/ly-markdown/ly-markdown.vue';
	import marked from '../../components/marked'
	import uParse from '../../components/uParse/src/wxParse.vue'
    import {
        mapState,
        mapMutations
    } from 'vuex'

	export default {
		data() {
			return {
				sessionid:"",
				websockautor:true,
				websockopened:false,
				websock:null,
				wsuriidx:0,
				socketMsgQueue: [],
				title:"评论界面",
				name:'xcecd@qq.com',
				style: {
					pageHeight: 0,
					contentViewHeight: 0,
					footViewHeight: 90,
					mitemHeight: 0,
				},
				//scrollTop: 0,
				messages: [{
					user: '系统小喇叭',
					type: 'head', //input,content 
					content: '唐能通短线是银 中国首个证券理论创始人 短线是银作者 中国最大盘中直播主讲人',
					datetime: '2019-2-25 09:09:09',
				}]
			}
		},
		components: {
			lyMarkdown,
			uParse,
		},
		onLoad: function () {
			console.log("onLoad................");
			
			
			
			this.initWebSocket();
		},
		onReady:function(){
		},
		computed: mapState(['userInfo']),
		methods: {
			preview(src, e) {
				// do something
				console.log("src: " + src);
			},
			navigate(href, e) {
				// 如允许点击超链接跳转，则应该打开一个新页面，并传入href，由新页面内嵌webview组件负责显示该链接内容
				console.log("href: " + href);
				uni.showModal({
					content : "点击链接为：" + href,
					showCancel:false
				})
			},		
			
			initWebSocket:function(){
				//const wsuris = ["ws://localhost:8090/ws","ws://localhost:8091/ws","ws://localhost:8092/ws"]
				const wsuris = [this.$wssUrl + "/ws/001"]
				
				
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
			websocketonmessage(res){ //数据接收 
					const ans = JSON.parse(res.data);
					switch(ans.t){
						case "toall":
							this.addMessage(ans.userInfo.UserName, ans.msg, false);
						break;
						case "checkin":
							console.log(ans);
							if(ans.status != 0){
								this.websockautor = false;
							}else{
								
							}
						break;
						case "sessionid":
							
							var userinfo;
							try {
								//sessionid = uni.getStorageSync('sessionid');
								//uni.setStorage({key: 'userinfo', data: ui});	
								userinfo = uni.getStorageSync('userinfo');
								if(userinfo){
									//let ui = JSON.parse(userinfo);
									var checkin ={
										t:'checkin',
										sessionid:userinfo.SessionId,
										userinfo:userinfo
									}
									this.websocketsend(JSON.stringify(checkin))
									return;
								}
							} catch (e) {
								console.log(e);
							}					
							try {
								
								//uni.setStorageSync('sessionid', redata.sessionid);
							} catch (e) {
								// error
							}		
							this.websockautor = false;
							this.websock.close();
							uni.showToast({
								icon: 'none',
								title: '请先登录哦！'
							});											
						break;						
					}			
			},
			websocketclose(e){ //关闭 
				this.websockopened = false;
				let that = this;
				console.log("connection closed (" + e.code + ")");
				
				if(this.websockautor){
					uni.showToast({
						icon: 'none',
						title: '网络连接断开，准备重连...'
					});
					setTimeout(function(){
						that.initWebSocket();
					},8000);
				}
			},			
			websocketsend:function(msg){

				if (this.websockopened) {
					this.websock.send(msg);
				} else {
					if(!this.websockautor){
						uni.showToast({
							icon: 'none',
							title: '请先登录哦！'
						});						
					}else{				
						this.socketMsgQueue.push(msg);
					}
				}				
			},
			getInputMessage: function (message) { //获取子组件的输入数据
				const data = {
					t:"toall",
					userInfo:{
						UserName:this.userInfo.UserName,
						UserUuid:this.userInfo.UserUuid,
						avatarUrl:this.userInfo.avatarUrl,
					},
					msg:message.content,
				}
				//this.addMessage(this.userInfo.UserName, message.content, false);
				//this.toRobot(message.content);
				this.websocketsend(JSON.stringify(data));
			},
			addMessage: function (user, content, hasSub, subcontent) {
				var that = this;
				that.messages.push({
					user: user,
					content: content,
					hasSub: hasSub,
					subcontent: subcontent,
					datetime: '2019-02-29',
				});
				
				setTimeout(function(){
					that.scrollToBottom();
				},1);					
				
			},
			scrollToBottom: function () {
				let mainView = uni.createSelectorQuery().select("#mainView");
				
				mainView.fields({
				  size: true,
				  scrollOffset: true
				}, data => {
				  console.log("得到节点信息" + JSON.stringify(data));
				  console.log("节点的宽为" + data.width);
				}).exec();				
				
				let view = uni.createSelectorQuery().select("#content");
				let that = this;
				view.boundingClientRect(data => {

					uni.pageScrollTo({
						scrollTop: data.height,
						duration: 1
					});						
				}).exec();			
			},
			toRobot: function (info) {
				var apiUrl = 'http://www.tuling123.com/openapi/api';
				uni.request({
					url: apiUrl,
					data: {
						"key": 'acfbca724ea1b5db96d2eef88ce677dc',
						"info": info,
						"userid": 'uni-test'
					},
					success: (res) => {
						console.log("s", res);
						this.addMessage('客服小花', res.data.text, false);
						let that = this;
						setTimeout(function(){
							that.scrollToBottom();
						},1);						
								
						console.log('request success:' + res.data.text);
					},
					fail: (err) => {
						console.log('request fail', err);
						uni.showModal({
							content: err.errMsg,
							showCancel: false
						})
					}
				});
			}
		}
	}
</script>

<style>
	.uni-column {
		display: flex;
		flex-direction: row;
	}

	.content {
		display: flex;
		flex: 1;
		margin-bottom: 240upx;
		flex-direction: row;
	}

	.foot {
		position: fixed;
		width: 100%;
		height: 240upx;
		min-height: 240upx;
		left: 0upx;
		bottom: 0upx;
		overflow: hidden;
	}
</style>
