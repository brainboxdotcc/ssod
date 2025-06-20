# Tag Documentation: `<bank>` – Access Banking Services

The `<bank>` tag is used to embed a link within a paragraph that allows the player to **access their bank account**, enabling deposit, withdrawal, or inspection of stored gold and items.

---

## 🧩 Syntax

```html
<bank>
```

- No attributes or parameters.
- Should be placed in a paragraph where the player has access to banking services (e.g. at a bank or secure trading post).

---

## 🏦 Behaviour

When this tag is encountered:

- A navigation link is created that routes the player to a **bank interface**.
- The player can interact with their stored possessions and gold.
- The label for the bank link is internationalised via the `USEBANK` translation key.
- The link is displayed with an incremented direction marker from the paragraph's `directions[]` array.

---

## 📘 Example

```html
<bank>
```

Displays something like:

```text
**Use the Bank** → [Link 1]
```

(Actual text depends on translation.)

---

## 🔍 Related Tags

- `<link>` – General purpose navigation
- `<paylink>` – Purchasable navigation options
- `<i>` – Buyable inventory items
