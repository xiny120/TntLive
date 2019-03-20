<template>
	<view>
		<text>如忘记帐号密码，请联系QQ：59838713</text>
	<form class='loginView' @submit="formSubmit" @reset="formReset"  v-show="0">    
            <view class="input-view">
                <text class="title">邮箱：</text>
                <m-input type="text" focus clearable v-model="email" placeholder="请输入邮箱"></m-input>
            </view>

        <view class="input-view">
            <button type="primary" class="primary" @tap="findPassword">提交</button>
        </view>
    </form>		
	</view>

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
                email: ''
            }
        },
        methods: {
            findPassword() {
                /**
                 * 仅做示例
                 */
                if (this.email.length < 3 || !~this.email.indexOf('@')) {
                    uni.showToast({
                        icon: 'none',
                        title: '邮箱地址不合法',
                    });
                    return;
                }
                uni.showToast({
                    icon: 'none',
                    title: '已发送重置邮件至注册邮箱，请注意查收。',
                    duration: 3000
                });
            }
        }
    }
</script>

<style>

</style>
