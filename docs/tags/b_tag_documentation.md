# Tag Documentation: `<b>` / `</b>` – Bold Text Formatting

The `<b>` and `</b>` tags are used to apply **bold formatting** to text within game paragraphs.

---

## 🧩 Syntax

```html
<b>Text to be bolded</b>
```

- `<b>` begins a bold segment.
- `</b>` ends the bold segment.
- Markdown-style double asterisks `**` are injected into the output stream by the parser.

---

## 🎯 Purpose

These tags are used to **emphasise** specific words or phrases within game text. They affect **visual display only** and do not influence game logic.

---

## ⚙️ Behaviour in Parser

- When `<b>` or `</b>` is encountered:
  - The game emits a `**` to start or stop bold formatting.
  - A word count is incremented when `<b>` is processed (used for paragraph analysis).

Note: `</b>` does not increment the word count.

---

## 📘 Example

```html
<b> Danger! </b> You sense something moving in the shadows...
```

This will display as:

**Danger!** You sense something moving in the shadows...

---

## ⚠️ Notes

- Always ensure every `<b>` tag has a matching `</b>` tag to avoid formatting issues.
- Nested formatting is not supported.

---

## 🔍 Related Tags

- `<br>` – Line break
- `<i>` (italic formatting, if supported separately in your system)
