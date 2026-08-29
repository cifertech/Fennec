#include "WebRemote.h"

#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <string.h>

#include "Config.h"

namespace fennec {

static const char WEB_INDEX[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover"/>
<meta name="apple-mobile-web-app-capable" content="yes"/>
<meta name="theme-color" content="#0A0906"/>
<title>Fennec</title>
<style>
:root{--bg:#0A0906;--sand:#E7C98A;--line:#C9A468;--muted:#8A7A56;--dim:#3A3226;--ink:#1A1610}
*{box-sizing:border-box}
html,body{margin:0;background:var(--bg);color:var(--sand);font-family:ui-sans-serif,system-ui,sans-serif;min-height:100%}
body{padding:12px 14px 32px}
.wrap{max-width:560px;margin:0 auto}
.head{display:flex;justify-content:space-between;align-items:baseline;letter-spacing:.28em;font-size:11px}
.head b{font-weight:600}
.ip,.clock{letter-spacing:.08em;color:var(--muted);font-size:11px}
.dial{margin:10px 0 4px}
.freq{font-variant-numeric:tabular-nums;font-size:44px;line-height:1;letter-spacing:.02em}
.meta{display:flex;justify-content:space-between;gap:8px;color:var(--muted);font-size:12px;letter-spacing:.12em;margin-top:6px}
nav{display:flex;gap:6px;margin:14px 0 16px}
nav button{flex:1;background:transparent;border:1px solid var(--dim);color:var(--muted);border-radius:0;padding:9px 2px;font-size:10px;letter-spacing:.12em}
nav button.on{border-color:var(--sand);color:var(--sand)}
.row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap}
button.k{flex:1;min-width:56px;background:transparent;color:var(--sand);border:1px solid var(--line);padding:12px 8px;font-size:13px;letter-spacing:.1em}
button.k:active,button.k.on{background:var(--sand);color:var(--ink)}
.meter{height:8px;background:var(--dim);margin-top:5px}
.meter>i{display:block;height:100%;background:var(--sand);width:0}
label{display:block;font-size:11px;color:var(--muted);letter-spacing:.16em;margin-top:12px}
input[type=number],input[type=text]{width:100%;margin-top:8px;padding:10px;border:1px solid var(--dim);background:transparent;color:var(--sand);font-size:16px}
input[type=range]{width:100%;margin-top:8px;accent-color:var(--sand)}
.panel{display:none}
.panel.on{display:block}
.pic{width:100%;aspect-ratio:224/176;background:#000;border:1px solid var(--dim);object-fit:contain;image-rendering:pixelated}
.cw{min-height:160px;border:1px solid var(--dim);padding:12px;font-family:ui-monospace,monospace;font-size:16px;line-height:1.5;white-space:pre-wrap;word-break:break-word}
canvas{width:100%;height:96px;border:1px solid var(--dim);display:block;background:#000}
.log{font-size:12px;letter-spacing:.04em}
.log div{border-bottom:1px solid var(--dim);padding:8px 0;color:var(--muted)}
.log b{color:var(--sand);font-weight:500}
.hint{color:var(--muted);font-size:11px;letter-spacing:.08em;margin-top:10px}
</style>
</head>
<body>
<div class="wrap">
  <div class="head"><b>FENNEC</b><span class="clock" id="clock"></span><span class="ip" id="ip"></span></div>
  <div class="dial">
    <div class="freq" id="freq">--</div>
    <div class="meta"><span id="band">-</span><span id="rds"></span><span id="kind"></span></div>
  </div>
  <nav>
    <button id="n-tuner" class="on" onclick="view('tuner')">RADIO</button>
    <button id="n-sstv" onclick="view('sstv')">SSTV</button>
    <button id="n-cw" onclick="view('cw')">MORSE</button>
    <button id="n-hunt" onclick="view('hunt')">HOMING</button>
    <button id="n-log" onclick="view('log')">WATCH</button>
  </nav>

  <section id="p-tuner" class="panel on">
    <div class="row">
      <button class="k" id="k-fm" onclick="api('/api/band?kind=0')">FM</button>
      <button class="k" id="k-am" onclick="api('/api/band?kind=1')">AM</button>
      <button class="k" id="k-sw" onclick="api('/api/band?kind=2')">SW</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/tune?dir=-1')">TUNE −</button>
      <button class="k" onclick="api('/api/seek?dir=1')">SEEK</button>
      <button class="k" onclick="api('/api/tune?dir=1')">TUNE +</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/band?dir=-1')">BAND −</button>
      <button class="k" onclick="api('/api/band?dir=1')">BAND +</button>
    </div>
    <form class="row" onsubmit="go(event)">
      <input id="f" type="number" step="any" placeholder="103.5 or 14230"/>
      <button class="k" type="submit">GO</button>
    </form>
    <label>RSSI <span id="rssi">0</span></label>
    <div class="meter"><i id="brssi"></i></div>
    <label>SNR <span id="snr">0</span></label>
    <div class="meter"><i id="bsnr"></i></div>
    <label>VOLUME <span id="vol">0</span></label>
    <input id="vslide" type="range" min="0" max="63" value="40" onchange="api('/api/volume?v='+this.value)"/>
    <div class="row">
      <button class="k" onclick="api('/api/volume?dir=-1')">VOL −</button>
      <button class="k" id="mute" onclick="api('/api/mute')">MUTE</button>
      <button class="k" onclick="api('/api/volume?dir=1')">VOL +</button>
    </div>
  </section>

  <section id="p-sstv" class="panel">
    <img class="pic" id="pic" alt="SSTV"/>
    <div class="meta"><span id="sstvMode">AUTO</span><span id="sstvProg">—</span><span id="sstvSrc">RADIO</span></div>
    <label>LEVEL</label>
    <div class="meter"><i id="sstvPeak"></i></div>
    <div class="row">
      <button class="k" onclick="api('/api/sstv?reset=1')">RESET</button>
      <button class="k" id="k-mic0" onclick="api('/api/sstv?mic=0')">RADIO</button>
      <button class="k" id="k-mic1" onclick="api('/api/sstv?mic=1')">MIC</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/sstv?lock=1')">MODE</button>
      <button class="k" id="k-usb" onclick="api('/api/usb')">USB/LSB</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/bfo?dir=-1')">BFO −</button>
      <button class="k" onclick="api('/api/bfo?dir=1')">BFO +</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/tune?f=3733')">3.733</button>
      <button class="k" onclick="api('/api/tune?f=7171')">7.171</button>
      <button class="k" onclick="api('/api/tune?f=14230')">14.230</button>
      <button class="k" onclick="api('/api/tune?f=21340')">21.340</button>
    </div>
    <div class="hint" id="sstvHint">Picture is decoded on the radio and drawn here as it arrives.</div>
  </section>

  <section id="p-cw" class="panel">
    <div class="meta"><span id="cwMeta">USB</span><span id="cwPitch">700 Hz</span><span id="cwWpm">— WPM</span></div>
    <div class="cw" id="cwText"> </div>
    <div class="row">
      <button class="k" onclick="api('/api/cw?pitch=-1')">PIT −</button>
      <button class="k" id="k-cwusb" onclick="api('/api/usb')">USB/LSB</button>
      <button class="k" onclick="api('/api/cw?pitch=1')">PIT +</button>
    </div>
    <div class="row">
      <button class="k" id="k-cwmic0" onclick="api('/api/cw?mic=0')">RADIO</button>
      <button class="k" id="k-cwmic1" onclick="api('/api/cw?mic=1')">MIC</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/bfo?dir=-1')">BFO −</button>
      <button class="k" onclick="api('/api/bfo?dir=1')">BFO +</button>
    </div>
    <div class="hint" id="cwHint">Copy from the radio ADC, or from the MEMS mic.</div>
  </section>

  <section id="p-hunt" class="panel">
    <div class="meta"><span id="huntRssi">RSSI 0</span><span id="huntSnr">SNR 0</span><span id="huntPeak">PEAK 0</span></div>
    <canvas id="huntPlot" width="400" height="96"></canvas>
    <div class="row">
      <button class="k" onclick="api('/api/hunt?peak=1')">PEAK</button>
    </div>
    <div class="hint">Walk the antenna. The plot is live RSSI from the SI4732.</div>
  </section>

  <section id="p-log" class="panel">
    <div class="meta"><span id="logArm">IDLE</span><span id="logThr">THR —</span><span id="logN">0</span></div>
    <div class="row">
      <button class="k" id="armBtn" onclick="api('/api/log?arm=1','POST')">ARM</button>
      <button class="k" onclick="api('/api/log?clr=1','POST')">CLR</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/log?thr=-1','POST')">THR −</button>
      <button class="k" onclick="api('/api/log?thr=1','POST')">THR +</button>
    </div>
    <div class="row">
      <button class="k" onclick="api('/api/log?preset=0','POST')">UVB</button>
      <button class="k" onclick="api('/api/log?preset=1','POST')">S06</button>
      <button class="k" onclick="api('/api/log?preset=2','POST')">E07</button>
      <button class="k" onclick="api('/api/log?preset=3','POST')">HM01</button>
      <button class="k" onclick="api('/api/log?preset=4','POST')">HERE</button>
    </div>
    <div class="log" id="logList"></div>
  </section>
</div>
<script>
const LOCKS=['AUTO','MARTIN 1','MARTIN 2','SCOTTIE 1','SCOTTIE 2','ROBOT 36'];
let page='tuner', lastGen=-1, lastMode='', navLock=0;
function show(v){
  page=v;
  document.querySelectorAll('nav button').forEach(b=>b.classList.toggle('on', b.id==='n-'+v));
  document.querySelectorAll('.panel').forEach(p=>p.classList.toggle('on', p.id==='p-'+v));
}
function view(v){
  if(page===v && v!=='tuner'){tick();return;}
  show(v);
  navLock=Date.now()+1600;
  if(v==='tuner') api('/api/mode?m=tuner');
  else if(v==='sstv') api('/api/sstv');
  else if(v==='cw') api('/api/cw');
  else if(v==='hunt') api('/api/mode?m=hunt');
  else if(v==='log') api('/api/log','POST');
}
async function api(url, method){
  await fetch(url,{method:method||'POST'});
  tick();
}
function go(e){e.preventDefault();const v=document.getElementById('f').value;if(v)api('/api/tune?f='+encodeURIComponent(v))}
function drawHunt(hist,peak){
  const c=document.getElementById('huntPlot'),x=c.getContext('2d');
  const w=c.width,h=c.height;
  x.fillStyle='#000';x.fillRect(0,0,w,h);
  if(!hist||!hist.length)return;
  x.strokeStyle='#3A3226';
  for(let g=0;g<=4;g++){const y=h-4-(g/4)*(h-8);x.beginPath();x.moveTo(0,y);x.lineTo(w,y);x.stroke();}
  x.strokeStyle='#E7C98A';x.beginPath();
  hist.forEach((v,i)=>{
    const px=i*(w-2)/(hist.length-1||1)+1;
    const py=h-2-(Math.min(80,v)/80)*(h-6);
    i?x.lineTo(px,py):x.moveTo(px,py);
  });
  x.stroke();
  if(peak>0){
    const py=h-2-(Math.min(80,peak)/80)*(h-6);
    x.strokeStyle='#8A7A56';x.beginPath();x.moveTo(0,py);x.lineTo(w,py);x.stroke();
  }
}
async function tick(){
  const s=await (await fetch('/api/status')).json();
  document.getElementById('freq').textContent=s.display;
  document.getElementById('band').textContent=(s.band||'')+(s.title?' · '+s.title:'');
  document.getElementById('rds').textContent=s.rds||'';
  document.getElementById('kind').textContent=(s.usb&&s.ssb?'USB':s.ssb?'LSB':s.kind)+(s.muted?' MUTE':'');
  document.getElementById('rssi').textContent=s.rssi;
  document.getElementById('snr').textContent=s.snr;
  document.getElementById('vol').textContent=s.volume;
  document.getElementById('ip').textContent=s.ip||'';
  document.getElementById('clock').textContent=s.clock||'';
  document.getElementById('brssi').style.width=Math.min(100,s.rssi*1.4)+'%';
  document.getElementById('bsnr').style.width=Math.min(100,s.snr*3)+'%';
  document.getElementById('mute').classList.toggle('on', !!s.muted);
  document.getElementById('k-fm').classList.toggle('on', s.kind==='FM');
  document.getElementById('k-am').classList.toggle('on', s.kind==='AM');
  document.getElementById('k-sw').classList.toggle('on', s.kind==='SW');
  const slide=document.getElementById('vslide');
  if(document.activeElement!==slide) slide.value=s.volume;
  const lock=LOCKS[s.sstvLock]||'AUTO';
  const live=LOCKS[s.sstvMode]||lock;
  document.getElementById('sstvMode').textContent=live;
  document.getElementById('sstvSrc').textContent=s.sstvMic?'MIC':'RADIO';
  document.getElementById('sstvProg').textContent=s.sstvLines?(s.sstvLine+' / '+s.sstvLines):('BFO '+(s.bfo||0));
  document.getElementById('sstvPeak').style.width=Math.min(100,(s.sstvPeak||0)/8)+'%';
  document.getElementById('sstvHint').textContent=(s.sstvMic?'Speaker muted. ':'')+lock+(s.usb?' · USB':' · LSB')+' · '+s.display;
  document.getElementById('k-mic0').classList.toggle('on', !s.sstvMic);
  document.getElementById('k-mic1').classList.toggle('on', !!s.sstvMic);
  document.getElementById('k-usb').classList.toggle('on', !!s.usb);
  document.getElementById('k-cwusb').classList.toggle('on', !!s.usb);
  if(s.sstvGen!==lastGen){
    lastGen=s.sstvGen;
    document.getElementById('pic').src='/api/sstv.bmp?g='+s.sstvGen;
  }
  document.getElementById('cwMeta').textContent=s.usb?'USB':'LSB';
  document.getElementById('cwPitch').textContent=(s.cwPitch||0)+' Hz';
  document.getElementById('cwWpm').textContent=(s.cwWpm||0)+' WPM';
  document.getElementById('cwText').textContent=s.cw||' ';
  document.getElementById('k-cwmic0').classList.toggle('on', !s.cwMic);
  document.getElementById('k-cwmic1').classList.toggle('on', !!s.cwMic);
  document.getElementById('cwHint').textContent=s.cwMic?'Speaker muted. Hold the mic to the tone.':'Copy from the radio ADC.';
  document.getElementById('huntRssi').textContent='RSSI '+s.rssi;
  document.getElementById('huntSnr').textContent='SNR '+s.snr;
  document.getElementById('huntPeak').textContent='PEAK '+s.huntPeak;
  document.getElementById('logArm').textContent=s.logArmed?'ARMED':'IDLE';
  document.getElementById('logThr').textContent='THR '+(s.logThr||0);
  document.getElementById('logN').textContent=(s.logCount||0)+' EV';
  document.getElementById('armBtn').textContent=s.logArmed?'DISARM':'ARM';
  document.getElementById('armBtn').onclick=()=>api('/api/log?arm='+(s.logArmed?0:1),'POST');
  if(Date.now()>navLock && s.mode && s.mode!==page){
    show(s.mode);
  }
  if(page==='hunt'){
    const h=await (await fetch('/api/hunt')).json();
    drawHunt(h.hist,h.peak);
  }
  if(page==='log'){
    const L=await (await fetch('/api/log')).json();
    const box=document.getElementById('logList');
    box.innerHTML=(L.events||[]).map(e=>'<div><b>'+e.when+'</b>  '+e.freq+' kHz  rssi '+e.rssi+(e.cw?'  '+e.cw:'')+'</div>').join('')||'<div>No events yet.</div>';
  }
  lastMode=s.mode;
}
tick();setInterval(tick,700);
</script>
</body></html>
)HTML";


namespace {
WebServer server(80);

int argInt(const char* name, int fallback) {
  if (!server.hasArg(name)) {
    return fallback;
  }
  return server.arg(name).toInt();
}

float argFloat(const char* name, float fallback) {
  if (!server.hasArg(name)) {
    return fallback;
  }
  return server.arg(name).toFloat();
}

void sendOk() { server.send(200, "application/json", "{\"ok\":true}"); }
}

void WebRemote::begin() {
  WiFi.persistent(false);
  WiFi.setHostname(FENNEC_HOSTNAME);

#if FENNEC_WIFI_ENABLE_AP
  if (strlen(FENNEC_WIFI_STA_SSID) > 0) {
    WiFi.mode(WIFI_AP_STA);
  } else {
    WiFi.mode(WIFI_AP);
  }
  if (strlen(FENNEC_WIFI_AP_PASS) >= 8) {
    WiFi.softAP(FENNEC_WIFI_AP_SSID, FENNEC_WIFI_AP_PASS, FENNEC_WIFI_AP_CHANNEL);
  } else {
    WiFi.softAP(FENNEC_WIFI_AP_SSID, nullptr, FENNEC_WIFI_AP_CHANNEL);
  }
  apOn_ = true;
#else
  WiFi.mode(strlen(FENNEC_WIFI_STA_SSID) > 0 ? WIFI_STA : WIFI_OFF);
  apOn_ = false;
#endif

  if (strlen(FENNEC_WIFI_STA_SSID) > 0) {
    WiFi.begin(FENNEC_WIFI_STA_SSID, FENNEC_WIFI_STA_PASS);
  }

  server.on("/", HTTP_GET, [this]() { handleIndex(); });
  server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server.on("/api/tune", HTTP_ANY, [this]() { handleTune(); });
  server.on("/api/band", HTTP_ANY, [this]() { handleBand(); });
  server.on("/api/seek", HTTP_ANY, [this]() { handleSeek(); });
  server.on("/api/volume", HTTP_ANY, [this]() { handleVolume(); });
  server.on("/api/mute", HTTP_ANY, [this]() { handleMute(); });
  server.on("/api/sstv", HTTP_ANY, [this]() { handleSstv(); });
  server.on("/api/sstv.bmp", HTTP_GET, [this]() { handleSstvBmp(); });
  server.on("/api/mode", HTTP_ANY, [this]() { handleMode(); });
  server.on("/api/usb", HTTP_ANY, [this]() { handleUsb(); });
  server.on("/api/bfo", HTTP_ANY, [this]() { handleBfo(); });
  server.on("/api/cw", HTTP_ANY, [this]() { handleCw(); });
  server.on("/api/hunt", HTTP_ANY, [this]() { handleHunt(); });
  server.on("/api/log", HTTP_ANY, [this]() { handleLog(); });
  server.begin();

  ready_ = true;
  refreshIp();
  Serial.printf("[wifi] AP \"%s\"  http://%s/\n", FENNEC_WIFI_AP_SSID, ip_);
}

void WebRemote::setApEnabled(bool on) {
  if (on == apOn_) {
    return;
  }
  if (on) {
#if FENNEC_WIFI_ENABLE_AP
    if (strlen(FENNEC_WIFI_STA_SSID) > 0) {
      WiFi.mode(WIFI_AP_STA);
    } else {
      WiFi.mode(WIFI_AP);
    }
    if (strlen(FENNEC_WIFI_AP_PASS) >= 8) {
      WiFi.softAP(FENNEC_WIFI_AP_SSID, FENNEC_WIFI_AP_PASS, FENNEC_WIFI_AP_CHANNEL);
    } else {
      WiFi.softAP(FENNEC_WIFI_AP_SSID, nullptr, FENNEC_WIFI_AP_CHANNEL);
    }
    apOn_ = true;
    refreshIp();
    Serial.printf("[wifi] AP \"%s\"  http://%s/\n", FENNEC_WIFI_AP_SSID, ip_);
#endif
    return;
  }
  WiFi.softAPdisconnect(true);
  apOn_ = false;
  if (strlen(FENNEC_WIFI_STA_SSID) > 0) {
    WiFi.mode(WIFI_STA);
  } else {
    WiFi.mode(WIFI_OFF);
  }
  snprintf(ip_, sizeof(ip_), "0.0.0.0");
  Serial.println(F("[wifi] AP off"));
}

void WebRemote::poll() {
  if (!ready_) {
    return;
  }
  server.handleClient();
  const bool sta = WiFi.status() == WL_CONNECTED;
  if (sta != staUp_) {
    staUp_ = sta;
    refreshIp();
    if (sta) {
      Serial.printf("[wifi] STA %s\n", WiFi.localIP().toString().c_str());
    }
  }
}

void WebRemote::setSnapshot(const RadioSnapshot& snap) {
  snap_ = snap;
}

void WebRemote::setLogJson(const char* json) {
  if (json == nullptr) {
    return;
  }
  strncpy(logJson_, json, sizeof(logJson_) - 1);
  logJson_[sizeof(logJson_) - 1] = 0;
}

void WebRemote::setSstvFrame(const uint16_t* px, uint16_t w, uint16_t h) {
  sstvPx_ = px;
  sstvW_ = w;
  sstvH_ = h;
}

void WebRemote::setHuntHist(const uint8_t* hist, uint8_t n, uint8_t head) {
  huntHist_ = hist;
  huntN_ = n;
  huntHead_ = head;
}

bool WebRemote::take(RemoteMsg& msg) {
  if (pending_.cmd == RemoteCmd::None) {
    return false;
  }
  msg = pending_;
  pending_ = {};
  return true;
}

void WebRemote::handleIndex() { server.send_P(200, "text/html", WEB_INDEX); }

void WebRemote::handleStatus() {
  const Band& b = bandAt(snap_.bandIndex);
  char display[16];
  const char* kind = "SW";
  if (snap_.sstv || snap_.ssb || strcmp(snap_.mode, "cw") == 0 || strcmp(snap_.mode, "log") == 0) {
    snprintf(display, sizeof(display), "%u", snap_.frequency);
    if (strcmp(snap_.mode, "cw") == 0) {
      kind = "MORSE";
    } else if (strcmp(snap_.mode, "log") == 0) {
      kind = "WATCH";
    } else if (strcmp(snap_.mode, "hunt") == 0) {
      kind = "HOMING";
    } else {
      kind = snap_.frequency >= 10000 ? "USB" : "SSB";
    }
  } else if (strcmp(snap_.mode, "hunt") == 0) {
    if (b.kind == BandKind::FM) {
      snprintf(display, sizeof(display), "%u.%02u", snap_.frequency / 100, snap_.frequency % 100);
    } else {
      snprintf(display, sizeof(display), "%u", snap_.frequency);
    }
    kind = "HOMING";
  } else if (b.kind == BandKind::FM) {
    snprintf(display, sizeof(display), "%u.%02u", snap_.frequency / 100, snap_.frequency % 100);
    kind = "FM";
  } else {
    snprintf(display, sizeof(display), "%u", snap_.frequency);
    kind = b.kind == BandKind::AM ? "AM" : "SW";
  }

  char json[1280];
  snprintf(json, sizeof(json),
           "{\"ok\":true,\"present\":%s,\"freq\":%u,\"display\":\"%s\",\"band\":\"%s\","
           "\"title\":\"%s\",\"bandIndex\":%u,\"kind\":\"%s\",\"volume\":%u,\"muted\":%s,"
           "\"stereo\":%s,\"rssi\":%u,\"snr\":%u,\"rds\":\"%s\",\"ip\":\"%s\","
           "\"sstv\":%s,\"ssb\":%s,\"usb\":%s,\"bfo\":%d,\"sstvMic\":%s,\"sstvMode\":%u,"
           "\"sstvLock\":%u,\"sstvLine\":%u,\"sstvLines\":%u,\"sstvVis\":%u,\"sstvPeak\":%u,"
           "\"sstvTone\":%u,\"sstvGen\":%u,\"cwPitch\":%u,\"cwWpm\":%u,\"cwMic\":%s,\"cw\":\"%s\","
           "\"huntPeak\":%u,\"logArmed\":%s,\"logThr\":%u,\"logCount\":%u,\"clock\":\"%s\","
           "\"mode\":\"%s\"}",
           snap_.present ? "true" : "false", snap_.frequency, display, b.name, b.title, snap_.bandIndex,
           kind, snap_.volume, snap_.muted ? "true" : "false", snap_.stereo ? "true" : "false", snap_.rssi,
           snap_.snr, snap_.rds, ip_,
           snap_.sstv ? "true" : "false", snap_.ssb ? "true" : "false", snap_.usb ? "true" : "false",
           static_cast<int>(snap_.bfo), snap_.sstvMic ? "true" : "false", snap_.sstvMode, snap_.sstvLock,
           snap_.sstvLine, snap_.sstvLines, snap_.sstvVis, snap_.sstvPeak, snap_.sstvTone, snap_.sstvGen,
           snap_.cwPitch, snap_.cwWpm, snap_.cwMic ? "true" : "false", snap_.cw, snap_.huntPeak, snap_.logArmed ? "true" : "false",
           snap_.logThr, snap_.logCount, snap_.clock, snap_.mode);
  server.send(200, "application/json", json);
}

void WebRemote::handleTune() {
  if (server.hasArg("f")) {
    const float f = argFloat("f", 0);
    uint8_t band = snap_.bandIndex;
    uint16_t freq = 0;
    if (!resolveTune(f, snap_.bandIndex, &band, &freq)) {
      sendOk();
      return;
    }
    queue(RemoteCmd::SetFreq, (static_cast<int32_t>(band) << 16) | freq);
  } else if (argInt("dir", 1) >= 0) {
    queue(RemoteCmd::TuneUp);
  } else {
    queue(RemoteCmd::TuneDown);
  }
  sendOk();
}

void WebRemote::handleBand() {
  if (server.hasArg("kind")) {
    queue(RemoteCmd::SetKind, argInt("kind", 0));
  } else if (server.hasArg("id")) {
    queue(RemoteCmd::SetBand, argInt("id", 0));
  } else if (argInt("dir", 1) >= 0) {
    queue(RemoteCmd::BandNext);
  } else {
    queue(RemoteCmd::BandPrev);
  }
  sendOk();
}

void WebRemote::handleSeek() {
  queue(argInt("dir", 1) >= 0 ? RemoteCmd::SeekUp : RemoteCmd::SeekDown);
  sendOk();
}

void WebRemote::handleVolume() {
  if (server.hasArg("v")) {
    queue(RemoteCmd::SetVol, argInt("v", 40));
  } else {
    queue(RemoteCmd::VolDelta, argInt("dir", 1) >= 0 ? 2 : -2);
  }
  sendOk();
}

void WebRemote::handleMute() {
  queue(RemoteCmd::MuteToggle);
  sendOk();
}

void WebRemote::handleSstv() {
  if (server.hasArg("mic")) {
    queue(RemoteCmd::SstvMic, argInt("mic", 1));
  } else if (server.hasArg("lock")) {
    queue(RemoteCmd::SstvCycleMode);
  } else if (server.hasArg("reset")) {
    queue(RemoteCmd::SstvReset);
  } else {
    queue(RemoteCmd::SetMode, 1);
  }
  sendOk();
}

void WebRemote::handleSstvBmp() {
  const uint16_t w = sstvW_ > 0 ? sstvW_ : 224;
  const uint16_t h = sstvH_ > 0 ? sstvH_ : 176;
  const uint32_t rowBytes = static_cast<uint32_t>(w) * 3;
  const uint32_t imgBytes = rowBytes * h;
  const uint32_t fileBytes = 54 + imgBytes;
  uint8_t hdr[54] = {};
  hdr[0] = 'B';
  hdr[1] = 'M';
  hdr[2] = static_cast<uint8_t>(fileBytes);
  hdr[3] = static_cast<uint8_t>(fileBytes >> 8);
  hdr[4] = static_cast<uint8_t>(fileBytes >> 16);
  hdr[5] = static_cast<uint8_t>(fileBytes >> 24);
  hdr[10] = 54;
  hdr[14] = 40;
  hdr[18] = static_cast<uint8_t>(w);
  hdr[19] = static_cast<uint8_t>(w >> 8);
  hdr[22] = static_cast<uint8_t>(h);
  hdr[23] = static_cast<uint8_t>(h >> 8);
  hdr[26] = 1;
  hdr[28] = 24;
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(fileBytes);
  server.send(200, "image/bmp", "");
  WiFiClient client = server.client();
  client.write(hdr, sizeof(hdr));
  uint8_t row[768];
  for (int y = static_cast<int>(h) - 1; y >= 0; --y) {
    for (uint16_t x = 0; x < w; ++x) {
      const uint16_t c = (sstvPx_ != nullptr) ? sstvPx_[static_cast<uint32_t>(y) * w + x] : 0;
      const uint8_t r = static_cast<uint8_t>(((c >> 11) & 31) * 255 / 31);
      const uint8_t g = static_cast<uint8_t>(((c >> 5) & 63) * 255 / 63);
      const uint8_t b = static_cast<uint8_t>((c & 31) * 255 / 31);
      row[x * 3 + 0] = b;
      row[x * 3 + 1] = g;
      row[x * 3 + 2] = r;
    }
    client.write(row, rowBytes);
    yield();
  }
}

void WebRemote::handleMode() {
  String m = server.hasArg("m") ? server.arg("m") : "";
  m.toLowerCase();
  int32_t a = 0;
  if (m == "sstv" || m == "tv") {
    a = 1;
  } else if (m == "cw") {
    a = 2;
  } else if (m == "hunt") {
    a = 3;
  } else if (m == "log") {
    a = 4;
  }
  queue(RemoteCmd::SetMode, a);
  sendOk();
}

void WebRemote::handleUsb() {
  queue(RemoteCmd::UsbToggle);
  sendOk();
}

void WebRemote::handleBfo() {
  queue(RemoteCmd::BfoDelta, argInt("dir", 1) >= 0 ? 20 : -20);
  sendOk();
}

void WebRemote::handleCw() {
  if (server.hasArg("mic")) {
    queue(RemoteCmd::CwMic, argInt("mic", 1));
  } else if (server.hasArg("pitch")) {
    queue(RemoteCmd::CwPitchDelta, argInt("pitch", 1) >= 0 ? 50 : -50);
  } else {
    queue(RemoteCmd::SetMode, 2);
  }
  sendOk();
}

void WebRemote::handleHunt() {
  if (server.hasArg("peak")) {
    queue(RemoteCmd::HuntPeak);
    sendOk();
    return;
  }
  char json[1400];
  int n = snprintf(json, sizeof(json), "{\"ok\":true,\"peak\":%u,\"n\":%u,\"hist\":[", snap_.huntPeak,
                   huntN_);
  for (uint8_t i = 0; i < huntN_ && n > 0 && static_cast<size_t>(n) + 6 < sizeof(json); ++i) {
    const uint8_t idx = static_cast<uint8_t>((huntHead_ + i) % (huntN_ == 0 ? 1 : huntN_));
    const uint8_t v = huntHist_ != nullptr ? huntHist_[idx] : 0;
    n += snprintf(json + n, sizeof(json) - static_cast<size_t>(n), "%s%u", i ? "," : "", v);
  }
  if (n > 0 && static_cast<size_t>(n) + 3 < sizeof(json)) {
    snprintf(json + n, sizeof(json) - static_cast<size_t>(n), "]}");
  }
  server.send(200, "application/json", json);
}

void WebRemote::handleLog() {
  if (server.method() != HTTP_GET) {
    if (server.hasArg("arm")) {
      queue(RemoteCmd::LogArm, argInt("arm", 1));
    } else if (server.hasArg("clr")) {
      queue(RemoteCmd::LogClear);
    } else if (server.hasArg("thr")) {
      queue(RemoteCmd::LogThrDelta, argInt("thr", 1) >= 0 ? 1 : -1);
    } else if (server.hasArg("preset")) {
      queue(RemoteCmd::LogPreset, argInt("preset", 0));
    } else {
      queue(RemoteCmd::SetMode, 4);
    }
    sendOk();
    return;
  }
  server.send(200, "application/json", logJson_);
}

void WebRemote::queue(RemoteCmd cmd, int32_t arg) {
  pending_.cmd = cmd;
  pending_.arg = arg;
}

void WebRemote::refreshIp() {
  const IPAddress a = staUp_ ? WiFi.localIP() : WiFi.softAPIP();
  snprintf(ip_, sizeof(ip_), "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
}

}
