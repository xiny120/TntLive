<template>
	
	<view style="display: flex;flex-direction: column;height: 180upx;min-height: 180upx; width: 100%;">
		<view class="footer1">
			<image  @click="emojiClick('dianzan')" mode="aspectFit" style="height: 64upx; width: 128upx;" src="../../static/dianzan.png"></image>
			<image  @click="emojiClick('dianzan02')" mode="aspectFit" style="height: 64upx; width: 128upx;" src="../../static/dianzan02.png"></image>
		</view>		
		<view class="footer">
			<view class="footer-left">
				<view class="uni-icon uni-icon-mic" @tap="startRecognize"> </view>
			</view>
			<view class="footer-center">	
	
				<view>
					<view>
						
						<view class="toolbar" v-if="showToolbar">
							<view class="iconfont icon-bold" @click="toolBarClick('bold')"></view>
							<view class="iconfont icon-italic" @click="toolBarClick('italic')"></view>
							<view class="iconfont icon-xiahuaxian1" @click="toolBarClick('header')"></view>
							<view class="iconfont icon-underline" @click="toolBarClick('underline')"></view>
							<view class="iconfont icon-strike" @click="toolBarClick('strike')"></view>
							<view class="iconfont icon-sup" @click="toolBarClick('sup')"></view>
							<view class="iconfont icon-sub" @click="toolBarClick('sub')"></view>
							<view class="iconfont icon-alignleft" @click="toolBarClick('alignleft')"></view>
							<view class="iconfont icon-aligncenter" @click="toolBarClick('aligncenter')"></view>
							<view class="iconfont icon-alignright" @click="toolBarClick('alignright')"></view>
							<view class="iconfont icon-link" @click="toolBarClick('link')"></view>
							<view class="iconfont icon-image" @click="toolBarClick('imgage')"></view>
							<view class="iconfont icon-code" @click="toolBarClick('code')"></view>
							<view class="iconfont icon-table" @click="toolBarClick('table')"></view>
							<view class="iconfont icon-qingkong" @click="toolBarClick('clear')"></view>
						</view>
						
						<view class="input-content">
							<textarea fixed="true" maxlength="-1" v-model="textareaDataSync" @blur="getCursor"></textarea>
						</view>
					</view>
					<view class="preview" v-if="showPreview && textareaHtmlSync">
						<scroll-view scroll-y :style="'height:'+screenHeight/2.5+'px;padding:10px;box-sizing: border-box;'">
							<uParse :content="textareaHtmlSync" @preview="preview" @navigate="navigate" />
						</scroll-view>
					</view>
				</view>
			</view>
			<view class="footer-right" @tap="sendMessge">
				<view id='msg-type' >发送</view>
			</view>
		</view>

	</view>				
</template>

<script>
	import marked from '../marked'
	import uParse from '../uParse/src/wxParse.vue'
	export default {
		name: "ly-markdown",
		components: {
			uParse
		},
		data: function () {
			return {
				screenHeight: 0,
				cursor: 0,
                textareaDataSync:this.textareaData,
                textareaHtmlSync:this.textareaHtml
			}
		},
		props: {
			textareaData: {
				type: String,
				default: ""
			},
			textareaHtml: {
				type: String,
				default: ""
			},
			showPreview: {
				type: Boolean,
				default: false
			},
			showToolbar:{
				type:Boolean,
				default:false
			}
		},
		methods: {
			startRecognize: function () {
				var options = {};
				var that = this;
				options.engine = 'iFly';
				that.textareaDataSync = "";
				plus.speech.startRecognize(options, function (s) {
					console.log(s);
					that.textareaDataSync += s;
				}, function (e) {
					console.log("语音识别失败：" + e.message);
				});
			},
			sendMessge: function () {
				var that = this;
				if (that.textareaDataSync.trim() == '') {

					that.textareaDataSync = '';
				} else {

					//点击发送按钮时，通知父组件用户输入的内容
					this.$emit('send-message', {
						type: 'text',
						content: that.textareaDataSync
					});
					that.textareaDataSync = '';
					
				}
			},
			preview(src, e) {
				uni.previewImage({
					urls: src,
				})
			},
			navigate(href, e) {
				// 如允许点击超链接跳转，则应该打开一个新页面，并传入href，由新页面内嵌webview组件负责显示该链接内容
				// #ifdef APP-PLUS
				plus.runtime.openURL(href)
				// #endif
				// #ifdef MP-WEIXIN
				uni.setClipboardData({
					data: href,
					success: function () {
						uni.showModal({
							content: "网址已复制,请在浏览器中粘贴打开",
							showCancel: false
						})
					}
				})
				// #endif
			},
			emojiClick(obj){
				var uri = "";
				if(obj == 'dianzan'){
					uri = "http://www.gwgz.com/images_man/dianzan@48.png";
				}else if(obj == 'dianzan02'){
					uri = "http://www.gwgz.com/images_man/dianzan02@48.png";
				}
				
				//点击发送按钮时，通知父组件用户输入的内容
				this.$emit('send-message', {
					type: 'text',
					content: "<img src='" + uri +"'>"
				});				
				
			},
			toolBarClick(type) {
				if (type == 'bold') {
					this.textareaDataSync += "**粗体文字** "
				} else if (type == "italic") {
					this.textareaDataSync += "*斜体* "
				} else if (type == "header") {
					uni.showActionSheet({
						itemList: ["标题1", "标题2", "标题3", "标题4", "标题5", "标题6"],
						success: res => {
							switch (res.tapIndex) {
								case 0:
									this.textareaDataSync += "# 标题1\r";
									return;
								case 1:
									this.textareaDataSync += "## 标题2\r";
									return;
								case 2:
									this.textareaDataSync += "### 标题3\r";
									return;
								case 3:
									this.textareaDataSync += "#### 标题4\r";
									return;
								case 4:
									this.textareaDataSync += "##### 标题5\r";
									return;
								case 5:
									this.textareaDataSync += "###### 标题6\r";
									return;
							}
						}
					})
				} else if (type == "underline") {
					this.textareaDataSync += "++下划线++ "
				} else if (type == "strike") {
					this.textareaDataSync += "~~中划线~~ "
				} else if (type == "sup") {
					this.textareaDataSync += "^上角标^ "
				} else if (type == "sub") {
					this.textareaDataSync += "~下角标~ "
				} else if (type == "alignleft") {
					this.textareaDataSync += "\n::: hljs-left\n\n左对齐\n\n:::\n"
				} else if (type == "aligncenter") {
					this.textareaDataSync += "\n::: hljs-center\n\n居中对齐\n\n:::\n"
				} else if (type == "alignright") {
					this.textareaDataSync += "\n::: hljs-right\n\n\n\n右对齐\n\n:::\n"
				} else if (type == "link") {
					this.textareaDataSync += "[在此输入网址描述](在此输入网址) "
				} else if (type == "imgage") {
					this.textareaDataSync += "![](在此输入图片地址) "
				} else if (type == "code") {
					this.textareaDataSync += "\n``` 代码块 \n\n```\n"
				} else if (type == "table") {
					this.textareaDataSync += "\n|列1|列2|列3|\n|-|-|-|\n|单元格1|单元格2|单元格3|\n"
				} else if (type == "clear") {
					uni.showModal({
						title: "提示",
						content: "确定清空?",
						success: res => {
							if (res.confirm) {
								this.textareaDataSync = "";
							}
						}
					})
				}
			},
			getCursor(e) {
				//安卓失去焦点获取不到cursor位置,暂不使用
				//this.cursor = e.detail.cursor; 
			}
		},
		watch: {
			"textareaDataSync": function (newValue, oldValue) {
				this.textareaHtmlSync = marked(newValue)
				this.$emit('update:textareaData', newValue)
				this.$emit('update:textareaHtml', this.textareaHtmlSync)
			}
		},
		mounted: function () {
			uni.getSystemInfo({
				success: res => {
					this.screenHeight = res.screenHeight
				}
			})
		}
	}
</script>

<style>
	@import './markdown.css';
	@import url("../uParse/src/wxParse.css");
	
	.footer1 {
		display: flex;
		flex-direction: row;
		align-content:flex-start;
		width: 100%;
		height: 60upx;
		min-height: 60upx;
		border-top: solid 1px #bbb;
		overflow: hidden;
		padding: 5upx;
		background-color: #fafafa;
	}	
	.footer {
		display: flex;
		flex-direction: row;
		width: 100%;
		height: 180upx;
		min-height: 180upx;
		
		overflow: hidden;
		padding: 5upx;
		background-color: #fafafa;
	}
	.footer-left {

		width: 80upx;

		display: flex;
		justify-content: center;
		align-items: center;
	}
	.footer-right {
		width: 120upx;
		display: flex;
		justify-content: center;
		align-items: center;
		color: #1482D1;
	}
	.footer-center {
		flex: 1;
		display: flex;
		justify-content: center;
		align-items: center;
	}
	.footer-center .input-text {
		flex: 1;
		background: #fff;
		border: solid 1upx #ddd;
		padding: 10upx !important;
		font-family: verdana !important;
		overflow: hidden;
		border-radius: 15upx;
	}	

	.input-content {
		width: 100%;
	}

	.input-content textarea {
		font-size: 30upx;
		height: 100%;
		min-height: 120upx;
		border: solid 1px #bbb;
		margin-bottom: 8upx;
		
	}

	.preview {
		border-top: 1upx solid #e0e0e0;
		width: 100%;
	}

	.toolbar {
		width: 100%;
		border: none;
		box-shadow: 0 0upx 4upx rgba(0, 0, 0, 0.157), 0 0upx 4upx rgba(0, 0, 0, 0.227);
	}

	.toolbar .iconfont {
		display: inline-block;
		cursor: pointer;
		height: 61.6upx;
		width: 61.6upx;
		margin: 13upx 0 11upx 0upx;
		font-size: 33upx;
		padding: 10upx 13upx 11upx 8upx;
		color: #757575;
		border-radius: 11upx;
		text-align: center;
		background: none;
		border: none;
		outline: none;
		line-height: 2.2;
		vertical-align: middle;
	}

	.input-content {
		
	}
</style>
