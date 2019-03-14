<template>
	<form class='loginView' @submit="formSubmit" @reset="formReset">
		<view class="input-view">
			<view class="label-view">
				<text class="label">账号 </text>
			</view>
			<input class="input" type="text" placeholder="请输入用户名/帐号/手机号码" name="nameValue" />
		</view>
		<view class="input-view">
			<view class="label-view">
				<text class="label">密码</text>
			</view>
			<input class="input" type="password" placeholder="请输入密码" name="passwordValue" />
		</view>
		<view class="button-view">
			<button type="default" class="login" hover-class="hover" formType="submit">登录</button>
			<button type="default" class="register" hover-class="hover" @click="register">免费注册</button>
		</view>
	</form>
</template>

<script>
	var  graceChecker = require("../../common/graceChecker.js");
	export default {
		data() {
			return {};
		},
		methods: {
			register() {
				console.log("前往注册页面")
			},
			formSubmit: function (e) {
				//将下列代码加入到对应的检查位置
				//定义表单规则
				var rule = [
					{name:"nameValue", checkType : "string", checkRule:"1,30",  errorMsg:"请输入帐号！"},
					{name:"passwordValue", checkType : "string", checkRule:"1,30",  errorMsg:"请输入密码"},
				];
				//进行表单检查
				var formData = e.detail.value;
				var checkRes = graceChecker.check(formData, rule);
				if(checkRes){
					uni.showToast({title:"验证通过!", icon:"none"});
					console.log("得到账号:"+ e.detail.value.nameValue + ';得到密码:' + e.detail.value.passwordValue)					
				}else{
					uni.showToast({ title: graceChecker.error, icon: "none" });
				}
			},
			formReset: function (e) {
				console.log("清空数据")
				this.chosen = ''
			}			
		}
	}
</script>

<style>
	
</style>
