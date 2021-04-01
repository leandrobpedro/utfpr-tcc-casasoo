/** 
 * Project  : Casasoo
 * File     : script.js
 * Version  : 1.0.0
 * Date     : 2015-07-13
 * Author   : leandrobpedro@gmail.com
 */

window.onload = function () {
  // Update user interface.
  setInterval(function () { sendRequest('?cmd=refresh', updateUi) }, 2000);

  // Servo slider.
  range.addEventListener('mouseup', function () {
    sendRequest('?cmd=servo&value=' + this.value);
  });

  // Toggle.
  toggle.addEventListener('click', function () {
    sendRequest('?cmd=toggle&value=7', null);
  });

  celsius.addEventListener('click', function () {
    setCookie('unit', 'celsius');
  });

  fahrenheit.addEventListener('click', function () {
    setCookie('unit', 'fahrenheit');
  });

  // For test porpouses.
  // this.button_foo.addEventListener('click', function () {});
};


function setCookie(cname, cvalue, exdays) {
  var d = new Date();
  d.setTime(d.getTime() + (exdays * 24 * 3600 *1000));
  var expires = "expires=" + d.toUTCString();
  document.cookie = cname + "=" + cvalue + "; " + expires;
}

function getCookie(cname) {
  var name = cname + "=";
  var ca = document.cookie.split(';');
  for(var i = 0; i < ca.length; i++) {
    var c = ca[i];
    while (c.charAt(0)==' ') c = c.substring(1);
    if (c.indexOf(name) == 0) return c.substring(name.length, c.length);
  }
  return "";
}

function sendRequest(url, callback, postData) {
  var xhr = new XMLHttpRequest();
  if (!xhr) return;
  var method = (postData) ? "POST" : "GET";
  xhr.onreadystatechange = function() {
    if (xhr.readyState != 4) return;
    if (xhr.status != 200 && xhr.status != 304) return;
    if (callback) callback(xhr);
  };
  if (xhr.readyState == 4) return;
  xhr.open(method, url, true);
  xhr.timeout = 4000;
  xhr.ontimeout = function() {
    xhr.abort();
  };
  xhr.send(postData);
}

function getParameterByName(req, param) {
  param = param.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
  var regex = new RegExp("[\\?&]" + param + "=([^&#]*)"),
      results = regex.exec(req);
  return results === null ? "" : decodeURIComponent(results[1].replace(/\+/g, " "));
}

function updateUi(req) {
  var response = req.responseText;


  var servo = getParameterByName(response, 'servo');
  var light = parseInt(getParameterByName(response, 'light'));
  var temp = parseFloat(getParameterByName(response, 'temp'));

  var x = document.getElementById('axis');
  var y = document.getElementById('light');
  var z = document.getElementById('temp');

  if (getCookie('unit') != 'celsius') {
    document.getElementById('celsius').style.color = "#ccc";
    document.getElementById('fahrenheit').style.color = "#ffab40";
    z.innerHTML = parseFloat(temp * (5 / 1023 * 100) * 1.8 + 32).toFixed(1) + " \xB0F";
  } else {
    document.getElementById('celsius').style.color = "#ffab40";
    document.getElementById('fahrenheit').style.color = "#ccc";
    z.innerHTML = parseFloat(temp * (5 / 1023 * 100)).toFixed(1) + " \xB0C";
  }

  x.style.transform = "rotate(" + servo + "deg)"; 
  light ? y.style["-webkit-filter"] = "none" : y.style["-webkit-filter"] = "grayscale(1)";
  
}