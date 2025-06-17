# JavaScript Scripting Guide for Content Authors

In *Seven Spells of Destruction*, the `<script>` tag allows paragraph content to execute embedded JavaScript, enabling advanced logic, interactivity, and dynamic behaviour. This is powered internally by [Duktape](https://duktape.org/), a lightweight embedded JavaScript engine.

This guide explains what you can do with `<script>` and how to write safe, effective scripts for use within game locations.

---

## 📜 Basic Usage

```html
<script>
  if (player.gold >= 50) {
    player.gold -= 50;
    print("\nYou hand over 50 gold.");
  } else {
    print("\nYou don't have enough gold.");
  }</script>
```

Scripts are placed inside a `<script>` block. Do **not** include the closing `</script>` on its own line — embed it inline with your code.

---

## 🧰 Available Objects

### `player`

This object reflects the current player. All fields are read/write unless noted.

| Property                                     | Type   | Description                               |
| -------------------------------------------- | ------ | ----------------------------------------- |
| `name`, `race`, `profession`, `gender`      | string | Character identity fields                 |
| `gold`, `silver`, `scrolls`                 | number | Currency and scrolls                      |
| `stamina`, `skill`, `luck`, `speed`, `sneak`| number | Core attributes                           |
| `mana`, `mana_tick`                         | number | Magic pool and recovery timer             |
| `rations`, `experience`, `notoriety`, `days`| number | Status tracking fields                    |
| `level`                                     | number | Derived level                             |
| `armour`, `weapon`                          | object | Equipped gear, contains `.rating`         |
| `armour_item`, `weapon_item`                | string | Gear names                                |
| `last_use`, `last_strike`                   | number | Combat event timestamps                   |
| `pinned`, `muted`                           | bool   | Flags for communication restrictions      |
| `scrolls`, `stance`, `attack`               | varies | Combat or quest details                   |

### `paragraph`

Contains information about the current paragraph.

| Property          | Type   | Description                                  |
| ----------------- | ------ | -------------------------------------------- |
| `id`              | number | Paragraph ID                                 |
| `text`            | string | Paragraph raw text                           |
| `safe`            | bool   | If false, paragraph is unsafe (e.g. hostile) |
| `links`           | number | Number of link buttons so far                |
| `words`           | number | Number of words printed so far               |
| `combat_disabled` | bool   | Disables combat in this paragraph            |
| `magic_disabled`  | bool   | Disables magic                               |
| `chat_disabled`   | bool   | Disables public chat                         |
| `tag`             | string | Name of the last parsed tag                  |
| `auto_test`       | bool   | Result of last test (e.g. from `<test>`)     |

> Do not try to change the paragraph text directly. Use `print()` instead.

You may also use:
- `BOT_ID` — the bot’s Discord ID.
- `PARAGRAPH_ID` — the numeric ID of the current paragraph.

---

## ✍️ Built-in Functions

### 🔹 Output & Utility

| Function                     | Description                                           |
|-----------------------------|-------------------------------------------------------|
| `print(...)`                | Print to the screen. Accepts multiple values.         |
| `log(...)`                  | Internal debug log, not visible to players.           |
| `exit(code)`                | Halt execution. If code is `1`, no further processing.|
| `tag("name")`              | Sets a custom tag name for this paragraph.            |
| `toast("title", "msg")`   | Shows a stylised overlay message.                     |

### 🔹 Getters
These read current player values.

```js
get_name(); get_race(); get_profession(); get_gender();
get_stamina(); get_skill(); get_luck(); get_sneak(); get_speed();
get_silver(); get_gold(); get_rations(); get_experience();
get_level(); get_notoriety(); get_days(); get_scrolls();
get_paragraph(); get_parent(); get_armour(); get_weapon();
get_last_use(); get_last_strike(); get_pinned(); get_muted();
get_mana(); get_mana_tick();
```

### 🔹 Setters
These assign new values.

```js
set_name("..."), set_race("..."), set_profession("..."), set_gender("...");
set_stamina(n), set_skill(n), set_luck(n), set_sneak(n), set_speed(n);
set_silver(n), set_gold(n), set_rations(n), set_experience(n);
set_notoriety(n), set_days(n), set_scrolls(n), set_paragraph(n);
set_armour("..."), set_weapon("...");
set_last_use(n), set_last_strike(n);
set_pinned(true), set_muted(true);
set_mana(n), set_mana_tick(n);
```

### 🔹 Adders
These increment or decrement values.

```js
add_stamina(n), add_skill(n), add_luck(n), add_sneak(n), add_speed(n);
add_silver(n), add_gold(n), add_rations(n);
add_experience(n), add_notoriety(n), add_mana(n);
```

### 🔹 Key/Value Store
These store data associated with the player. Useful for tracking state.

```js
set_key("key", "value");         // Save string value
get_key("key");                  // Retrieve value (or null)
delete_key("key");              // Remove the value
```

### 🔹 Achievement Key Store
Similar to above but scoped to achievements.

```js
set_ach_key("key", "value");
get_ach_key("key");
delete_ach_key("key");
has_ach("ACH_ID");
unlock_ach("ACH_ID");
```

---

## 🛑 Notes on `print()`

Use `print(...)` to output text. It automatically appends to the paragraph’s content.

```js
print("You found a hidden cache!");
```

You can use multiple arguments:

```js
print("Hello, ", player.name, "!\n");
```

---

## ⚠️ Important Notes

- Scripts **must** end with `</script>` inline — not on a new line.
- Scripts run **after** paragraph parsing.
- `player` and `paragraph` are sandboxed — changes persist but are isolated.
- Syntax errors fail silently — check carefully.
- Use `toast()` sparingly for in-game flavour.

---

## 🛠 Recommended Practices

- Use for conditionals and modifiers based on player stats.
- Avoid overly complex calculations or loops.
- Keep logic readable — aim for clarity over cleverness.
- Use key store for tracking things like visited areas or item flags.

---

## 🔗 Reference

- [Duktape API](https://duktape.org/api.html) – underlying engine
- [Seven Spells GitHub](https://github.com/brainboxdotcc/ssod/) – dev context (optional)

For help, visit the dev Discord or consult working scripts in the content repo.

---

Happy scripting!

