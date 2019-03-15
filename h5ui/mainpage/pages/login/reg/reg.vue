<template>
    <form class='loginView' @submit="formSubmit" @reset="formReset">
            <view class="input-view">
                <text class="title">账号：</text>
                <m-input type="text" focus clearable v-model="account" placeholder="请输入账号"></m-input>
            </view>
            <view class="input-view">
                <text class="title">密码：</text>
                <m-input type="password" displayable v-model="password" placeholder="请输入密码"></m-input>
            </view>
            <view class="input-view">
                <text class="title">邮箱：</text>
                <m-input type="text" clearable v-model="email" placeholder="请输入邮箱"></m-input>
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
                email: ''
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
				}, 2000)
				
				
				
				console.log("reg");
				
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
                if (this.email.length < 3 || !~this.email.indexOf('@')) {
                    uni.showToast({
                        icon: 'none',
                        title: '邮箱地址不合法'
                    });
					
                    return;
                }

                const data = {
                    account: this.account,
                    password: this.password,
                    email: this.email
                }
				
                service.addUser(data);
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
