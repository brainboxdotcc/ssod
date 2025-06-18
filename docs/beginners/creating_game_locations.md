# Creating Game Locations

Locations are the core building blocks of **The Seven Spells of Destruction**. Each location represents a paragraph of interactive text, and players move from one to another via links, tests, and game mechanics.

This guide walks you through the process of creating your own locations via the admin panel.

---

## 🗺️ What Is a Location?

A location is a discrete, numbered narrative moment — often a place, encounter, or situation. It contains the following:

- Descriptive prose
- Game tags (e.g. `<LINK=5>`, `<COMBAT>`, `<MOD>` etc.)
- Navigation and decisions
- Possibly item pickups, tests, effects, or achievements

Locations are stored in a numbered range (1 to 9999), and can be edited freely via the **Admin Panel**.

---

## 📍 Getting Started

1. Open the **Admin Panel** and select **Edit Location**
2. Choose an **empty location number** (somewhere unused between 1 and 9999)
3. Begin writing your prose in the `Location Text` box

You can use existing locations (such as 1, 45, 51) as inspiration — but always write original content.

---

## 🔗 Linking to Other Locations

Use the `<LINK=ID>` or `<LINK=ID, button text>` tag to connect your paragraph to other destinations.

```html
<LINK=53, Desert of Skulls>
<LINK=35, River Larret>
<LINK=45, Larton Marshlands>
```

These become link buttons with emojis (1️⃣, 2️⃣, etc.) during gameplay.

> 📌 Don't forget to connect **your location to the rest of the game** once it is complete and tested. Usually this means updating an existing location to point to yours via a `<LINK>`.

---

## ✍️ Writing Style

- Write in **second person present tense**: *"You push aside the curtain..."*
- Keep paragraphs concise but vivid
- Use concrete imagery and specific detail
- Avoid overly modern phrasing unless deliberate

You can refer to the [Narrative Style Guide](narrative-style-guide.md) for more advice.

---

## 🧪 Testing Your Location

Once written, test your location:

1. Click **Save** in the admin panel
2. Visit the location via its ID (e.g. `/admin teleport 8712`)
3. Try each link and interaction
4. Watch for:
   - Broken tags
   - Dead ends
   - Spelling/grammar issues

Peer review by another writer or moderator is **strongly encouraged** before connecting it to live content.

---

## ✅ Best Practices

- **Never orphan** a location — ensure it’s accessible from somewhere
- Use `<time>` and `<eat>` tags when resting or travelling overnight
- Flag experimental or work-in-progress content so others know

---

Once your location is stable, narratively sound, and technically clean — you can link it into the main world!

Ready to build? Pick a number, and begin your story.

