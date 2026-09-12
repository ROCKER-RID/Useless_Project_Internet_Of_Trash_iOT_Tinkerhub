```markdown
<img width="1280" height="640" alt="git (1)" src="https://github.com/user-attachments/assets/8920b256-2ba8-4988-b824-5351134eb4bd" /> 

# Switch Devil 🎯

## Basic Details
### Team Name: SyncSpace: Internet Of Trash iOT

### Team Members
- Team Lead: Ridhin George - Sahrdaya College of Engineering & Technology
- Member 2: Aaron A S - Sahrdaya College of Engineering & Technology

### Project Description
Switch Devil is a cyber-physical platformer where your household 230V AC appliances are held hostage behind a brutal 2D gauntlet. To operate your room's LED lights, induction ceiling fan, and halogen lamp, you must physically survive platform traps, Tesla lightning barriers, and high-voltage wind corridors while iconic Malayalam cinema icons roast your failures in real-time.

### The Problem (that doesn't exist)
Flipping a standard wall switch to turn on your lights or fans is dangerously effortless, boring, and encourages lazy human behavior. Why should electricity flow to your appliances just because a finger exerted 0.2 Newtons of mechanical force? There is zero adrenaline, zero character growth, and absolutely no emotional trauma involved in turning on a desk lamp.

### The Solution (that nobody asked for)
We put a microcontroller, optoisolated AC TRIAC phase-angle controllers, and an SPDT master relay between the human and their electricity bill. Want illumination? Beat Sector 1. Want to keep playing without having your room plunged into darkness? Risk your unlocked utilities in a high-stakes "Risk vs. Reward" game-show prompt before each level. If you fall, high-voltage halogen lamps flicker violently in microsecond sync with screen flashes while legendary Malayalam actors remind you of your complete lack of hand-eye coordination.

---

## Technical Details

### Technologies/Components Used

#### For Software:
- **Languages:** JavaScript (ES6+), C++ (Arduino ESP32 Core), HTML5, CSS3
- **Backend & Gateways:** Node.js, Express.js, Socket.IO, Eclipse Mosquitto (MQTT)
- **Frontend / Engine:** HTML5 Canvas 2D Rendering Engine, Web Audio API, Google TTS API fallback
- **Protocols:** WebSockets (Socket.IO bi-directional bridge), MQTT (TCP 1883 with JSON state telemetry), SoftAP Captive Portal (DNS 53 + HTTP 80)
- **Tools & Environments:** VS Code, Arduino IDE, Git, Paint.NET

#### For Hardware:
- **Main Microcontroller:** ESP32 Dev Module (Dual-core Xtensa 32-bit LX6 @ 240 MHz)
- **High Voltage Actuation:**
  - 3x BT136 600V 4A Sensitive Gate TRIACs (Heat-sink mounted)
  - 3x MOC3021 Non-Zero-Cross Random-Phase Optocouplers (Galvanic isolation)
  - 1x 10A / 250V AC SPDT Electromechanical Relay Module (Master Phase Isolator)
- **AC Zero-Crossing Detection (ZCD):**
  - H11AA1 Dual-LED AC Input Optocoupler / ZMPT101B Network (Falling Edge on GPIO 14)
- **Connected 230V AC Loads:**
  - 1st TRIAC: Dimmable 230V AC LED Utility Bulb
  - 2nd TRIAC: 230V AC Induction Motor Fan (Ceiling / Table Fan)
  - 3rd TRIAC: High-Intensity Halogen Lamp (Ghost flicker & ambient dimming)
- **Tools Required:** Soldering station, digital multimeter, differential oscilloscope probe, wire strippers, non-conductive enclosure.

---

### Implementation

#### For Software:

# Installation
```bash
# 1. Clone the repository
git clone [https://github.com/your-username/switch-devil.git](https://github.com/your-username/switch-devil.git)
cd switch-devil

# 2. Install backend gateway dependencies
npm install express socket.io mqtt

# 3. Ensure Mosquitto MQTT Broker is installed and running
# On Ubuntu/Debian:
sudo apt-get install mosquitto mosquitto-clients -y
sudo systemctl start mosquitto

```

# Run

```bash
# Start the Node.js Socket.IO ↔ MQTT Gateway Server
node server.js

```

*The web interface will be live at `http://localhost:3000`.*

#### For Hardware (ESP32 Firmware):

1. Open `firmware/useless_projecr11sep26v2/useless_projecr11sep26v2.ino` in the Arduino IDE.
2. Select Board: **ESP32 Dev Module**.
3. Install required library: **PubSubClient** by Nick O'Leary via the Library Manager.
4. Upload to the ESP32 via USB.
5. On first boot, connect your phone or laptop to the Wi-Fi AP **`SyncSpace_Setup_Useless`** (Password: `12345678`), navigate to `192.168.4.1`, select your home Wi-Fi, and enter your server's MQTT IP.

---

### Project Documentation

#### For Software:

# Screenshots


*Switch Devil title screen showing the Terms & Conditions acceptance gate, Malayalam/English voice selector, and real-time hardware latency calibration slider.*


*Sector 4 moving platform gauntlet with active induction fan wind particle streams, Tesla electrical discharge gate, and real-time IoT status indicator.*


*Sector Complete dialog prompting the player to either cash out their unlocked physical appliance (LED / Fan / Halogen) or risk it all to enter the next sector.*

# Diagrams

```
┌────────────────────────────────────────────────────────┐
│             HTML5 Canvas Client (Browser)              │
│    - Player Physics & Collision Engine                 │
│    - In-game 2D Fan Wind Calculations                  │
│    - Dynamic Flash Latency Calibrator                  │
└───────────────────────────┬────────────────────────────┘
                            │ WebSockets (Socket.IO)
                            ▼
┌────────────────────────────────────────────────────────┐
│            Node.js Gateway Server (server.js)          │
│    - Bridges Socket.IO to Local MQTT Broker            │
│    - Caches Device State & Logs Structured JSON Events │
└───────────────────────────┬────────────────────────────┘
                            │ TCP Port 1883
                            ▼
┌────────────────────────────────────────────────────────┐
│             Mosquitto MQTT Broker (localhost)          │
│    - Topics: syncspace/fan/+/cmd, syncspace/relay/+/cmd│
└───────────────────────────┬────────────────────────────┘
                            │ 2.4 GHz Wi-Fi
                            ▼
┌────────────────────────────────────────────────────────┐
│                   ESP32 Microcontroller                │
│    - Zero-Cross Interrupt on GPIO 14 (50Hz Mains)      │
│    - Microsecond esp_timer Phase-Angle Slicing         │
│    - 2-Second Kickstart & Safety Watchdog Auto-Off     │
└─────────┬──────────────────┬──────────────────┬────────┘
          │ GPIO 27          │ GPIO 4           │ GPIO 16           │ GPIO 17
          ▼                  ▼                  ▼                   ▼
    [Master Relay]     [1st TRIAC]        [2nd TRIAC]         [3rd TRIAC]
    230V AC Mains      LED Utility Bulb   Induction Fan       Halogen Lamp
    Phase Isolation    (100% On Claim)    (Kickstart+Thrust)  (Ghost Flicker)

```

*End-to-end cyber-physical architecture: Real-time user input and game physics are bridged through Socket.IO and MQTT to the ESP32's microsecond phase-angle timers.*

---

#### For Hardware:

# Schematic & Circuit

```
230V AC Mains (Live) ──► [Master Relay (GPIO 27)] ──┬──► [MOC3021 + BT136 (GPIO 4)]  ──► LED Bulb (Triac 0)
                                                    ├──► [MOC3021 + BT136 (GPIO 16)] ──► AC Fan (Triac 1)
                                                    └──► [MOC3021 + BT136 (GPIO 17)] ──► Halogen (Triac 2)

230V AC Mains (Neutral) ────────────────────────────┴────────────────────────────────► Common Return

AC Mains Sensing ──► [H11AA1 Optocoupler] ──────────► ESP32 GPIO 14 (ZCD Hardware Interrupt)

```

*Complete AC power distribution: Master relay cuts/energizes the phase rail for all three TRIAC channels, while the zero-cross detector provides phase-angle synchronization.*


*Full hardware schematic document: Refer to [Syncspace_internet_Of_Trash_iot_Useless_Hardware.pdf](https://www.google.com/search?q=Syncspace_internet_Of_Trash_iot_Useless_Hardware.pdf) for the complete optoisolated PCB routing, high-voltage creepage separation, snubber networks, and zero-crossing detection stages.*

# Build Photos


*All key components: ESP32 development board, BT136 TRIACs mounted to aluminum extrusions, MOC3021 opto-triacs, H11AA1 optocoupler, relay module, and snubber networks.*


*Assembly process: Mounting heat sinks, wiring optoisolated gate trigger lines, and bench-testing zero-crossing waveforms.*


*Final deployment: The real-world test rig with the physical LED bulb, induction desk fan, and halogen floodlight synced to the Switch Devil display.*

---

### Project Demo

# Video

[]https://youtu.be/iDIchzv8JpA?si=oO54uRwdL7lqyoZV)
*Demonstration of full gameplay progression: Real-time halogen ghost flickers on death, kick-start break-away torque on the AC induction fan, dynamic 100% thrust acceleration on Sector 4's 4th platform, and cashing out unlocked utilities via the Risk vs. Reward modals.*

# Additional Demos

* [Interactive Web Client Preview](https://www.google.com/search?q=https://github.com/your-username/switch-devil)
* [Complete Hardware Schematics (PDF)](https://github.com/ROCKER-RID/Useless_Project_Internet_Of_Trash_iOT_Tinkerhub/tree/main/Syncspace_internet_Of_Trash_iot_Useless_Hardware.pdf)
* Captive portal provisioning demonstration under `firmware/`

---

## Team Contributions

* **Ridhin George:** Architected and built the entire cyber-physical system—engineered the high-voltage AC circuit schematics (`Syncspace_internet_Of_Trash_iot_Useless_Hardware.pdf`), authored the ESP32 phase-angle dimming firmware with microsecond zero-cross timer interrupts, developed the Node.js Socket.IO ↔ MQTT gateway server, programmed the core HTML5 Canvas 2D game engine, physics, hazard logic, real-time latency calibration, and the Risk vs. Reward progression system.
* **Aaron A S:** Optimized the game UI aesthetics, visual styling, CRT filters,programmed the core HTML5 Canvas 2D game engine, physics, hazard logic, real-time latency calibration, fine-tuned the Malayalam meme audio sound effects and timing, and integrated the dialogue reaction stickers.

---

Made with ❤️ at TinkerHub Useless Projects

```

```
