# Tag Documentation: `<book>` – Discoverable Lore Books

The `<book>` tag is used to embed a reference to a **lore book** within the game world. It allows players to encounter and acquire books, which can later be read from their inventory.

---

## 🧩 Syntax

```html
<book BOOK_ID>
```

- `BOOK_ID`: Numeric ID of the book from the `books` database table.

---

## 📚 Purpose

The tag introduces a book that the player may discover and optionally acquire. Books contain world lore, fiction, technical texts, or historical records.

---

## 🎮 In-Game Behaviour

- When a `<book>` tag is encountered:
  - The game engine queries the `books` database using the provided ID.
  - A short **flavour text snippet** is shown to preview the book's tone or content.
  - The title and author of the book are presented to the player.
  - A clickable link appears allowing the player to read or pick up the book.

---

## 🌐 Internationalisation

- The system checks for a translated title, author, and flavour text if the player's locale is not English (`en`).
- Translations are pulled from the `translations` table where available.

---

## 🔍 Book Availability Rules

The book **will not be shown** if:

- The player already has the book in their inventory.
- The player has stored the book in their bank.

---

## 🧠 Flavour Text Matching

Flavour text is selected dynamically:

- The system searches for flavour entries in `book_flavour_text` whose JSON tag lists overlap the book's tags.
- If no match is found, a random entry is shown.

This provides thematic immersion without needing unique flavour text per book.

---

## 📘 Example

```html
<book 17>
```

May display:

> "You find a dusty volume hidden behind the shelves."  
> 📕 *The Severance Litany*  
> By M. Vael Thalven

---

## 📝 Tips for Authors

- Only use valid book IDs known to exist in the `books` table.
- Avoid repeating the same `<book>` ID multiple times in unrelated paragraphs unless thematically appropriate.

---

## 🔍 Related Tags

- `<pickup>` – For acquiring items (including books).
- `<i>` – Inventory/shop item listings.
- `<if item "Book Title">` – Conditional logic based on books owned.
