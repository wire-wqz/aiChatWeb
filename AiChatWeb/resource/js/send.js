/* 发送消息逻辑 */
const chatContainer = document.getElementById('chat-container');        /* 消息显示区 */
const chatForm = document.getElementById('chat-form');                  /* 消息发送表单 */
const questionInput = document.getElementById('question');              /* textarea消息输入框 */
const modelTypeSelect = document.getElementById('modelType');           /* 模型选择select */

/*绑定回调函数*/
chatForm.addEventListener("submit",chatFormSumbit);

/*消息清空函数*/
function clearMessages() 
{ chatContainer.innerHTML = ''; }

/*消息发送函数*/
async function appendMessage(role, content) {
    // 创建一个div对象
    const msgDiv = document.createElement('div');   
    // 给div对象添加类    
    msgDiv.classList.add('message', role);
    // 将content先由makedown转为html格式（marked.parse）、然后安全插入，过滤掉可能导致 XSS 攻击的恶意代码（DOMPurify.sanitize）
    const safeHtml = DOMPurify.sanitize(marked.parse(content));
    // 插入HTML内容
    msgDiv.innerHTML = safeHtml;
    // 将div对象添加为chatContainer的子对象
    chatContainer.appendChild(msgDiv);
    // 聊天容器的滚动条自动滚动到底部
    chatContainer.scrollTop = chatContainer.scrollHeight; 
    //const escapedContent = content.replace(/`/g, '\\`').replace(/\$/g, '\\$'); 将字符串中的反引号和美元符号前加上反斜杠进行转义
}

/*  消息发送回调函数 */
async function chatFormSumbit(e){
    console.log("一次发送事件");
    //e为事件对象，阻止事件对象
    e.preventDefault();
    // 获取并清理用户输入前的空格
    const question = questionInput.value.trim();
    if (!question) return;
    // 前端消息显示
    appendMessage('user', question);
    /* 后端处理 */
    if (tempSession) {    // 临时会话
        try {
            // 向'/chat/send-new-session'发出POST请求，请求体包括用户输入question和模型选择select的值
            const response = await fetch('/chat/send', 
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    question, 
                    modelType: modelTypeSelect.value,
                    isMcp:isMcp,
                    isGoogle:isGoogle
                }) // // question 等效于question:question
            });

            // 模拟调试
            /*const response = {
                json: async () => {
                    return {
                        success: true, 
                        sessionId: "mock_session_" + Date.now(), 
                        Information: `[前端模拟回复]这是一个静态调试响应。`
                    };
                }
            };*/

            
            // 从响应体中读取并解析​ JSON 数据
            const data = await response.json();

            if (data.success) {
                // 赋予会话ID
                const sessionId = String(data.sessionId);
                sessions[sessionId] = {
                    name: question,
                    messages: [
                        { role: 'user', content: question },
                        { role: 'assistant', content: data.Information }
                    ]
                };
                console.log(sessionId);
                currentSessionId = sessionId;
                console.log(currentSessionId);
                tempSession = false;
                updateRecommendVisibility();
                renderSessionList();
                appendMessage('assistant', data.Information);
                if(isLogin)
                {
                    totalTokens=data.totalTokens; // totaltokens是全局变量，存储当前该会话的tokens数
                    addTokenHTML(totalTokens);
                }
            } 
            else {
                appendMessage('assistant', '[错误] ' + (data.message || '未知错误'));
            }
        } 
        catch (err) 
        {
            console.error(err);
            appendMessage('assistant', '[错误] 无法连接到服务器');
        }
    }
    else {
        // 向sessions中存储对话内容
        sessions[currentSessionId].messages.push({ role: 'user', content: question });
        console.log(currentSessionId);
        // 后端处理
        try {
            const response = await fetch('/chat/send', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    question, 
                    modelType: modelTypeSelect.value, 
                    sessionId: currentSessionId,
                    isMcp:isMcp,
                    isGoogle:isGoogle
                }) // question 等效于question:question
            });
            const data = await response.json();
            // 成功获取后端响应
            if (data.success) {
                // 前端显示ai对话
                appendMessage('assistant', data.Information);
                // 向sessions中存储对话内容
                sessions[currentSessionId].messages.push({ role: 'assistant', content: data.Information });
                if(isLogin)
                {
                    totalTokens=data.totalTokens; // totaltokens是全局变量，存储当前该会话的tokens数
                    addTokenHTML(totalTokens);
                }
            } 
            else 
            {
                appendMessage('assistant', '[错误] ' + (data.message || '未知错误'));
            }
        } 
        catch (err) 
        {
            console.error(err);
            appendMessage('assistant', '[错误] 无法连接到服务器');
        }
    }

    // 清空输入框
    questionInput.value = '';
    initInputResize();
}

// 输入框自动高度调整
document.addEventListener('DOMContentLoaded', () => {
    const textarea = document.getElementById('question');

    if (textarea) {
        // 初始化：设置初始高度
        textarea.style.height = '44px'; 

        // 监听输入事件
        textarea.addEventListener('input', function() {
            autoResize(this);
        });
        
        // 监听聚焦，防止奇怪的跳动
        textarea.addEventListener('focus', function() {
            autoResize(this);
        });
    }
});

function autoResize(el) {
    // 1. 先重置高度，以便准确计算 scrollHeight (用于缩回)
    el.style.height = 'auto'; 
    el.style.height = '44px'; // 恢复最小高度基准

    // 2. 获取实际内容高度
    let newHeight = el.scrollHeight;

    // 3. 定义最大高度 (需要与 CSS 中的 max-height 一致)
    const maxHeight = 180; 

    // 4. 判断逻辑
    if (newHeight > maxHeight) {
        // 超过最大高度：固定高度，开启滚动条
        el.style.height = maxHeight + 'px';
        el.style.overflowY = 'auto'; 
    } else {
        // 未超过：高度设为内容高度，隐藏滚动条
        el.style.height = newHeight + 'px';
        el.style.overflowY = 'hidden';
    }
}

function initInputResize(){
    const inputArea = document.getElementById('question');
    inputArea.style.height = '44px'; // 这里的数值要对应 CSS 的 min-height
    inputArea.style.overflowY = 'hidden';
}