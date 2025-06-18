# ✨ Introduction to the Seven Spells Scripting System

Welcome to the reference documentation for **The Seven Spells of Destruction** content authoring system. This guide not only teaches you how to use the tools, it also explains *why* they exist and *what* they enable.

---

## 📚 What *Is* This System?

At its core, Seven Spells is powered by a **domain-specific language (DSL)** called *Paragraph*, embedded directly into the game’s narrative content. It’s a fusion of:

- **Choice-based narrative scripting**
- **Tag-driven command markup**
- **Server-side gameplay logic**
- **Inline JavaScript via `SCRIPT` blocks**

This lets you, the worldbuilder, writer, or designer, create rich interactive experiences that **feel alive**, without needing to be a software engineer.

It’s not just a "choose your own adventure" tool, nor a visual novel engine. It occupies a unique space: **dynamic prose blended with consequence-driven logic**.

---

## 🧠 Why Not Use an Existing Engine?

Most interactive fiction engines and MUD frameworks were built for different goals. Tools like Twine, Ink, and Ren'Py are excellent for solo experiences or small-scale branching stories. Classic MUD engines offer deep simulation and multiplayer, but require low-level scripting and rigid data models.

**Seven Spells** takes a different path - combining the **narrative control of IF**, the **persistence of RPG systems**, and the **real-time presence of a modern multiplayer environment**, all through a **Discord-first interface**.

---

### What Makes *Paragraph* and *Seven Spells* Different?

* **Persistent**: Choices are stored in a live database and can have world-altering consequences. A player who kills a major NPC changes the game for everyone.
* **Multiplayer-native**: Players exist in the same world. Chat, PvP combat, party coordination, and global events are all built in.
* **Modular and Scalable**: Locations are written as self-contained units, but interlink via a connected world graph. There’s no central script to manage or break.
* **Scriptable**: Embedded JavaScript allows for detailed logic, dynamic effects, and conditional content far beyond what most IF engines allow.
* **Discord-native**: The game is played directly inside Discord, making use of its chat interface, user sessions, and presence features. No extra installs, no external UI — just instant access and built-in community.

---

### Built for a Different Kind of Story

Seven Spells isn’t a static gamebook. It’s a **living world**, where players shape the outcome together. *Paragraph* was designed to support that world - reactive, safe to write in, expressive, and scalable.

It’s closer to **writing for a multiplayer RPG campaign** than building a fixed narrative tree. Writers shape locations and events. Developers control state and structure. Players explore a persistent graph that changes with them.

---

## 🧩 What You Can Build

Using the *Paragraph* language, you can:

- Create richly written locations and interlink them (`LINK`, `COMBAT`, etc.)
- Modify stats, inventory, or state with tags like `MOD`, `PICKUP`, `DROP`
- Trigger complex logic using inline JavaScript (`SCRIPT`)
- Define conditionals, one-time events, or hidden paths
- Display different content based on skill, race, profession, or flags
- Define **reusable logic** with passive effects or achievement scripting

You're not just writing scenes - you’re building a **living, reactive world**.

---

## 🚫 What This Is *Not*

This system is **not**:

- A complete general-purpose programming language (though JavaScript is supported)
- A drag-and-drop visual editor
- A no-code game engine
- A 3D or physics-based platform

This is immersive, persistent fiction, where player decisions **matter**, and the medium remains **text**.

---

# 📖 Index

## 🧒 Beginners

- [What is Paragraph?](beginners/paragraph_language_description.md)
- [Creating Game Locations](beginners/creating_game_locations.md)
- [Narrative Style Guide](beginners/narrative_style_guide.md)

---

## 🏷️ Tag Documentation Index (Grouped by Function)

### Important Notes on Tag Syntax:

- Spacing between tags is critical
- Use only one space between attributes in any tag
- Always include a space or newline between the angle brackets and any adjacent text (e.g., `Go to <LINK=5>` not `Go to<LINK=5>`)
- Tag attribute order must follow the documented examples (e.g., `<I>`)

---

### 🧭 Navigation and Linking

- [LINK](tags/link_tag_doc.md) — Link to another location
- [AUTOLINK](tags/autolink_tag_doc.md) — Conditional links based on player state
- [PAYLINK](tags/paylink_tag_doc.md) — Gold-required link activation
- [INPUT](tags/input_tag_doc.md) — Accept text input from the player
- [BOOK](tags/book_tag_documentation.md) — Acquire and read a book

---

### 🧪 Branching, Logic, and State

- [IF](tags/if.md) — Conditional content blocks
- [TEST](tags/test_tag_doc.md) — Stat-based content display
- [SET](tags/set_tag_doc.md) — Set a player-specific state flag
- [UNSET](tags/unset_tag_doc.md) — Remove a player flag
- [SETGLOBAL](tags/setglobal_tag_doc.md) — Set a global flag (all players)
- [UNSETGLOBAL](tags/unsetglobal_tag_doc.md) — Remove a global flag
- [TEMPSET](tags/tempset_tag_doc.md) — Temporary state for timed events
- [SCRIPT](tags/script_tag_doc.md) — Run JavaScript logic inline
- [MACRO](tags/macro_tag_doc.md) — Insert shared content dynamically

---

### ⚔️ Combat and Tests

- [COMBAT](tags/combat_tag_documentation.md) — Start a combat encounter
- [SNEAKTEST](tags/sneaktest_tag_doc_updated.md) — Test sneak ability
- [D12](tags/d12_tag_documentation.md) — Roll a 12-sided die
- [2D6](tags/2d6.md) — Roll two 6-sided dice
- [DICE](tags/dice_tag_documentation.md) — Generic dice behaviour

---

### 📦 Inventory and Items

- [I](tags/i.md) — Buy item from a vendor
- [PICKUP](tags/pickup_tag_doc.md) — Add item to inventory
- [DROP](tags/drop_tag_documentation.md) — Remove item from inventory
- [PICK](tags/pick_tag_doc.md) — Single-choice reward selection

---

### 💠 UI and Display

- [B](tags/b_tag_documentation.md) — Bold text formatting
- [BR](tags/br_tag_documentation.md) — Insert a line break
- [Comments](tags/comment_tag_documentation.md) — Hidden developer notes

---

### 💾 Stack and State Persistence

- [PUSH](tags/push_tag_doc.md) — Push a value onto a stack
- [POP](tags/pop_tag_doc.md) — Pop a value from the stack
- [BANK](tags/bank_tag_documentation.md) — Access the Utopia Bank

---

### ⏳ Time and Effects

- [TIME](tags/time_tag_doc.md) — Advance time (e.g. nightfall)
- [EAT](tags/eat_tag_documentation.md) — Consume a ration
- [EFFECT](tags/effect_tag_documentation.md) — Trigger a passive effect
- [EXPIRE](tags/expire_tag_documentation.md) — Remove a temporary flag

---

## 💻 JavaScript Scripting (Advanced)

- [JavaScript Scripting Guide](js-scripting/javascript_scripting_guide.md)
- [Scripting Quick Reference](js-scripting/scripting_quick_reference.md)
- [Common Scripting Patterns](js-scripting/common_scripting_patterns.md)
- [Player Object Guide](js-scripting/player_object_guide.md)
- [Paragraph State Guide](js-scripting/paragraph_state_guide.md)
- [Game Keys Guide](js-scripting/game_keys_guide.md)
- [Achievement Keys Guide](js-scripting/achievement_keys_guide.md)
- [Toast Message Guide](js-scripting/toast-message-guide.md)

---

## 🏆 Achievement Scripting

- [Achievement Event List](js-scripting/achievement-events.md)
