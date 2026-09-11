const express = require("express");
const http = require("http");
const { Server } = require("socket.io");
const mqtt = require("mqtt");
const fs = require("fs");

let logs = []; // store logs here

// load existing logs if file exists
if (fs.existsSync("logs.json")) {
  try {
    logs = JSON.parse(fs.readFileSync("logs.json"));
  } catch (e) {
    logs = [];
  }
}

// We store the physical state of devices here. 
// This ensures that when a second user opens the webpage, they see the real current state.
let relayStates = [false, false, false, false, false];
let fanSpeeds = [0, 0, 0];

const app = express();
const server = http.createServer(app);
const io = new Server(server);
const PORT = 3000;

// ---------------- MQTT ----------------
const mqttClient = mqtt.connect("mqtt://localhost:1883");

mqttClient.on("connect", () => {
  console.log("✅ MQTT Connected");
  mqttClient.subscribe("syncspace/#");
});

mqttClient.on("message", (topic, message) => {
  const msg = message.toString();
  console.log("MQTT:", topic, msg);

  // Keep the server's cache updated if the physical hardware reports a state change
  if (topic.includes("relay") && topic.includes("state")) {
      const r = parseInt(topic.split("/")[2]);
      if (!isNaN(r)) relayStates[r] = (msg === "ON");
  }
  if (topic.includes("fan") && topic.includes("state")) {
      const f = parseInt(topic.split("/")[2]);
      if (!isNaN(f)) fanSpeeds[f] = parseInt(msg);
  }

  if (topic === "syncspace/logs") {
    try {
      const entry = JSON.parse(msg);

      // FIX timestamp
      if (entry.timestamp === "NOW" || !entry.timestamp) {
        entry.timestamp = new Date().toLocaleString();
      }

      // Ensure fields exist
      entry.level = entry.level || "INFO";
      entry.source = entry.source || "UNKNOWN";
      entry.event = entry.event || "Unknown";
      entry.current = entry.current ?? "N/A";

      // ID
      entry.id = logs.length + 1;

      // Store
      logs.unshift(entry);
      if (logs.length > 1000) logs.pop();

      // SAVE to file
      fs.writeFileSync("logs.json", JSON.stringify(logs, null, 2));

      // Send structured log to UI
      io.emit("log_update", entry);

    } catch (e) {
      console.log("Invalid JSON log");
    }
  }

  // keep this also (for other data)
  io.emit("mqtt_update", { topic, msg });
});

// ---------------- SOCKET ----------------
io.on("connection", (socket) => {
  console.log("Client connected");

  // SEND OLD LOGS
  socket.emit("log_history", logs);

  // 🔥 SEND CURRENT DEVICE STATE IMMEDIATELY UPON LOAD
  socket.emit("initial_state", { relays: relayStates, fans: fanSpeeds });

  // relay control
  socket.on("relay_control", ({ relay, state }) => {
    // 1. Update server cache
    relayStates[relay] = (state === "ON"); 
    
    // 2. Publish to physical hardware
    const topic = `syncspace/relay/${relay}/cmd`;
    mqttClient.publish(topic, state);

    // 3. Immediately broadcast state change to all users to keep webpages in perfect sync
    io.emit("mqtt_update", { topic: `syncspace/relay/${relay}/state`, msg: state });
  });

  // fan control
  socket.on("fan_control", ({ fan, speed }) => {
    // 1. Update server cache
    fanSpeeds[fan] = speed;
    
    // 2. Publish to physical hardware
    const topic = `syncspace/fan/${fan}/cmd`;
    mqttClient.publish(topic, String(speed));

    // 3. Immediately broadcast state change to all users to keep webpages in perfect sync
    io.emit("mqtt_update", { topic: `syncspace/fan/${fan}/state`, msg: String(speed) });
  });
});

// ---------------- STATIC ----------------
app.use(express.static("public"));

server.listen(PORT, () => {
  console.log(`🚀 Server running on http://localhost:${PORT}`);
});