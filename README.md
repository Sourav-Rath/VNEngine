# VNEngine 🎮

A modular **Visual Novel Engine** built using **Qt (C++) and QML**, designed with a fully **data-driven architecture** for scalable narrative systems.

---

## 🚀 Features

### 🧠 Dialogue System
- Node-based dialogue structure
- JSON-driven story data
- Dynamic text rendering

### 🎯 Choice System
- Conditional choices
- Requirements display
- Real-time evaluation

### 🔧 State & Flag System
- Boolean flags (overwrite)
- Integer flags (stacking)
- Fully integrated with choices and conditions

### ⚡ Event System
- Queue-based asynchronous execution
- Supports:
  - `print`
  - `log`
  - `sound`
  - `delay`
  - `transition`
- Non-blocking flow using `QTimer`

### 🔒 Input Lock System
- Prevents input spam
- Frame-safe locking
- UI + backend synchronized
- Visual feedback for disabled state

### 💾 Save / Load
- Persistent game state (`save.json`)
- Stores:
  - Current node
  - Flags / variables

---

## 🏗️ Architecture

### Tech Stack
- **Backend:** C++ (Qt)
- **Frontend:** QML
- **Data:** JSON

### Core Components
- `DialogueManager` → Core logic controller
- `ChoiceModel` → QAbstractListModel for UI binding
- `Node System` → Dialogue structure
- `Event Queue` → Async execution pipeline

---

## 🔥 Current Status

✅ Stable core systems  
✅ Input handling robust  
✅ Event system functional  
⚠️ Condition system needs scalability improvements  

---

## 🛣️ Roadmap

### Step 19.6 — Advanced Conditions
- Operator-based logic (`gte`, `lte`, `eq`, `neq`)
- Fully data-driven condition evaluation

### Future Plans
- Unified event pipeline
- Animation system
- Audio manager
- UI polish & transitions

---

## 🧠 Design Philosophy

This engine is built with a focus on:

- Data-driven design
- Modular systems
- Scalability for complex narrative structures
- Clean separation of logic and UI

---

## 📌 Author

**Sourav Bhadra**  
Game Developer & VFX Artist  

---

## ⚠️ Note

This project is actively under development. Systems are being refined for scalability and performance.

---
