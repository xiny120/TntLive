import Vue from 'vue'
import App from './App'


import store from './store'

Vue.config.productionTip = false

Vue.prototype.$store = store
Vue.prototype.$baseUrl = 'gpk01.gwgz.com:8092';
Vue.prototype.$serverUrl = 'http://'+Vue.prototype.$baseUrl;
Vue.prototype.$wssUrl = 'ws://'+Vue.prototype.$baseUrl;

App.mpType = 'app'


const app = new Vue({
	store,
    ...App
})
app.$mount()


var jweixin = require('jweixin-module');
//获取微信公众号的配置
var jweixin_url = Vue.prototype.$serverUrl + "/api/1.00/public"
var link = location.href;
uni.request({
	url: jweixin_url,
	method: 'POST',
	dataType:'json',
	data: {
		action:"wxjsinit",
		url: link,
	},
	header:{  
		'content-type':'application/json',
	}, 		
	success: res => {
		var s = JSON.parse(res.data);
		console.log(s.data);
		
		jweixin.config({
			debug: false, // 开启调试模式,调用的所有api的返回值会在客户端alert出来，若要查看传入的参数，可以在pc端打开，参数信息会通过log打出，仅在pc端时才会打印。
			appId: s.data.appId, // 必填，公众号的唯一标识
			timestamp: s.data.timestamp, // 必填，生成签名的时间戳
			nonceStr: s.data.nonceStr, // 必填，生成签名的随机串
			signature: s.data.signature.toLowerCase(), // 必填，签名，见附录1
			jsApiList: ["onMenuShareTimeline", "onMenuShareAppMessage","onMenuShareQQ","onMenuShareWeibo","onMenuShareQZone"]
		});	
		
		jweixin.ready(function () {
			//获取“分享到QQ”按钮点击状态及自定义分享内容接口（即将废弃）
			jweixin.onMenuShareQQ({
				title: title, // 分享标题
				desc: desc, // 分享描述
				link: url, // 分享链接
				imgUrl: imgurl, // 分享图标
				success: function () {
				 // 用户确认分享后执行的回调函数
				},
				cancel: function () {
				 // 用户取消分享后执行的回调函数
				}
			});
		
			//获取“分享给朋友”按钮点击状态及自定义分享内容接口（即将废弃）
			jweixin.onMenuShareAppMessage({
				title: title, // 分享标题
				desc: desc, // 分享描述
				link: url, // 分享链接
				imgUrl: imgurl, // 分享图标
				type: '', // 分享类型,music、video或link，不填默认为link
				dataUrl: '', // 如果type是music或video，则要提供数据链接，默认为空
				success: function () {
				 // 用户确认分享后执行的回调函数
				},
				cancel: function () {
				 // 用户取消分享后执行的回调函数
				}
			});
		
			//获取“分享到朋友圈”按钮点击状态及自定义分享内容接口（即将废弃）
			jweixin.onMenuShareTimeline({
				title: title, // 分享标题
				desc: desc, // 分享描述
				link: url, // 分享链接
				imgUrl: imgurl, // 分享图标
				type: '', // 分享类型,music、video或link，不填默认为link
				dataUrl: '', // 如果type是music或video，则要提供数据链接，默认为空
				success: function () {
				 // 用户确认分享后执行的回调函数
				},
				cancel: function () {
				 // 用户取消分享后执行的回调函数
				}
			});
		
			//获取“分享到腾讯微博”按钮点击状态及自定义分享内容接口
			jweixin.onMenuShareWeibo({
				title: title, // 分享标题
				desc: desc, // 分享描述
				link: url, // 分享链接
				imgUrl: imgurl, // 分享图标
				type: '', // 分享类型,music、video或link，不填默认为link
				dataUrl: '', // 如果type是music或video，则要提供数据链接，默认为空
				success: function () {
				 // 用户确认分享后执行的回调函数
				},
				cancel: function () {
				 // 用户取消分享后执行的回调函数
				}
			});
		
			//获取“分享到QQ空间”按钮点击状态及自定义分享内容接口（即将废弃）
			jweixin.onMenuShareQZone({
				title: title, // 分享标题
				desc: desc, // 分享描述
				link: url, // 分享链接
				imgUrl: imgurl, // 分享图标
				type: '', // 分享类型,music、video或link，不填默认为link
				dataUrl: '', // 如果type是music或video，则要提供数据链接，默认为空
				success: function () {
				 // 用户确认分享后执行的回调函数
				},
				cancel: function () {
				 // 用户取消分享后执行的回调函数
				}
			});
		});
	},
	fail: err => {
		console.log('request fail', err);
	}
});	
