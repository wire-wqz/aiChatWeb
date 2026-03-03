/* 会话处理逻辑 */
let currentSessionId = null;                                            // 当前会话ID
let tempSession = true;                                                 // 是否为临时会话
const sessions = {};                                                    // 存储会话列表:存储 name 和 messages
const sessionList = document.getElementById('sessionList');             // 会话列表
const newChatBtn = document.getElementById('newChatBtn');

/*会话列表初始化函数*/
loadSessions();

/*绑定回调函数*/
newChatBtn.addEventListener("click",addNewChat);

// 在会话列表中将存储的sessions信息显示
function renderSessionList() {
    const reversedIds = Object.keys(sessions).reverse();

    sessionList.innerHTML = '';
    for (let id of reversedIds) {
        const li = document.createElement('li');
        li.dataset.id = id;
        if (id === currentSessionId) 
            li.classList.add('active');

        // 会话名称
        const nameSpan = document.createElement('span');
        nameSpan.className = 'session-name';
        nameSpan.textContent = sessions[id].name || `会话 ${id}`;

        // 删除图标
        const deleteBtn = document.createElement('span');
        deleteBtn.className = 'delete-icon';
        deleteBtn.innerHTML = '&times;'; // HTML实体 ×
        deleteBtn.title = "删除会话"

        // 绑定删除会话回调函数
        deleteBtn.addEventListener("click", (e) => {
            e.stopPropagation();  // 关键：阻止事件冒泡！防止触发 li 的 switchSession 点击事件
            deleteSession(id);
        });
        
        // 将两个元素添加到 li
        li.appendChild(nameSpan);
        li.appendChild(deleteBtn);
        
       
        // 绑定切换会话事件（点击 li 的空白处或文字时触发）
        li.addEventListener("click",() => switchSession(id));

        // 加入无序列表
        sessionList.appendChild(li);
    }
}

// 添加新的会话
function addNewChat(){
    if(tempSession==true)
    {
        alert("ℹ️已是最新的对话");
    }
    else
    {
        currentSessionId = null;
        tempSession = true;
        clearMessages();
        updateRecommendVisibility();
    }
}

// 会话切换函数
async function switchSession(id) {
    currentSessionId = String(id); 
    tempSession = false; 
    updateRecommendVisibility();
    clearMessages();
    // loadSessions仅恢复会话id和会话名,只有真正切换到具体会话时才恢复全部的message
    if (!sessions[id].messages || sessions[id].messages.length === 0) 
    {
        try {
            const res = await fetch('/chat/messages', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ sessionId: currentSessionId }) });
            const data = await res.json();
            if (data.success && Array.isArray(data.messages)) {
                sessions[id].messages = [];
                data.messages.forEach(item => { sessions[id].messages.push({ role: item.is_user ? 'user' : 'assistant', content: item.content }); });
            }
        } catch (err) { console.error('获取历史失败', err); }
    } 
    // 将该会话的消息添加回消息显示区
    sessions[id].messages.forEach(m => appendMessage(m.role, m.content));
    // 更新会话列表
    renderSessionList();
}

// 从后端初始化会话列表:临时会话和登录会话都从内存中加载,在初始化时,登录会话的信息已经从sql加载到内存了
async function loadSessions() {
    try {
        // 发送url请求
        const res = await fetch('/chat/sessions');
        // 解析响应
        const data = await res.json();
        // 可能返回为空,为空则不初始化
        if (data.success && Array.isArray(data.sessions)) {
            // 取出data.sessions(仅取出了name和id,节约内存)
            data.sessions.forEach(s => { // s表示data.sessions
                const sid = String(s.sessionId);
                sessions[sid] = { name: s.name || `会话 ${sid}`, messages: [] 
                };
            });
            renderSessionList();
        }
    } 
    catch (err) {
        console.error('获取会话列表失败', err);
    }
}

// 删除会话函数
async function deleteSession(id) {
    // 简单确认（可选）
    if (!confirm('确定要删除这个会话吗？')) return;

    try {

        // 后端删除数据记录  
        const res = await fetch(`/chat/delSession`, { 
            method: 'POST',
            headlers: { "Content-Type": "application/json" },
            body:JSON.stringify(
            {
                sessionId: String(id)
            })
        });

        const data = await res.json();
        if (!data.success) {
            alert('删除失败');
            return;
        }
        
        
        // 3. 前端逻辑处理
        delete sessions[id];

        // 如果删除的是当前正在查看的会话，重置界面
        if (currentSessionId === id) {
            addNewChat();
        }

        // 重新渲染列表
        renderSessionList();

    } catch (err) {
        console.error('删除会话出错', err);
        alert('删除出错，请检查网络');
    }
}