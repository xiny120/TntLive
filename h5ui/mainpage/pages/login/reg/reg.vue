<template>
    <form class='loginView' @submit="formSubmit" @reset="formReset">
            <view class="input-view">
                <text class="title">用户账号：</text>
                <m-input type="text" focus clearable v-model="account" placeholder="请填写新账号"></m-input>
            </view>
            <view class="input-view">
                <text class="title">用户密码：</text>
                <m-input type="password" displayable v-model="password" placeholder="请填写密码"></m-input>
            </view>
            <view class="input-view">
                <text class="title">确认密码：</text>
                <m-input type="password" displayable v-model="password1" placeholder="请再填写一遍密码"></m-input>
            </view>

            <view class="input-view">
                <text class="title">邮箱或手机：</text>
                <m-input type="text" clearable v-model="email" placeholder="请填写邮箱或手机号码"></m-input>
            </view>
        <view class="input-view">
            <button type="primary" class="primary" @tap="register" :disabled="isSubmitBnDisable" >注册</button>
        </view>
    </form>
</template>

<script>
    import service from '../../../service.js';
    import mInput from '../../../components/m-input.vue';

    export default {
        components: {
            mInput
        },
        data() {
            return {
				isSubmitBnDisable: false,
                account: '',
                password: '',
				password1:'',
                email: '',
				cellphone:'',
            }
        },
        methods: {
            register() {
                /**
                 * 客户端对账号信息进行一些必要的校验。
                 * 实际开发中，根据业务需要进行处理，这里仅做示例。
                 */
				
				this.isSubmitBnDisable = true;

				setTimeout(() => {
				  this.isSubmitBnDisable = false
				}, 1600)
				
                if (this.account.length < 5) {
                    uni.showToast({
                        icon: 'none',
                        title: '账号最短为 5 个字符'
                    });
                    return;
                }
                if (this.password.length < 6) {
                    uni.showToast({
                        icon: 'none',
                        title: '密码最短为 6 个字符'
                    });
                    return;
                }
				
				if(this.password != this.password1){
                    uni.showToast({
                        icon: 'none',
                        title: '两次填写的密码不一致，请重新输入！'
                    });
                    return;
				}
				
				var msg = "手机号码正确";
				var ok = false;
				var email0="";
				var cellphone0 = "";
				
				if(this.email.length < 5){
                    uni.showToast({
                        icon: 'none',
                        title: '电子邮件或者手机号码填写错误！'
                    });
                    return;					
				}
				
				if(/^[0-9]*$/.test(this.email)){
					if(!(/^1[34578]\d{9}$/.test(this.email))){
						msg = "手机号码有误，请重填";  					
					}
					else{
						msg = "手机号码正确";
						cellphone0 = this.email;
						ok = true;
					}
				}
				else{

					if(!(/^[A-Za-z0-9._%-]+@([A-Za-z0-9-]+\.)+[A-Za-z]{2,4}$/.test(this.email))){	
						msg = "邮箱地址不合法";
					}else{
						msg = "邮箱地址正确";
						email0 = this.email;
						ok = true;
					}
			
				} 
							
				if(!ok){
                    uni.showToast({
                        icon: 'none',
                        title: msg,
                    });
					return;
				}


                const data = {
                    account: this.account,
                    password: this.password,
                    email: email0,
					cellphone:cellphone0,
                }
				
                service.addUser(data);
				console.log(data);
                uni.showToast({
                    title: '注册成功',
					complete: function(res){
				
					}
                });
				
				setTimeout(() => {
					uni.navigateBack({
							delta: 1
					});						  
				}, 2000)				
				

            }
        }
    }
</script>

<style>

</style>
