# `<combat>` Tag Documentation

The `<combat>` tag is used to initiate a combat encounter in a game paragraph. It is one of the more complex tags and introduces a branching mechanic where the paragraph is effectively split into multiple **fragments**. Only one fragment is visible at a time, and progression through fragments depends on the outcome of combat.

---

## 📌 Syntax

```text
<combat NAME="monster name" SKILL="X" STAMINA="Y" ARMOUR="Z" WEAPON="W">
```

Optional attribute:

```text
LEVELED
```

If present, this dynamically scales the monster's stats with the player's level.

---

## 🧠 How It Works

- When a `<combat>` tag is encountered in a paragraph, the game engine splits the paragraph into fragments.
- The current fragment ends where the combat starts.
- After the combat is resolved, the **next fragment** of the paragraph becomes visible.
- This allows writers to script events that happen **after** combat inline, without branching to a new paragraph.

---

## 📈 Parameters

| Attribute | Description |
|----------|-------------|
| `NAME`   | Name of the monster. Displayed in the combat interface. |
| `SKILL`  | Monster's skill stat. |
| `STAMINA`| Monster's stamina stat. |
| `ARMOUR` | Monster's armour value. |
| `WEAPON` | Monster's weapon rating. |
| `LEVELED`| *(Optional)* When set, all stats are increased based on player level. |

---

## 💬 Translation Support

If the player's locale is not English, the monster name is translated using a lookup from the database.

---

## 📜 Example

```text
You step into the clearing and face a monstrous bear.
<combat NAME="Dire Bear" SKILL="7" STAMINA="10" ARMOUR="2" WEAPON="3">
It slumps to the ground, defeated.
```

---

## ⚠️ Important Notes

- This tag **must** be placed within a paragraph that allows multiple fragments.
- The portion of the paragraph after the `<combat>` is **not parsed** until combat is over.
- If multiple `<combat>` tags are present, each introduces a new fragment.
- Illness effects are evaluated only once per paragraph on the first combat fragment.

---

## 🧪 Internal Mechanics Summary

- Adds a navigation link of type `nav_type_combat`.
- Calls a translation check for the monster name if locale ≠ "en".
- Checks player's diseases and applies debuffs.
- Calculates XP from monster stats using `calc_xp_worth()`.
- Appends ANSI-style combat formatting to the output.

---

## 🔒 Side Effects

This tag:
- Ends the current visible fragment.
- Sets `p.safe = false`.
- Throws `parse_end_exception()` to halt further processing of the current fragment.

Use this tag with care. It is one of the few tags that controls paragraph flow.

