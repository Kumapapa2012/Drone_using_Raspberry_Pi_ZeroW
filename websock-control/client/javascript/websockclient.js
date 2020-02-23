// handlers
// connect
function onOpen(d) {
  // Connected
  console.log("Connected!!");
  //connection.send("サンプルデータ");
}
function onClose() {
  // Closed
  console.log("Closed!!");
}
// receive
function onMessage(d) {
  // received from server.
  // this must be telemetry data fomr Drone...

  // for testing...
  var d2 = document.getElementById("testDiv2");
  var p2 = document.createElement("pre");
  p2.textContent = d.data + Date.now();
  d2.innerHTML = p2.outerHTML;

  wsAcked = true;
}
function onError(event) {
  // Error
  console.error("Error!!",event);
}

//test


// main
var wsconn = new WebSocket("ws://192.168.1.167:8000/ws");
wsconn.onmessage = onMessage;
wsconn.onopen = onOpen;
wsconn.onclose = onClose;
wsconn.onerror = onError;

// Hmm.. there should be a better way but for now...
var wsAcked = true;