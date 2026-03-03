/* 心跳检查逻辑 */
const HEARTBEAT_INTERVAL = 5000; // 5秒检查一次
let isSessionExpired = false;    // 过期标志位

// 每HEARTBEAT_INTERVAL心跳检查一次
initHttpHeartbeat();

// 心跳检测
async function httpHeartbeat(){
    if(isSessionExpired) return;

    try {
        // 发送请求检查 Session 是否有效
        const response = await fetch('/chat/check');
        const data = await response.json();
        if (response.status === 401 && data.success) {
            isSessionExpired = true;
            console.warn("Session已过期，准备刷新...");
            alert("您的会话已过期，请重新登录");         // 这里有个问题:alert会阻塞主线程,但不阻塞setInterval,导致可能重复触发,通过标志位解决
            isSessionExpired = false;
            window.location.reload();  // 刷新
            // window.location.href = '/chat.html'; // 或者跳转登录页
        }
        else{
            console.log("心跳检测通过,连接会话vaild");
        }
    } 
    catch (error) {
        console.error("心跳检测网络错误:", error);
    }

    // 关键点：只有当前逻辑执行完了（且没过期），才开启下一次倒计时
    if (!isSessionExpired) {
        setTimeout(httpHeartbeat, HEARTBEAT_INTERVAL);
    }
}

function initHttpHeartbeat() {
    // setInterval(httpHeartbeat,HEARTBEAT_INTERVAL); // 定时执行
    setTimeout(httpHeartbeat,HEARTBEAT_INTERVAL); // 只延时执行一次，上次完成了进行下次
}