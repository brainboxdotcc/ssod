# Toast Message Guide

Toasts are optional visual overlays that appear **beneath** the rest of the game content in **Seven Spells of Destruction**. Use them to provide narrative flavour, humorous feedback, or highlight minor events.

> 🛑 Toasts are for *occasional use only*. Overusing them may annoy players or reduce their narrative impact.

---

## 📦 What is a Toast?

A **toast** is a temporary message shown to the player in a small visual box, with a **thumbnail image** and short block of text. It appears below the current paragraph content.

They are not interactive — they are purely informational.

---

## 🧪 Example

```js
toast("You have drank the 'ad jui'!\n\nThe label originally read 'mad juice'... This was quite a bad idea.", "adjui.png");
add_stamina(-8);
add_skill(-6);
add_speed(-4);
add_luck(-5);
add_sneak(-4);
```

This snippet shows a toast with a funny warning and applies several negative stat changes.

---

## 🔧 Syntax

```js
toast(text, thumbnail_filename);
```

### Parameters:
- `text`: A short string to display in the toast. You may use `\n` for line breaks.
- `thumbnail_filename`: A file from the `resources/` directory on GitHub (e.g. `"adjui.png"`).

> ⚠️ The image **must** be uploaded to the GitHub repo's `resources/` folder and will be served via CDN.

---

## ✨ Best Practices

- Use toasts for:
  - Potion effects
  - Item flavour
  - Magical misfires
  - Humorous moments
- Keep the message **short** and **punchy**
- Don't display the same toast repeatedly

---

## 🛠 Troubleshooting

- ❌ **Image not displaying?** Make sure it exists and is named correctly in the GitHub repo.
- ❌ **No toast shown?** Check your JavaScript for syntax errors.

---

## ✅ Summary

Toasts are an effective way to create lightweight flavour moments. Use them alongside stat changes, achievements, or transformations to let the player *feel* the consequences of their actions — without breaking the flow of gameplay.