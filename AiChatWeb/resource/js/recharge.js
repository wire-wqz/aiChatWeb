const btnRechargeMenu = document.getElementById('btn-recharge');      // 侧边栏菜单里的按钮，充值中心按钮
const rechargeModal = document.getElementById('rechargeModal');       // 充值模态框总体
const closeRechargeBtn = document.getElementById('close-recharge');   // 关闭充值模态框按钮
const rechargeCards = document.querySelectorAll('.recharge-card');    // 充值金额卡片
const payBtn = document.getElementById('btn-pay-confirm');            // 确定选项

const rechargeTokens = document.getElementById('recharge-tokens');    // Tokens余额


// 点击充值中心
btnRechargeMenu.addEventListener('click', function() {
    rechargeModal.classList.add('active'); 
    rechargeTokens.innerHTML = totalTokens;
    // 默认清除之前的选择
    clearSelection();
    
    // // 这里的 user-menu 可能会挡住模态框，建议点击后隐藏菜单
    // const userMenu = document.getElementById('user-menu');
    // if(userMenu) 
    //     userMenu.style.display = 'none'; 
});

// 关闭充值模态框
closeRechargeBtn.addEventListener('click', closeRechargeModal);

// 充值套餐选择（给所有卡片添加click回调函数）
rechargeCards.forEach(card => {
    card.addEventListener('click', function() {
        // 移除其他卡片的选中状态
        clearSelection();
        
        // 选中当前卡片
        this.classList.add('selected');
        
        // 更新按钮文字
        const price = this.getAttribute('data-price');   // 注意!!!!:getAttribute()方法获得的一定是字符串string类型
        const tokens = this.getAttribute('data-tokens');
        
        payBtn.removeAttribute('disabled');
        payBtn.innerText = `立即支付 ¥${price}`;
        payBtn.style.backgroundColor = '#ff9800'; // 变为橙色支付色
        
        // 存储当前选中的数据（用于后续支付逻辑）
        payBtn.dataset.currentPrice = price;  // 这里存储的也会自动转化为string
        payBtn.dataset.currentTokens = tokens;
    });
});

// 支付按钮点击逻辑
payBtn.addEventListener('click', async function(e) {
    e.preventDefault(); 
    
    if (this.hasAttribute('disabled')) return;

    const price = Number(this.dataset.currentPrice);     // 充值金额
    const tokens = Number(this.dataset.currentTokens);   // 充值Tokens
 
    this.innerText = '支付处理中...';
    if(price == 0)
    {
        try{
            const response = await fetch("/chat/rechargeUpdate",{
                method:"POST",
                headers: { "Content-Type": "application/json" },
                body:JSON.stringify(
                {
                    totalTokens:totalTokens+tokens
                })
            });
            const data = await response.json();
            if (response.ok && data.success){
                totalTokens += tokens;
                addTokenHTML(totalTokens);
                alert("🎉充值成功！🎉");
                clearSelection();
                closeRechargeModal();
            }
            else
            {
                alert("后端内部错误");
                clearSelection();
                closeRechargeModal();
            }
        }
        catch (error) {
            console.error("Recharge error:", error);
            alert("网络错误");
            clearSelection();
            closeRechargeModal();
        } 
    }
    else
    {
        const result = await payPrice(price); 
        clearSelection();
        closeRechargeModal();
    }
});

// 充值逻辑
async function payPrice(price){
    try{
        const response = await fetch("/chat/recharge",{
            method:"POST",
            headers: { "Content-Type": "application/json" },
            body:JSON.stringify(
            {
                price:price
            })
        });
        const data = await response.json();
        if (response.ok && data.success){
            // 跳转到支付标签页
            window.location.href = data.htmlUrl;
        }
        else
        {
            alert("后端内部错误");
            clearSelection();
            closeRechargeModal();
        }
    }
    catch (error) {
        console.error("payPrice error:", error);
        alert("网络错误");
        clearSelection();
        closeRechargeModal();
    } 
}




function closeRechargeModal(){
    rechargeModal.classList.remove('active'); 
}


// 清除到初始化状态
function clearSelection() {
    rechargeCards.forEach(c => c.classList.remove('selected'));
    payBtn.setAttribute('disabled', 'true');
    payBtn.innerText = '请选择金额';
    payBtn.style.backgroundColor = ''; // 恢复默认灰色
}