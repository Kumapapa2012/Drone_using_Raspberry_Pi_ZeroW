/*
 * Gamepad API Test
 * Written in 2013 by Ted Mielczarek <ted@mielczarek.org>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */
var haveEvents = "GamepadEvent" in window;
var controllers = {};
var rAF =
  window.mozRequestAnimationFrame ||
  window.requestAnimationFrame;
var strControllerPrev="";

function connecthandler(e) {
  addgamepad(e.gamepad);
}

function addgamepad(gamepad) {
  controllers[gamepad.index] = gamepad;
  rAF(updateStatus);
}

function disconnecthandler(e) {
  removegamepad(e.gamepad);
}

function removegamepad(gamepad) {
  delete controllers[gamepad.index];
}

function updateStatus() {
  scangamepads();
  if(wsconn.readyState == 1) {
    for (j in controllers) {
      var controller = controllers[j];
      //format must be ="frm%08X%08X%08X%08X%02X-";
      var strController = buildStringToSend(controller);
  
      if(strControllerPrev != strController)
      {
        wsconn.send(strController);        
        strControllerPrev = strController;
      }
      var d = document.getElementById("testDiv");
      var p = document.createElement("pre");
      p.textContent = strController + Date.now();
      d.innerHTML = p.outerHTML;
  
      //var d2 = document.getElementById("testDiv2");
      //var p2 = document.createElement("pre");
      //p2.textContent = buildStringAllControls(controller) + Date.now();
      //d2.innerHTML = p2.outerHTML;
    }
  }
rAF(updateStatus);
}

function buildStringToSend(controller) {
  var strController = "frm";
  for (var i = 0; i < 4; i++) {
    var n = controller.axes[i] * 32767;
    n = n >>> 0; // to make the number uint32
    strController += ("00000000" + n.toString(16).toUpperCase()).slice(-8);
  }
  strController += ("00" + controller.buttons[5].value).slice(-2); // fixed to 'R1' - index = 5
  strController += "-"; // fixed to 'A' for now
  return strController;
}

function buildStringAllControls(controller) {
  var strController = "all";
  strController += "-";
  strController += ("00" + controller.axes.length.toString(16).toUpperCase()).slice(-2);
  strController += "-";
  strController += ("00" + controller.buttons.length.toString(16).toUpperCase()).slice(-2);
  strController += "-";
    
  for (var i = 0; i < controller.axes.length; i++) {
    var n = controller.axes[i] * 32767;
    n = n >>> 0; // to make the number uint32
    strController += ("00000000" + n.toString(16).toUpperCase()).slice(-8);
  }
  strController += "\n";
  for (var i = 0; i < controller.buttons.length; i++) {
    //strController += ("00" + i).slice(-2) + ":";
    strController += ("00" + controller.buttons[i].value).slice(-2);
  }
  strController += "-"; // fixed to 'A' for now
  return strController;
}

function scangamepads() {
  var gamepads = navigator.getGamepads ?
    navigator.getGamepads() :
    navigator.webkitGetGamepads ? navigator.webkitGetGamepads() : [];
  for (var i = 0; i < gamepads.length; i++) {
    if (gamepads[i]) {
      if (!(gamepads[i].index in controllers)) {
        addgamepad(gamepads[i]);
      } else {
        controllers[gamepads[i].index] = gamepads[i];
      }
    }
  }
}

if (haveEvents) {
  window.addEventListener("gamepadconnected", connecthandler);
  window.addEventListener("gamepaddisconnected", disconnecthandler);
} else {
  setInterval(scangamepads, 500);
}