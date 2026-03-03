/* 登录逻辑 */
let isLogin = false;                                           // 是否登录
let isRegisterMode = false;                                    // 登录遮罩层的登录/注册模式
let totalTokens = 0;

const modal = document.getElementById('loginModal');                   // 登录遮罩层
const modalTitle = document.getElementById('modal-title');             // 登录遮罩层的主标题
const modalSubtitle = document.getElementById('modal-subtitle');       // 登录遮罩层的次标题
const closeBtn = document.querySelector('.close-btn');                 // 关闭按钮
const loginForm = document.getElementById('loginForm');                // 表单整体
const userNameInput = document.getElementById('username');             // 获取用户名
const passwordInput = document.getElementById('password');             // 获取密码
const confirmPwdGroup = document.getElementById('confirm-pwd-group');  // 确认密码框整体显示
const confirmPwdInput = document.getElementById('confirm-password');   // 确认密码框
const submitBtn = document.getElementById('login-submit-btn');         // 表单提交按钮
const registerText = document.getElementById('register-text');         // 注册提示小字
const registerBtn = document.getElementById('register-btn')            // 注册按钮

const openBtn = document.querySelector('.login-btn');          // 会话列表下的登录按钮
const userBtn = document.getElementById('user-action-btn');    // 会话列表下的用户信息(和openBtn一致)
const userIcon = document.getElementById('user-icon');         // 用户图标
const userNameSpan  = document.getElementById('user-name');    // 用户名
const btnRecharge = document.getElementById('btn-recharge');   // 充值按钮
const btnLogout = document.getElementById('btn-logout')        // 退出登录按钮 

const badge = document.getElementById('token-balance');        // token余额显示

loadLoginStatus(); // 获取登录状态

// 绑定登录按钮
openBtn.addEventListener('click', () => {
    if(!isLogin)
    {
        modal.classList.add('active');                             // 添加class触发显示和动画(登录遮罩层添加active)
        document.getElementById('username').focus();               // 自动聚焦用户名输入框
    }
});

// 登录界面中的关闭按钮
closeBtn.addEventListener('click', () => {
    modal.classList.remove('active');                          // 移除登录遮罩层的active  
    isRegisterMode = false;  
    // -> 切换为【登录模式】
    modalTitle.innerText = "欢迎回来";
    modalSubtitle.innerText = "请登录您的账号以继续";
    submitBtn.innerText = "立即登录";
    registerText.innerText = "还没有账号？";
    registerBtn.innerText = "注册新账号";
    confirmPwdGroup.style.display = "none";
    confirmPwdInput.removeAttribute("required"); // 登录时不需要填
    // 清空输入
    userNameInput.value = ""; 
    passwordInput.value = ""; 
    confirmPwdInput.value = "";
});

// 点击背景空白处，也可以关闭
/*modal.addEventListener('click', (e) => {
    if (e.target === modal) {
        modal.classList.remove('active');
    }
});*/

// 绑定登录表单提交选项
loginForm.addEventListener("submit",loginFormSumbit);

// 绑定退出登录选择
btnLogout.addEventListener("click",async ()=>{
    if(isLogin)
    {
        try {
            const response = await fetch('/chat/logout', { method: 'POST' });
            const data =await response.json();
            if(response.ok&&data.success)
            {
                alert("退出登录");
                window.location.reload(); // 刷新，等效为全部重置，由loadLoginStatus判断登录逻辑
            }
            else{
                console.log("登出失败");
            }
        } catch (error) {
            console.error("Login error:", error);
            alert("网络错误，请稍后再试");
        } 
    }
});

// 切换逻辑
registerBtn.addEventListener("click",()=>{
    
    isRegisterMode = !isRegisterMode; // 状态取反
    userNameInput.value = ""; 
    passwordInput.value = ""; 
    confirmPwdInput.value = "";
    if (isRegisterMode) 
    {
        // -> 切换为【注册模式】
        modalTitle.innerText = "创建新账号";
        modalSubtitle.innerText = "请填写以下信息注册";
        submitBtn.innerText = "立即注册";
        registerText.innerText = "已有账号？";
        registerBtn.innerText = "直接登录";
        
        // 显示确认密码框
        confirmPwdGroup.style.display = "block";
        confirmPwdInput.setAttribute("required", "required"); // 注册时必须填
    } else {
        // -> 切换为【登录模式】
        modalTitle.innerText = "欢迎回来";
        modalSubtitle.innerText = "请登录您的账号以继续";
        submitBtn.innerText = "立即登录";
        registerText.innerText = "还没有账号？";
        registerBtn.innerText = "注册新账号";
        
        // 隐藏确认密码框
        confirmPwdGroup.style.display = "none";
        confirmPwdInput.removeAttribute("required"); // 登录时不需要填
        confirmPwdInput.value = ""; // 清空
    }
});

// 登录表单提交-交互函数
async function loginFormSumbit(e){
    e.preventDefault(); // 阻止表单默认跳转

    const userNameVal = userNameInput.value;      // 获取用户名
    const passwordVal = passwordInput.value;      // 获取密码
    const confirmPasswordVal = confirmPwdInput.value;


    // UI反馈：按钮变灰并显示加载中
    let isOriginalBtnText = true;
    const originalBtnText = submitBtn.innerText;
    submitBtn.innerText = "处理中...";
    submitBtn.disabled = true;
    submitBtn.style.opacity = "0.7";

    // 后端交互
    try{
        if (isRegisterMode)  // --- A. 注册逻辑分支 ---
        {   
            // 前端校验
            if (passwordVal !== confirmPasswordVal) {
                alert("两次输入的密码不一致！");
                return; // 终止执行，finally 会恢复按钮
            }
            if (passwordVal.length < 6) {
                alert("密码长度不少于6位");
                return; // 终止执行，finally 会恢复按钮
            }
            if (passwordVal.length > 18) {
                alert("密码长度建议不多于18位");
                return; // 终止执行，finally 会恢复按钮
            }

            // 发起注册请求 (后端接口为 /chat/register)
            const response = await fetch("/chat/register", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    userName: userNameVal,
                    password: passwordVal
                })
            });
            
            const data = await response.json();
            
            if (response.ok && data.success) 
            {
                if(data.registerStatus)
                {
                    alert("注册成功！请登录。");
                    isOriginalBtnText = false;
                }
                else
                {
                    alert("注册失败:"+"用户已存在");
                }
            } 
            else 
            {
                alert("注册失败: 内部错误");
            }
        } 
        else               // --- B. 登录逻辑分支 ---
        {
            const response = await fetch("/chat/login",
            {
                method:"POST",
                headers:{"Content-Type":"application/json"},
                body:JSON.stringify({
                    userName:userNameVal,
                    password:passwordVal
                })
            });
            const data = await response.json();
            if (response.ok && data.success){
                if(data.is_Login){
                    modal.classList.remove('active'); // 关闭窗口 
                    alert("登录成功！");
                    window.location.reload();         // 刷新页面更新状态
                }
                else
                {
                    const errorMessage = "登录失败，请检查账号或密码";
                    alert(errorMessage); // 弹出错误提示
                }
            }
            else
            {
                const errorMessage = "错误，数据库无法检索";
                alert(errorMessage);    // 弹出错误提示
            }
        }
    }
    catch (error) {
        console.error("Login error:", error);
        alert("网络错误，请稍后再试");
    } finally {
        // 恢复按钮状态
        if(isOriginalBtnText){
            submitBtn.innerText = originalBtnText;
        }
        else{
            // 注册成功后，自动切换回登录视图
            registerBtn.click(); 
            // 自动填入刚才注册的账号
            userNameInput.value = userNameVal;
            passwordInput.value = "";
        }       
        submitBtn.disabled = false;
        submitBtn.style.opacity = "1";
    }
}

// 获取本次会话的登录状态
async function loadLoginStatus(){
    try{
        /*调试用*/
        /*
        isLogin = true;
        userBtn.classList.add("is-logged-in");// 添加登录信息
        loginStatusName.innerHTML = data.userName;
        console.log(data.userName+"已登录");
        */
        
        const response = await fetch("/chat/loginStatus");
        const data = await response.json();
        if (response.ok && data.success){
            isLogin = true;
            console.log(data.userName+"已登录");
            // 修改图标
            userIcon.remove();
            // 添加登录用户信息
            userBtn.classList.add("is-logged-in");
            userNameSpan.innerHTML = data.userName;
            // 添加登录Tokens信息
            console.log(data.totalTokens);
            totalTokens = data.totalTokens;
            badge.classList.remove('hidden');
            addTokenHTML(totalTokens);
        }
    }
    catch (error) {
        console.error("Login error:", error);
    } 
} 

function addTokenHTML(tokens){

    let displayValue;
    if (tokens > 10000) {
        displayValue = (tokens / 1000).toFixed(1) + 'k';
    }
    badge.innerText ='余额：' + displayValue + ' T'; // 加个 T 代表 Token


    badge.classList.remove('token-status-good', 'token-status-warning', 'token-status-danger');
    if (tokens >= 10000) {
        badge.classList.add('token-status-good');   // 绿色
    } else if (tokens >= 1000) {
        badge.classList.add('token-status-warning'); // 橙色
    } else {
        badge.classList.add('token-status-danger');  // 红色
    }
}