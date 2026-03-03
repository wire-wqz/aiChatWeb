// 工具 联网按钮处理逻辑
const netButton = document.getElementById('btn-net-search');      // 联网工具按钮
const toolButton = document.getElementById('btn-tool-use');       // 工具使用按钮
let isGoogle = false;
let isMcp = false;

// 初始化函数
initButton();

// 绑定回调函数
netButton.addEventListener("click",clickNetButton);
toolButton.addEventListener("click",clickToolButton);


// 按钮初始化函数
function initButton(){
    changeNetButton(); 
}

// 联网按钮点击函数
async function clickNetButton()
{
    changeNetButton();
    // 通过发送函数，统一进行后端交互
    // try {
    //     const res = await fetch(`/chat/tools`, { 
    //         method: 'POST',
    //         headlers: { "Content-Type": "application/json" },
    //         body:JSON.stringify(
    //         {
    //             isGoogle: isGoogle
    //         })
    //     });

    //     const data = await res.json();
    //     if (!data.success) {
    //         console.error('工具按钮处理失败');
    //         return;
    //     }
    // } catch (err) {
    //     console.error('工具按钮网络连接错误', err);
    // }
}


// 联网按钮点击函数
async function clickToolButton()
{
    changeToolButton();
    // try {
    //     const res = await fetch(`/chat/tools`, { 
    //         method: 'POST',
    //         headlers: { "Content-Type": "application/json" },
    //         body:JSON.stringify(
    //         {
    //             isMcp: isMcp
    //         })
    //     });

    //     const data = await res.json();
    //     if (!data.success) {
    //         console.error('工具按钮处理失败');
    //         return;
    //     }
    // } catch (err) {
    //     console.error('工具按钮网络连接错误', err);
    // }
}



// 反转联网工具按钮状态
function changeNetButton(){
    if(isGoogle)
    {
        netButton.classList.remove("active");
    }
    else
    {
        netButton.classList.add("active");
    }
    isGoogle = !isGoogle;
}

// 反转工具使用按钮
function changeToolButton(){
    if(isMcp)
    {
        toolButton.classList.remove("active");
    }
    else
    {
        toolButton.classList.add("active");
    }
    isMcp = !isMcp;
}