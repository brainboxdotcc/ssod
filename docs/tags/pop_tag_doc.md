
# `<pop>` Tag

The `<pop>` tag allows you to create a link that returns the player to the most recently *pushed* location in their navigation stack. This is useful when you've previously used a `<push>` tag to temporarily divert the player to a side scene, like a shop, sub-choice, or cutscene, and now want to let them return cleanly.

---

### ✅ Syntax

```plaintext
<pop>
```

This tag has no attributes and is self-closing.

---

### 📌 Behaviour

- Adds a new navigation link to return to the previously pushed location.
- Automatically creates a link button with the label “Return” (translated based on the player’s language).
- This navigation is one-directional; it only works if a `<push>` was used earlier.
- If the navigation stack is empty, this button will silently do nothing.

---

### 🛠 Example

```plaintext
You've browsed the merchant's wares long enough.

<pop>
```

This would render:

> **Return** 🡒 (button)

---

### 🔄 Related Tags

- `<push>` — Save the current location onto the player's navigation stack.
