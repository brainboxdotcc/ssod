# Player Object Overview

The `player` object represents the current player during paragraph script execution. It provides **read-only** access to character stats, inventory, and metadata — and allows **controlled changes** via special global setter and adder functions.

> ⚠️ Direct modifications to the `player` object are **ignored**. You must use global functions like `set_gold()` or `add_skill()` to make changes that persist.

---

## 🧍 What is the `player` Object?

The `player` object is a snapshot of the player's current state. You can use it to:

- Check player stats and inventory
- Display custom messages based on their condition
- Make decisions in branching logic

However, you **cannot** update it like a normal object. For example:

```js
player.gold = 99999; // ❌ Has no effect
set_gold(99999);     // ✅ Works as expected
```

---

## 🔄 How to Modify Player State

### ✅ Setters (absolute)

Use these global functions to assign a **specific value** to a player stat:

```js
set_gold(25);
set_skill(10);
set_paragraph(901);
```

### ➕ Adders (relative)

Use these to add (or subtract) from a stat. Negative values are accepted.

```js
add_gold(10);     // Add 10 gold
add_stamina(-3);  // Lose 3 stamina
```

> ℹ️ Changes are stored and persisted in the player save data. Use responsibly!

---

## 🧾 Fields You Can Read

| Field               | Description                             |
| ------------------- | --------------------------------------- |
| `player.name`       | Player's current name                   |
| `player.race`       | Race chosen at character creation       |
| `player.profession` | Profession/class                        |
| `player.gender`     | Player gender                           |
| `player.stamina`    | Health points                           |
| `player.skill`      | Skill score                             |
| `player.luck`       | Luck score                              |
| `player.sneak`      | Sneak skill                             |
| `player.speed`      | Speed attribute                         |
| `player.gold`       | Gold carried                            |
| `player.silver`     | Silver coins                            |
| `player.rations`    | Number of meals carried                 |
| `player.experience` | XP score                                |
| `player.level`      | Level derived from experience           |
| `player.notoriety`  | How infamous the player is              |
| `player.days`       | In-game days passed                     |
| `player.scrolls`    | Number of scrolls carried (key items)   |
| `player.paragraph`  | Paragraph ID the player is currently in |
| `player.armour`     | Name of equipped armour (if any)        |
| `player.weapon`     | Name of equipped weapon (if any)        |
| `player.mana`       | Current mana pool                       |
| `player.mana_tick`  | Countdown until mana regenerates        |

---

## 🚫 Avoid Setting These Unless You Know What You're Doing

Some fields should only be changed by special scripts or events:

| Field               | Why you should avoid it              |
| ------------------- | ------------------------------------ |
| `player.name`       | Often set at character creation only |
| `player.race`       | Changing it affects lore and logic   |
| `player.profession` | May unbalance combat logic           |
| `player.gender`     | Rarely necessary to change           |

If you're writing something like a **transformation effect**, such as the passive effect that turns the player into a chicken temporarily, it's okay to modify these fields — but always restore them properly later.

---

## 🧪 Example Usage

### 💰 Give the Player 50 Gold

```js
add_gold(50);
```

### 😨 Hurt the Player

```js
add_stamina(-2);
```

### 🗺️ Move Player to Paragraph 104

```js
set_paragraph(104);
```

### ⚔️ Equip Weapon (if you trust the value!)

```js
set_weapon("Iron Dagger");
```

---

## 🔐 Related Global Functions

You may also find these helpful:

- `get_key(name)` — read a state flag
- `set_key(name, value)` — set a persistent flag
- `delete_key(name)` — remove a saved key
- `toast(message, style)` — show a temporary overlay
- `unlock_ach(id)` — unlock an achievement for the player

---

Return to the [JavaScript Scripting Guide](../javascript-scripting-guide.md) or view the [Paragraph State Guide](../paragraph-state-guide.md) next.

