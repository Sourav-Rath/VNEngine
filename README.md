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

## 📂 Project Structure
