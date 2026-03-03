


// 填充输入框
function fillInput(text) {
    const inputArea = document.getElementById('question');
    if (inputArea) {
        inputArea.value = text;
        inputArea.focus(); // 聚焦以便用户直接发送
    }
}


/**
 * 更新推荐栏的显示状态
 * 请在切换会话(newChat)或发送消息后调用此函数
 */
function updateRecommendVisibility() {
    const recommendDiv = document.getElementById('recommend-container');
    const chatContainer = document.getElementById('chat-container');
    
    // 判断逻辑：
    // 1. tempSession 必须为 true
    // 2. 聊天记录最好是空的 (避免在历史记录中间显示)
    const isChatEmpty = chatContainer.children.length === 0;

    if (typeof tempSession !== 'undefined' && tempSession === true && isChatEmpty) {
        recommendDiv.classList.remove('hidden');
    } else {
        recommendDiv.classList.add('hidden');
    }
}

updateRecommendVisibility();