var ws = null;

connect.onclick = function () 
{
    ws = new WebSocket(uri.value);

    ws.onopen = function (ev) 
	{
        messages.innerText += "[connection opened]\n";
    };

    ws.onclose = function (ev) 
	{
        messages.innerText += "[connection closed]\n";
    };

    ws.onmessage = function (ev) 
	{
        messages.innerText += ev.data + "\n";
    };

    ws.onerror = function (ev) 
	{
        messages.innerText += "[error]\n";
        console.log(ev);
    };
};

disconnect.onclick = function () 
{
    ws.close();
};

send.onclick = function () 
{
    ws.send(userName.value + ": " + sendMessage.value);
    sendMessage.value = "";
};

sendMessage.onkeyup = function (ev) 
{
    ev.preventDefault();
    if (event.keyCode === 13)
	{
        send.click();
    }
}