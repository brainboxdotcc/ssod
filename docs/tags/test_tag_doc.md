# `<test>` Tag

The `<test>` tag is used to initiate a **stat-based skill check** against the player’s current attributes. The outcome is determined automatically (no input from the player is required) and can be used to branch outcomes via paired `<AUTOLINK>` tags.

### 🔍 Syntax

```
<test luck>
<test stamina>
<test skill>
<test speed>
<test exp>
```

Each of these will perform a test against the relevant stat. Only one stat may be tested per `<test>` tag.

---

### 🎲 Test Mechanics

- The player rolls a virtual D6 and adds it to their stat.
- A success threshold is determined internally based on the stat (and game rules).
- The result is saved to `p.auto_test`, allowing you to use:
  - A `<AUTOLINK>` for **success**
  - A second `<AUTOLINK>` for **failure**

Only one of the two will be enabled.

---

### 🧪 Luck is Finite!

> **Luck tests consume 1 point of Luck** each time they are performed.  
> A player’s luck can (and will) run out — both literally and narratively. Use these sparingly if you want to avoid punishing players.

---

### ✅ Use With

- **Two `<AUTOLINK>` tags** immediately after the `<test>` tag.
  - First link: Success path.
  - Second link: Failure path.

### 📌 Example

```
<test luck>
Were you lucky <AUTOLINK=1543> <!-- Success -->
Or unlucky <AUTOLINK=1544> <!-- Failure -->
```

---

### 🛑 Do Not

- Use `<test>` without pairing it with two `<AUTOLINK>`s.
- Mix multiple `<test>`s within a single paragraph.

---

### 🧠 Tip for Authors

Testing stats like `speed`, `stamina`, or `experience` can reflect more than raw numbers — consider using them to gate events that represent difficulty, knowledge, or endurance.

