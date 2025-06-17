# Tag Documentation: `<br>` – Line Break

The `<br>` tag inserts a manual **line break** into a paragraph. It functions similarly to `<br>` in HTML, separating content with a new line without starting a new paragraph.

---

## 🧩 Syntax

```html
<br>
```

There are no parameters or attributes for this tag.

---

## 📚 Purpose

Use `<br>` when you want to:

- Improve **readability** within a single paragraph.
- Visually separate distinct pieces of content (like descriptions and outcomes).
- Break up long text segments without forcing a new paragraph structure.

---

## 🎮 In-Game Behaviour

- Produces a **newline character** (`\n`) in the rendered output.
- Can be used multiple times within a single paragraph.
- Increments the internal word count for analytic and reward purposes.

---

## 📘 Example

```html
You drink the potion. <br> It tastes of bitter herbs.
```

Will appear in-game as:

```
You drink the potion.
It tastes of bitter herbs.
```

---

## 📝 Tips for Authors

- Avoid overusing `<br>` when a full paragraph split would be clearer.
- Use for **lightweight formatting** or in combination with tags like `<if>`, `<dice>`, etc.

---

## 🔍 Related Tags

- `<b>` – Bold formatting.
- `<i>` – Inventory/shop tag.
- `<if>` – Conditional branching logic.
