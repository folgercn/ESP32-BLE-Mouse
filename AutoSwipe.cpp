// AutoSwipe: implementation of auto swipe configuration, endpoints, and scheduler.
// Provides: config load/save, HTML/JSON handlers, and randomized swipe execution.
#include "AutoSwipe.h"

// Clamp integer to [minVal, maxVal]
int AutoSwipeManager::clampInt(int val, int minVal, int maxVal) {
    return min(max(val, minVal), maxVal);
}

// Random number around base with +/- spread
int AutoSwipeManager::randomAround(int base, int spread) {
    return base + random(-spread, spread + 1);
}

// Validate and bound config values
void AutoSwipeManager::normalizeConfig() {
    if (cfg.intervalMinSec < 1) cfg.intervalMinSec = 1;
    if (cfg.intervalMaxSec < cfg.intervalMinSec) cfg.intervalMaxSec = cfg.intervalMinSec;
    if (cfg.screenW <= 0) cfg.screenW = 1080;
    if (cfg.screenH <= 0) cfg.screenH = 2248;
    if (cfg.duration < 30) cfg.duration = 30;
    cfg.lengthPercent = clampInt(cfg.lengthPercent, 20, 200);
    cfg.lengthJitterPercent = clampInt(cfg.lengthJitterPercent, 0, 80);
    cfg.durationJitterPercent = clampInt(cfg.durationJitterPercent, 0, 80);
    cfg.delayJitterPercent = clampInt(cfg.delayJitterPercent, 0, 80);
    cfg.doubleTapProbPercent = clampInt(cfg.doubleTapProbPercent, 0, 100);
    cfg.doubleTapProbJitterPercent = clampInt(cfg.doubleTapProbJitterPercent, 0, 100);
    if (cfg.doubleTapIntervalMs < 40) cfg.doubleTapIntervalMs = 40;
    cfg.doubleTapIntervalJitterPercent = clampInt(cfg.doubleTapIntervalJitterPercent, 0, 200);
    if (cfg.doubleTapEdgeMinMs < 100) cfg.doubleTapEdgeMinMs = 100;
    if (cfg.doubleTapEdgeMaxMs < cfg.doubleTapEdgeMinMs + 50) cfg.doubleTapEdgeMaxMs = cfg.doubleTapEdgeMinMs + 50;
}

// Apply JSON fields (only English keys) into config
void AutoSwipeManager::applyJsonToConfig(JsonDocument& doc, AutoSwipeConfig& c) {
    if (doc.containsKey("enabled")) c.enabled = doc["enabled"];
    if (doc.containsKey("auto_start")) c.enabled = doc["auto_start"];

    if (doc.containsKey("x1")) c.x1 = doc["x1"];
    if (doc.containsKey("y1")) c.y1 = doc["y1"];
    if (doc.containsKey("x2")) c.x2 = doc["x2"];
    if (doc.containsKey("y2")) c.y2 = doc["y2"];
    if (doc.containsKey("duration")) c.duration = doc["duration"];
    if (doc.containsKey("screen_w")) c.screenW = doc["screen_w"];
    if (doc.containsKey("screen_h")) c.screenH = doc["screen_h"];
    if (doc.containsKey("delay_hover")) c.delayHover = doc["delay_hover"];
    if (doc.containsKey("delay_press")) c.delayPress = doc["delay_press"];
    if (doc.containsKey("delay_interval")) c.delayInterval = doc["delay_interval"];
    if (doc.containsKey("curve_strength")) c.curveStrength = doc["curve_strength"];
    if (doc.containsKey("double_check")) c.doubleCheck = doc["double_check"];
    
    if (doc.containsKey("interval_min_sec")) c.intervalMinSec = doc["interval_min_sec"];
    if (doc.containsKey("interval_max_sec")) c.intervalMaxSec = doc["interval_max_sec"];

    if (doc.containsKey("length_percent")) c.lengthPercent = doc["length_percent"];
    if (doc.containsKey("length_jitter_percent")) c.lengthJitterPercent = doc["length_jitter_percent"];
    if (doc.containsKey("duration_jitter_percent")) c.durationJitterPercent = doc["duration_jitter_percent"];
    if (doc.containsKey("delay_jitter_percent")) c.delayJitterPercent = doc["delay_jitter_percent"];
    if (doc.containsKey("double_tap_enabled")) c.doubleTapEnabled = doc["double_tap_enabled"];
    if (doc.containsKey("double_tap_prob_percent")) c.doubleTapProbPercent = doc["double_tap_prob_percent"];
    if (doc.containsKey("double_tap_prob_jitter_percent")) c.doubleTapProbJitterPercent = doc["double_tap_prob_jitter_percent"];
    if (doc.containsKey("double_tap_interval_ms")) c.doubleTapIntervalMs = doc["double_tap_interval_ms"];
    if (doc.containsKey("double_tap_interval_jitter_percent")) c.doubleTapIntervalJitterPercent = doc["double_tap_interval_jitter_percent"];
    if (doc.containsKey("double_tap_edge_min_ms")) c.doubleTapEdgeMinMs = doc["double_tap_edge_min_ms"];
    if (doc.containsKey("double_tap_edge_max_ms")) c.doubleTapEdgeMaxMs = doc["double_tap_edge_max_ms"];
}

// Load config from NVS (with defaults if missing/invalid)
void AutoSwipeManager::loadConfig() {
    pref.begin("auto_swipe", true);
    String raw = pref.getString("json", "");
    pref.end();

    if (raw.length() == 0) {
        normalizeConfig();
        return;
    }

    StaticJsonDocument<640> doc;
    if (!deserializeJson(doc, raw)) {
        applyJsonToConfig(doc, cfg);
    }
    normalizeConfig();
}

// Save config to NVS as JSON string
void AutoSwipeManager::saveConfig(const AutoSwipeConfig& c) {
    StaticJsonDocument<640> doc;
    doc["enabled"] = c.enabled;
    doc["x1"] = c.x1; doc["y1"] = c.y1;
    doc["x2"] = c.x2; doc["y2"] = c.y2;
    doc["duration"] = c.duration;
    doc["screen_w"] = c.screenW; doc["screen_h"] = c.screenH;
    doc["delay_hover"] = c.delayHover;
    doc["delay_press"] = c.delayPress;
    doc["delay_interval"] = c.delayInterval;
    doc["curve_strength"] = c.curveStrength;
    doc["double_check"] = c.doubleCheck;
    doc["interval_min_sec"] = c.intervalMinSec;
    doc["interval_max_sec"] = c.intervalMaxSec;
    doc["length_percent"] = c.lengthPercent;
    doc["length_jitter_percent"] = c.lengthJitterPercent;
    doc["duration_jitter_percent"] = c.durationJitterPercent;
    doc["delay_jitter_percent"] = c.delayJitterPercent;
    doc["double_tap_enabled"] = c.doubleTapEnabled;
    doc["double_tap_prob_percent"] = c.doubleTapProbPercent;
    doc["double_tap_prob_jitter_percent"] = c.doubleTapProbJitterPercent;
    doc["double_tap_interval_ms"] = c.doubleTapIntervalMs;
    doc["double_tap_interval_jitter_percent"] = c.doubleTapIntervalJitterPercent;
    doc["double_tap_edge_min_ms"] = c.doubleTapEdgeMinMs;
    doc["double_tap_edge_max_ms"] = c.doubleTapEdgeMaxMs;

    String out;
    serializeJson(doc, out);
    
    pref.begin("auto_swipe", false);
    pref.putString("json", out);
    pref.end();
}

// Render bilingual HTML form page
String AutoSwipeManager::renderPage(const AutoSwipeConfig& c, const String& message) {
    String html = "<!DOCTYPE html><html lang='zh-CN'><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<title>Auto Swipe 配置 / Auto Swipe Settings</title>"
                  "<style>body{font-family:-apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;"
                  "margin:24px;line-height:1.6;background:#0b1e2d;color:#e7f2ff;}"
                  "h1{font-size:22px;margin-bottom:8px;}form{background:rgba(255,255,255,0.05);"
                  "padding:16px;border-radius:12px;box-shadow:0 12px 30px rgba(0,0,0,0.35);}"
                  "fieldset{border:1px solid rgba(255,255,255,0.15);border-radius:10px;padding:12px;margin-top:12px;}"
                  "legend{padding:0 8px;color:#9cc9ff;font-weight:700;font-size:14px;}"
                  "label{display:block;margin-top:10px;font-weight:600;}input,button{width:100%;"
                  "padding:10px;margin-top:4px;border-radius:8px;border:1px solid rgba(255,255,255,0.15);"
                  "background:rgba(255,255,255,0.08);color:#e7f2ff;font-size:14px;}"
                  "button{background:#1f7aec;border:0;cursor:pointer;font-weight:700;margin-top:16px;}"
                  "button:hover{background:#2c8eff;}button.danger{background:#c63c3c;}button.danger:hover{background:#de4f4f;}"
                  "small{color:#99b5d6;} .msg{margin:8px 0;color:#5cf29c;}"
                  ".row{display:flex;gap:12px;} .row .col{flex:1;} .card{margin-top:16px;"
                  "padding:12px;border-radius:10px;background:rgba(255,255,255,0.03);} </style></head><body>";
    html += "<h1>自动上划 / Auto Swipe</h1>";
    if (message.length() > 0) html += "<div class='msg'>" + message + "</div>";
    html += "<form method='POST' action='/auto_swipe'>"
            "<fieldset><legend>开关 / Toggle</legend>"
            "<label>开机自动运行 / Auto start when WiFi+BLE OK"
            "<input type='checkbox' name='enabled' value='1' ";
    if (c.enabled) html += "checked";
    html += "></label></fieldset>";

    html += "<fieldset><legend>区域与屏幕 / Area & Screen</legend>"
            "<div class='row'><div class='col'><label>起点 X1 (Start X1)<input type='number' name='x1' value='" + String(c.x1) + "'></label></div>"
            "<div class='col'><label>起点 Y1 (Start Y1)<input type='number' name='y1' value='" + String(c.y1) + "'></label></div></div>"
            "<div class='row'><div class='col'><label>终点 X2 (End X2)<input type='number' name='x2' value='" + String(c.x2) + "'></label></div>"
            "<div class='col'><label>终点 Y2 (End Y2)<input type='number' name='y2' value='" + String(c.y2) + "'></label></div></div>"
            "<div class='row'><div class='col'><label>屏幕宽 (Screen W)<input type='number' name='screen_w' value='" + String(c.screenW) + "'></label></div>"
            "<div class='col'><label>屏幕高 (Screen H)<input type='number' name='screen_h' value='" + String(c.screenH) + "'></label></div></div>"
            "</fieldset>";

    html += "<fieldset><legend>时长与间隔 / Duration & Interval</legend>"
            "<label>基准滑动时长(ms) / Base duration<input type='number' name='duration' value='" + String(c.duration) + "'></label>"
            "<div class='row'><div class='col'><label>上划间隔最小(秒) / Min interval(s)<input type='number' name='interval_min_sec' value='" + String(c.intervalMinSec) + "'></label></div>"
            "<div class='col'><label>上划间隔最大(秒) / Max interval(s)<input type='number' name='interval_max_sec' value='" + String(c.intervalMaxSec) + "'></label></div></div>"
            "<label>时长波动百分比(%) / Duration jitter<input type='number' name='duration_jitter_percent' value='" + String(c.durationJitterPercent) + "'></label>"
            "<label>轨迹步进(ms) / Step interval<input type='number' name='delay_interval' value='" + String(c.delayInterval) + "'></label>"
            "</fieldset>";

    html += "<fieldset><legend>长度与随机 / Length & Randomness</legend>"
            "<label>滑动长度百分比(相对矩形高) / Length percent<input type='number' name='length_percent' value='" + String(c.lengthPercent) + "'></label>"
            "<label>滑动长度波动百分比 / Length jitter<input type='number' name='length_jitter_percent' value='" + String(c.lengthJitterPercent) + "'></label>"
            "<label>延迟/曲率波动百分比 / Delay & curve jitter<input type='number' name='delay_jitter_percent' value='" + String(c.delayJitterPercent) + "'></label>"
            "</fieldset>";

    html += "<fieldset><legend>延迟与曲率 / Delays & Curve</legend>"
            "<label>延迟-悬停(ms) / Hover delay<input type='number' name='delay_hover' value='" + String(c.delayHover) + "'></label>"
            "<label>延迟-按下后(ms) / Press delay<input type='number' name='delay_press' value='" + String(c.delayPress) + "'></label>"
            "<label>贝塞尔弯曲度(0-100) / Curve strength<input type='number' name='curve_strength' value='" + String(c.curveStrength) + "'></label>"
            "<label>双重抬起间隔(ms) / Double release delay<input type='number' name='double_check' value='" + String(c.doubleCheck) + "'></label>"
            "</fieldset>";

    html += "<fieldset><legend>点赞 / Double Tap</legend>"
            "<label>开启随机点赞 / Enable random double tap"
            "<input type='checkbox' name='double_tap_enabled' value='1' ";
    if (c.doubleTapEnabled) html += "checked";
    html += "></label>"
            "<label>概率基准(%) / Base probability<input type='number' name='double_tap_prob_percent' value='" + String(c.doubleTapProbPercent) + "'></label>"
            "<label>概率波动(%) / Probability jitter<input type='number' name='double_tap_prob_jitter_percent' value='" + String(c.doubleTapProbJitterPercent) + "'></label>"
            "<label>双击间隔基准(ms) / Double-tap gap<input type='number' name='double_tap_interval_ms' value='" + String(c.doubleTapIntervalMs) + "'></label>"
            "<label>双击间隔波动(%) / Gap jitter<input type='number' name='double_tap_interval_jitter_percent' value='" + String(c.doubleTapIntervalJitterPercent) + "'></label>"
            "<label>离上划起止的安全缓冲(ms) / Edge buffer range"
            "<div class='row'><div class='col'><input type='number' name='double_tap_edge_min_ms' value='" + String(c.doubleTapEdgeMinMs) + "'></div>"
            "<div class='col'><input type='number' name='double_tap_edge_max_ms' value='" + String(c.doubleTapEdgeMaxMs) + "'></div></div>"
            "<small>点赞在滑动前完成；时间随机落在两个滑动间隔的中段，距离前后边界都有随机缓冲。</small>"
            "</label>"
            "</fieldset>";

    html += "<button type='submit'>保存 / Save</button>"
            "</form>"
            "<form method='POST' action='/auto_swipe/reset_ble'>"
            "<button type='submit' class='danger'>重置蓝牙配对 / Reset BLE Pairing</button>"
            "</form>"
            "<div class='card'><small>提示 / Tip：保存后立即生效；蓝牙+WiFi 在线才会自动上划，起止点与时长会根据上述随机范围浮动。</small></div>"
            "</body></html>";
    return html;
}

// HTTP GET handler for the HTML form
void AutoSwipeManager::handleGet() {
    if (ble) ble->pulseRx(80);
    server->send(200, "text/html", renderPage(cfg));
}

// HTTP POST handler for form/JSON save
void AutoSwipeManager::handlePost() {
    if (ble) ble->pulseRx(80);
    AutoSwipeConfig newCfg = cfg;
    bool isJson = server->hasArg("plain") && server->header("Content-Type").indexOf("application/json") >= 0;
    bool parsed = false;

    if (isJson && server->arg("plain").length() > 0) {
        StaticJsonDocument<512> doc;
        if (!deserializeJson(doc, server->arg("plain"))) {
            applyJsonToConfig(doc, newCfg);
            parsed = true;
        }
    }

    if (!parsed) {
        if (server->hasArg("enabled")) newCfg.enabled = true;
        else newCfg.enabled = false;

        if (server->hasArg("x1")) newCfg.x1 = server->arg("x1").toInt();
        if (server->hasArg("y1")) newCfg.y1 = server->arg("y1").toInt();
        if (server->hasArg("x2")) newCfg.x2 = server->arg("x2").toInt();
        if (server->hasArg("y2")) newCfg.y2 = server->arg("y2").toInt();
        if (server->hasArg("duration")) newCfg.duration = server->arg("duration").toInt();
        if (server->hasArg("screen_w")) newCfg.screenW = server->arg("screen_w").toInt();
        if (server->hasArg("screen_h")) newCfg.screenH = server->arg("screen_h").toInt();
        if (server->hasArg("delay_hover")) newCfg.delayHover = server->arg("delay_hover").toInt();
        if (server->hasArg("delay_press")) newCfg.delayPress = server->arg("delay_press").toInt();
        if (server->hasArg("delay_interval")) newCfg.delayInterval = server->arg("delay_interval").toInt();
        if (server->hasArg("curve_strength")) newCfg.curveStrength = server->arg("curve_strength").toInt();
        if (server->hasArg("double_check")) newCfg.doubleCheck = server->arg("double_check").toInt();
        newCfg.doubleTapEnabled = server->hasArg("double_tap_enabled");
        if (server->hasArg("double_tap_prob_percent")) newCfg.doubleTapProbPercent = server->arg("double_tap_prob_percent").toInt();
        if (server->hasArg("double_tap_prob_jitter_percent")) newCfg.doubleTapProbJitterPercent = server->arg("double_tap_prob_jitter_percent").toInt();
        if (server->hasArg("double_tap_interval_ms")) newCfg.doubleTapIntervalMs = server->arg("double_tap_interval_ms").toInt();
        if (server->hasArg("double_tap_interval_jitter_percent")) newCfg.doubleTapIntervalJitterPercent = server->arg("double_tap_interval_jitter_percent").toInt();
        if (server->hasArg("double_tap_edge_min_ms")) newCfg.doubleTapEdgeMinMs = server->arg("double_tap_edge_min_ms").toInt();
        if (server->hasArg("double_tap_edge_max_ms")) newCfg.doubleTapEdgeMaxMs = server->arg("double_tap_edge_max_ms").toInt();
        if (server->hasArg("interval_min_sec")) newCfg.intervalMinSec = server->arg("interval_min_sec").toInt();
        if (server->hasArg("interval_max_sec")) newCfg.intervalMaxSec = server->arg("interval_max_sec").toInt();
        if (server->hasArg("length_percent")) newCfg.lengthPercent = server->arg("length_percent").toInt();
        if (server->hasArg("length_jitter_percent")) newCfg.lengthJitterPercent = server->arg("length_jitter_percent").toInt();
        if (server->hasArg("duration_jitter_percent")) newCfg.durationJitterPercent = server->arg("duration_jitter_percent").toInt();
        if (server->hasArg("delay_jitter_percent")) newCfg.delayJitterPercent = server->arg("delay_jitter_percent").toInt();
        parsed = true;
    }

    if (!parsed) {
        server->send(400, "application/json", "{\"error\":\"无法解析配置\"}");
        return;
    }

    cfg = newCfg;
    normalizeConfig();
    saveConfig(cfg);
    nextSwipeAt = 0; // 重置计时

    if (isJson) {
        server->send(200, "application/json", "{\"status\":\"ok\",\"note\":\"配置已保存\"}");
    } else {
        server->send(200, "text/html", renderPage(cfg, "配置已保存并生效"));
    }
}

// HTTP GET handler for JSON status
void AutoSwipeManager::handleStatus() {
    if (ble) ble->pulseRx(80);
    StaticJsonDocument<384> doc;
    doc["enabled"] = cfg.enabled;
    doc["x1"] = cfg.x1; doc["y1"] = cfg.y1;
    doc["x2"] = cfg.x2; doc["y2"] = cfg.y2;
    doc["duration"] = cfg.duration;
    doc["screen_w"] = cfg.screenW; doc["screen_h"] = cfg.screenH;
    doc["delay_hover"] = cfg.delayHover;
    doc["delay_press"] = cfg.delayPress;
    doc["delay_interval"] = cfg.delayInterval;
    doc["curve_strength"] = cfg.curveStrength;
    doc["double_check"] = cfg.doubleCheck;
    doc["interval_min_sec"] = cfg.intervalMinSec;
    doc["interval_max_sec"] = cfg.intervalMaxSec;
    doc["length_percent"] = cfg.lengthPercent;
    doc["length_jitter_percent"] = cfg.lengthJitterPercent;
    doc["duration_jitter_percent"] = cfg.durationJitterPercent;
    doc["delay_jitter_percent"] = cfg.delayJitterPercent;
    doc["double_tap_enabled"] = cfg.doubleTapEnabled;
    doc["double_tap_prob_percent"] = cfg.doubleTapProbPercent;
    doc["double_tap_prob_jitter_percent"] = cfg.doubleTapProbJitterPercent;
    doc["double_tap_interval_ms"] = cfg.doubleTapIntervalMs;
    doc["double_tap_interval_jitter_percent"] = cfg.doubleTapIntervalJitterPercent;
    doc["double_tap_edge_min_ms"] = cfg.doubleTapEdgeMinMs;
    doc["double_tap_edge_max_ms"] = cfg.doubleTapEdgeMaxMs;
    doc["wifi"] = (WiFi.status() == WL_CONNECTED);
    doc["ble"] = ble && ble->isConnected();
    doc["next_ms"] = nextSwipeAt == 0 ? 0 : (long)(nextSwipeAt - millis());
    doc["next_like_ms"] = nextLikeAt == 0 ? 0 : (long)(nextLikeAt - millis());

    String out;
    serializeJson(doc, out);
    server->send(200, "application/json", out);
}

// HTTP POST handler to reset BLE pairing/bonds
void AutoSwipeManager::handleResetBle() {
    if (ble) ble->pulseRx(80);
    if (!ble) {
        server->send(500, "application/json", "{\"error\":\"BLE 未初始化\"}");
        return;
    }

    ble->resetPairing();
    bool wantJson = server->header("Accept").indexOf("application/json") >= 0 ||
                    server->header("Content-Type").indexOf("application/json") >= 0;
    if (wantJson) {
        server->send(200, "application/json", "{\"status\":\"ok\",\"note\":\"蓝牙配对已重置\"}");
    } else {
        server->send(200, "text/html", renderPage(cfg, "蓝牙配对已重置，请重新搜索并连接"));
    }
}

// Randomize next interval in ms
unsigned long AutoSwipeManager::randomIntervalMs() {
    int minS = cfg.intervalMinSec;
    int maxS = cfg.intervalMaxSec;
    if (maxS < minS) maxS = minS;
    return (unsigned long)random(minS, maxS + 1) * 1000UL;
}

// Schedule next swipe timestamp
void AutoSwipeManager::scheduleNext() {
    nextSwipeAt = millis() + randomIntervalMs();
    scheduleLike();
}

// 计划下一次点赞时刻（在两次滑动之间）
void AutoSwipeManager::scheduleLike() {
    nextLikeAt = 0;
    if (!cfg.doubleTapEnabled || !ble) return;
    unsigned long now = millis();
    if (nextSwipeAt <= now) return;

    unsigned long interval = nextSwipeAt - now;
    int edgeMin = cfg.doubleTapEdgeMinMs;
    int edgeMax = cfg.doubleTapEdgeMaxMs;
    // 需要至少前后各留一段缓冲
    if (interval <= (unsigned long)(edgeMin + edgeMax + 120)) return;

    int prob = cfg.doubleTapProbPercent;
    int jitter = cfg.doubleTapProbJitterPercent;
    int delta = (prob * jitter + 50) / 100;
    prob = clampInt(prob + random(-delta, delta + 1), 0, 100);
    if (random(0, 100) >= prob) return;

    // 选择靠近上次滑动结束或靠近下一次滑动开始的窗口
    struct Window { unsigned long a; unsigned long b; };
    Window win[2];
    int winCount = 0;

    if (lastSwipeEndedAt > 0) {
        unsigned long a = lastSwipeEndedAt + edgeMin;
        unsigned long b = lastSwipeEndedAt + edgeMax;
        if (b > now + 20) {
            a = max(a, now + 20);
            if (a + 20 < b && a < nextSwipeAt - edgeMin) {
                b = min(b, nextSwipeAt - edgeMin);
                if (b > a + 20) win[winCount++] = {a, b};
            }
        }
    }

    {
        unsigned long a = nextSwipeAt > (unsigned long)edgeMax ? nextSwipeAt - edgeMax : now + 20;
        unsigned long b = nextSwipeAt - edgeMin;
        if (b > now + 40 && b > a + 20) {
            a = max(a, now + 20);
            if (b > a + 20) win[winCount++] = {a, b};
        }
    }

    if (winCount == 0) return;
    Window chosen = win[random(0, winCount)];
    nextLikeAt = random(chosen.a, chosen.b + 1);
}

// 执行一次双击点赞
void AutoSwipeManager::performLike() {
    if (!ble || nextLikeAt == 0) return;

    auto jitterVal = [this](int base, int pct, int minV, int maxV) {
        int delta = (base * pct + 50) / 100;
        return clampInt(base + random(-delta, delta + 1), minV, maxV);
    };

    // 取矩形中心附近一点作为点赞坐标，避免离滑动区域过远
    int minX = min(cfg.x1, cfg.x2);
    int maxX = max(cfg.x1, cfg.x2);
    int minY = min(cfg.y1, cfg.y2);
    int maxY = max(cfg.y1, cfg.y2);
    int cx = (minX + maxX) / 2;
    int cy = (minY + maxY) / 2;
    int jitterX = clampInt((maxX - minX) / 6, 6, 40);
    int jitterY = clampInt((maxY - minY) / 6, 6, 40);
    int px = clampInt(randomAround(cx, jitterX), minX, maxX);
    int py = clampInt(randomAround(cy, jitterY), minY, maxY);

    ActionOptions opts;
    opts.screenW = cfg.screenW;
    opts.screenH = cfg.screenH;
    opts.delayHover = jitterVal(cfg.delayHover, cfg.delayJitterPercent, 0, 2000);
    opts.delayPress = jitterVal(cfg.delayPress, cfg.delayJitterPercent, 0, 2000);
    opts.delayMultiClickInterval = jitterVal(cfg.doubleTapIntervalMs, cfg.doubleTapIntervalJitterPercent, 20, 1200);
    opts.delayDoubleCheck = jitterVal(cfg.doubleCheck, cfg.delayJitterPercent, 0, 2000);

    ble->click(px, py, 2, opts);
    nextLikeAt = 0;
}

// Execute one randomized swipe based on config
void AutoSwipeManager::performSwipe() {
    if (!ble) return;
    swipeInFlight = true;

    auto jitterVal = [this](int base, int pct, int minV, int maxV) {
        int delta = (base * pct + 50) / 100;
        return clampInt(base + random(-delta, delta + 1), minV, maxV);
    };

    // 构造动作参数
    ActionOptions opts;
    opts.screenW = cfg.screenW;
    opts.screenH = cfg.screenH;
    opts.delayHover = jitterVal(cfg.delayHover, cfg.delayJitterPercent, 0, 5000);
    opts.delayPress = jitterVal(cfg.delayPress, cfg.delayJitterPercent, 0, 5000);
    opts.delayInterval = max(2, jitterVal(cfg.delayInterval, cfg.delayJitterPercent, 1, 200));
    opts.curveStrength = clampInt(jitterVal(cfg.curveStrength, cfg.delayJitterPercent, 0, 100), 0, 100);
    opts.delayDoubleCheck = jitterVal(cfg.doubleCheck, cfg.delayJitterPercent, 0, 5000);

    // 计算矩形
    int minX = min(cfg.x1, cfg.x2);
    int maxX = max(cfg.x1, cfg.x2);
    int minY = min(cfg.y1, cfg.y2);
    int maxY = max(cfg.y1, cfg.y2);
    int boxW = max(8, maxX - minX);
    int boxH = max(8, maxY - minY);
    int jitterX = clampInt(boxW / 10, 4, 28);
    int jitterY = clampInt(boxH / 10, 4, 28);

    // 滑动长度 = 矩形高 * lengthPercent，并带波动
    float lenPct = cfg.lengthPercent / 100.0f;
    float lenJit = cfg.lengthJitterPercent / 100.0f;
    float factor = lenPct * (1.0f + random(-100, 101) / 100.0f * lenJit);
    factor = constrain(factor, 0.2f, 1.2f);
    int targetLen = max(8, (int)(boxH * factor));

    // 起点随机落在下半部分，确保方向向上；终点 = 起点向上 targetLen，再抖动
    int startYMin = minY + targetLen;
    if (startYMin > maxY) startYMin = maxY;
    int sx = random(minX, maxX + 1);
    int sy = random(startYMin, maxY + 1);
    sx = clampInt(randomAround(sx, jitterX), minX, maxX);
    sy = clampInt(randomAround(sy, jitterY), minY, maxY);

    int driftX = clampInt(boxW / 3, 6, 60);
    int ex = clampInt(sx + random(-driftX, driftX + 1), minX, maxX);
    int ey = clampInt(sy - targetLen, minY, maxY);
    ex = clampInt(randomAround(ex, jitterX), minX, maxX);
    ey = clampInt(randomAround(ey, jitterY), minY, maxY);

    // 根据实际距离做时长波动
    float distPx = sqrt(pow(ex - sx, 2) + pow(ey - sy, 2));
    int baseDur = cfg.duration + (int)(distPx * 0.06f);
    int swing = max(10, (int)(baseDur * cfg.durationJitterPercent / 100.0f));
    int duration = clampInt(baseDur + random(-swing, swing + 1), 80, 2000);

    ble->swipe(sx, sy, ex, ey, duration, opts);

    swipeInFlight = false;
    lastSwipeEndedAt = millis();
}

// Init manager: load config and register routes
void AutoSwipeManager::begin(WebServer* srv, BleDriver* bleDriver) {
    server = srv;
    ble = bleDriver;
    loadConfig();

    if (server) {
        server->on("/auto_swipe", HTTP_GET, [this]() { handleGet(); });
        server->on("/auto_swipe", HTTP_POST, [this]() { handlePost(); });
        server->on("/auto_swipe/status", HTTP_GET, [this]() { handleStatus(); });
        server->on("/auto_swipe/reset_ble", HTTP_POST, [this]() { handleResetBle(); });
    }
}

// Periodic scheduler tick, also checks WiFi/BLE readiness
void AutoSwipeManager::tick() {
    if (!cfg.enabled) {
        nextSwipeAt = 0;
        nextLikeAt = 0;
        return;
    }

    if (WiFi.status() != WL_CONNECTED || !ble || !ble->isConnected()) {
        nextSwipeAt = 0;
        nextLikeAt = 0;
        return;
    }

    if (nextSwipeAt == 0) {
        scheduleNext();
        return;
    }

    unsigned long now = millis();
    if (!swipeInFlight && nextLikeAt != 0 && now >= nextLikeAt && now + 40 < nextSwipeAt) {
        performLike();
    }

    if (!swipeInFlight && now >= nextSwipeAt) {
        performSwipe();
        scheduleNext();
    }
}
