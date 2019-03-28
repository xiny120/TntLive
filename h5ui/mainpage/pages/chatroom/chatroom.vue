<template>
	<view >
		<view id="content" style="display: flex;flex-direction: column;">
			<view style="display: flex;flex-direction: column;padding: 8upx;" v-for="(message,index) in messages" :key="index" :message="message" :id="index">
				<view style="display: flex;flex-direction: row;">
					<view style="margin: 8upx;">
						<image style="height: 60upx; width: 60upx;" src="https://img-cdn-qiniu.dcloud.net.cn/uniapp/images/uni@2x.png"></image>					
					</view>
					<text style="margin:4upx;color:#A0A0A0;">{{message.user}}</text>
					<text style="margin:4upx;color:#C8C8C8;">{{message.datetime}}</text>
						
				</view>
				<view>{{message.content}}</view>
			</view>
			<view id="bottom" :style="{height:style.contentViewHeight+'px'}"></view>
		</view>
		<view class="foot">
			<chat-input @send-message="getInputMessage" @focus="inputGetFocus"></chat-input>
		</view>
	</view>
</template>



<script>
	import chatInput from '../../components/im-chat/chatinput.vue';
	import messageShow from '../../components/im-chat/messageshow.vue';

	export default {
		data() {
			return {
				title:"评论界面",
				name:'xcecd@qq.com',
				style: {
					pageHeight: 0,
					contentViewHeight: 0,
					footViewHeight: 90,
					mitemHeight: 0,
				},
				scrollTop: 0,
				messages: [{
					user: 'home',
					type: 'head', //input,content 
					content: '你好!',
					datetime: '2019-2-25',
				}]
			}
		},
		components: {
			chatInput,
			messageShow
		},
		onLoad: function () {
		
		},
		onReady:function(){
			const res = uni.getSystemInfoSync();
			let view = uni.createSelectorQuery().select(".foot");
			let that = this;
			view.boundingClientRect(data => {
				that.style.contentViewHeight = data.height;
			}).exec();				
		},
		methods: {
			getInputMessage: function (message) { //获取子组件的输入数据
				// console.log(message);
				this.addMessage('我是大魔王', message.content, false);
				this.toRobot(message.content);
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
			},
			scrollToBottom: function () {
				let view = uni.createSelectorQuery().select("#bottom");
				let that = this;
				view.boundingClientRect(data => {
					uni.pageScrollTo({
						scrollTop: data.bottom,
						duration: 100
					});						
				}).exec();					
			},
			toRobot: function (info) {

				// this.addMessage('home', info, false);
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
		margin-bottom: 100upx;
		flex-direction: row;
	}

	.foot {
		position: fixed;
		width: 100%;
		height: 90upx;
		min-height: 90upx;
		left: 0upx;
		bottom: 0upx;
		overflow: hidden;
	}
</style>
