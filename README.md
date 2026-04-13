# VNEngine 🎮

A modular **Psychological Visual Novel Engine** built using **Qt (C++) and QML**, focused on **player-driven tension, uncertainty, and emotional feedback systems**.

---

## 🚀 Features

### 🧠 Dialogue System
- Node-based structure
- Fully JSON-driven
- Dynamic narrative flow

### 🎯 Choice System
- Conditional branching (AND / OR logic)
- Real-time requirement evaluation
- Dynamic enable/disable states

### 🔧 State & Flag System
- Boolean flags (overwrite)
- Integer flags (accumulative)
- Fully reactive via Q_PROPERTY binding

---

## ⚡ Advanced Systems

### ⏳ Adaptive Timer System
- Dynamic decision timer
- Adjusts based on player state (sanity, time)
- Randomized variation for unpredictability

### 🔥 Global Pressure System
- Time restricts available actions
- Sanity distorts UI and decision clarity
- System reacts to player condition

### 💀 Fail & Consequence System
- Soft failure (choice-based collapse)
- Hard failure (forced nodes)
- State-driven outcomes

### 🧩 Hidden Consequences
- Delayed effects system
- Actions trigger future consequences
- Encourages uncertainty and replayability

### 🧠 Player Trust System
- UI can mislead the player
- Feedback may not always be accurate
- Controlled deception mechanics

---

## 🎭 Emotional Feedback Layer

### Visual Feedback
- Dynamic background (danger-based)
- UI pulse under pressure
- Timer-based scaling

### Psychological Feedback
- Sanity-based UI distortion
- Subtle rotation and instability
- Increasing visual tension

---

## ⚙️ Event System
- Queue-based asynchronous execution
- Supports:
  - `print`
  - `log`
  - `sound`
  - `delay`
  - `transition`
- Non-blocking via QTimer

---

## 💾 Save / Load
- Persistent game state (`save.json`)
- Stores:
  - Current node
  - All flags / variables

---

## 🏗️ Architecture

### Tech Stack
- **Backend:** C++ (Qt)
- **Frontend:** QML
- **Data:** JSON

### Core Components
- `DialogueManager` → Core logic + systems
- `ChoiceModel` → UI binding
- `Node System` → Narrative structure
- `Event Queue` → Async execution

---

## 🔥 Current Status

✅ Core systems stable  
✅ Psychological systems integrated  
✅ Timer + pressure working  
✅ Emotional feedback implemented  

⚠️ Audio layer missing (next step)  
⚠️ Trust system needs deeper variation  

---

## 🛣️ Roadmap

### Step 24 — Audio System
- Heartbeat synced with timer
- Whisper effects (sanity-based)
- Ambient tension loop

### Future
- Advanced UI distortion shaders
- Long-term consequence tracking
- Narrative analytics system
- Editor UI for designers

---

## 🧠 Design Philosophy

This engine focuses on:

- Player emotion over raw mechanics  
- Controlled unpredictability  
- System-driven storytelling  
- Separation of logic, UI, and data  

---

## 📌 Author

**Sourav Bhadra**  
Game Developer & VFX Artist  

---

## ⚠️ Note

This is not a traditional VN engine.

It is designed as a **psychological interaction system** where:
- choices are uncertain  
- feedback is unreliable  
- and player perception is part of gameplay  

---