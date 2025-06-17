# ✨ Introduction to the Seven Spells Scripting System

Welcome to the reference documentation for **The Seven Spells of Destruction** content authoring system. This guide serves not only to teach you how to use the tools — but also to explain *why* they exist and *what* they enable.

---

## 📚 What *Is* This System?

At its heart, Seven Spells is powered by a **domain-specific language (DSL)** embedded directly into the narrative content of the game, called *Paragraph*. It is a fusion of:

- **Choice-based narrative scripting**
- **Tag-driven command markup**
- **Server-executed gameplay logic**
- **Inline JavaScript support via `<script` blocks**

This system allows you — the worldbuilder, writer, designer — to craft interactive experiences that **feel alive**, without requiring you to be a professional software developer.

It is not simply a "choose your own adventure" format, nor is it just a visual novel system. It occupies a unique middle ground: rich prose meets data-driven consequence.

---

## 🧠 Why Not Just Use Twine / Ink / Ren'Py?

While powerful, many existing tools make assumptions Seven Spells does not.

For example:
- They assume linearity or nested branching
- They lack persistent global state or multiplayer features
- They don't have a concept of **shared world continuity**, or item-based interactions that persist across locations
- They don’t natively support Discord, where Seven Spells is played

Seven Spells, by contrast, is:
- **Persistent**: The world state matters — your decisions ripple outward.
- **Multiplayer-aware**: Choices are not always isolated; consequences can affect others.
- **Modular**: Each location can be written independently but feels interconnected.
- **Programmable**: JavaScript blocks can dynamically control narrative, stats, or decisions.

It’s more like writing for a persistent MMORPG than a static story engine.

---

## 🧩 What This Can Do

The Seven Spells *Paragraph* language allows you to:

- Create paragraphs of descriptive prose and interlink them (`LINK`, `COMBAT`, etc.)
- Modify player stats, inventory, and state with tags like `MOD`, `PICKUP`, `DROP`
- Trigger game logic via inline JavaScript using `script` blocks
- Handle one-time events, unlock achievements, or conditionally hide/show content
- Display alternate outcomes depending on skill checks, race, profession, and more
- Define **custom reusable logic** via passive effects and achievement scripting

You are not just writing stories — you are building **interconnected world systems**.

---

## 🚫 What This Is *Not*

This system is **not**:

- A full programming language (though you *can* embed JavaScript)
- A drag-and-drop visual editor
- A no-code game engine
- A 3D engine, or a platform for minigames, physics, or UI-heavy gameplay

You are crafting immersive, persistent fiction where player choices **change the world** — but the medium is still **text**.

---

## 🧭 Navigating This Reference

This documentation is broken down into key categories:

- **Beginner guides**: How to create locations, link them, and structure narrative
- **Scripting tags**: Reference for every in-game tag, like `COMBAT` or `MOD`
- **JavaScript API**: A guide to writing `script` logic, player state, and paragraph interaction
- **Achievements and effects**: How to define one-time or time-limited behaviours
- **Narrative style**: Writing tips for maintaining tone, tense, and cohesion

You can start anywhere. Each section is self-contained — but mastery comes from seeing how they **interlock**.

---

## 🏁 Final Words

This system is **bespoke**, **living**, and **still growing**. There isn’t really anything else like it, because it was built around the specific world of Seven Spells. It respects writers, trusts their intelligence, and empowers them with just enough technical muscle to make their stories real.

You are the architect of not just what players see — but what they *become*.

Now pick a location, and shape the world.

---

# 📖 Index

## Beginners

- [Creating Game Locations](beginners/creating_game_locations.md)
- [Narrative Style Guide](beginners/narrative_style_guide.md)

## 📚 Tag Documentation Index (Grouped by Functionality)

Important notes about tags:

* Spacing between tags is essential
* You should only ever have one space between attributes in a tag.
* You MUST have at least one space or newline between the leading or terminating < > and any text outside the tag, e.g. `Go to <LINK=5>` and not `Go to<LINK=5>`
* Ordering of items within a tag is important and must follow the examples in this documentation (e.g. with the `<I>` tag). This is for simplicity of the parser, and speed of parsing.

Tags are grouped by their primary purpose:

---

### 🧭 Navigation and Linking

- [LINK](tags/link_tag_doc.md) — Link to another location with a button
- [AUTOLINK](tags/autolink_tag_doc.md) — Conditional links based on test results
- [PAYLINK](tags/paylink_tag_doc.md) — Requires gold to activate a link
- [INPUT](tags/input_tag_doc.md) — Accept player text input
- [BOOK](tags/book_tag_documentation.md) — Opens a readable book

---

### 🧪 Branching, Logic, and State

- [IF](tags/if.md) — Conditional branching for different content
- [TEST](tags/test_tag_doc.md) — Performs a stat test and displays different content
- [SET](tags/set_tag_doc.md) — Set a per-player state flag
- [UNSET](tags/unset_tag_doc.md) — Remove a per-player state flag
- [SETGLOBAL](tags/setglobal_tag_doc.md) — Set a global (server-wide) flag
- [UNSETGLOBAL](tags/unsetglobal_tag_doc.md) — Remove a global (server-wide) flag
- [TEMPSET](tags/tempset_tag_doc.md) — Temporarily set a flag for a limited time
- [SCRIPT](tags/script_tag_doc.md) — Run a custom JavaScript script
- [MACRO](tags/macro_tag_doc.md) — Insert shared content

---

### ⚔️ Combat and Tests

- [COMBAT](tags/combat_tag_documentation.md) — Triggers a combat encounter
- [SNEAKTEST](tags/sneaktest_tag_doc_updated.md) — Sneak test logic
- [D12](tags/d12_tag_documentation.md) — Roll a D12 die for randomness
- [2D6](tags/2d6.md) — Roll two six-sided dice
- [DICE](tags/dice_tag_documentation.md) — Generic dice roll behaviour

---

### 📦 Inventory and Item Handling

- [I](tags/i.md) — Let the player purchase an item
- [PICKUP](tags/pickup_tag_doc.md) — Allow player to collect something
- [DROP](tags/drop_tag_documentation.md) — Remove item(s) from inventory
- [PICK](tags/pick_tag_doc.md) — Allow one-time choice of a single reward

---

### 💠 UI and Display Enhancements

- [B](tags/b_tag_documentation.md) — Bold text
- [BR](tags/br_tag_documentation.md) — Line break
- [Comments](tags/comment_tag_documentation.md) — Leave a hidden comment in source

---

### 💾 Data Persistence and Stack

- [PUSH](tags/push_tag_doc.md) — Push a value onto the local stack
- [POP](tags/pop_tag_doc.md) — Pop a value from the stack
- [BANK](tags/bank_tag_documentation.md) — Give the player access to the bank of Utopia

---

### ⏳ Time and Effects

- [TIME](tags/time_tag_doc.md) — Represents a full night passing
- [EAT](tags/eat_tag_documentation.md) — Forces the player to consume a ration
- [EFFECT](tags/effect_tag_documentation.md) — Triggers a passive effect
- [EXPIRE](tags/expire_tag_documentation.md) — Clears a temporary state flag

## JavaScript ES5 Scripting (Advanced)

- [Javascript Scripting Guide](js-scripting/javascript_scripting_guide.md)
- [Scripting Quick Reference](js-scripting/scripting_quick_reference.md)
- [Common Scripting Patterns](js-scripting/common_scripting_patterns.md)
- [Player Object Guide](js-scripting/player_object_guide.md)
- [Paragraph State Guide](js-scripting/paragraph_state_guide.md)
- [Game Keys Guide](js-scripting/game_keys_guide.md)
- [Achievement Keys Guide](js-scripting/achievement_keys_guide.md)
- [Toast Message Guide](js-scripting/toast-message-guide.md)

## Achievement Scripting

- [Achievement Event List](js-scripting/achievement-events.md)
