# `<macro>` Tag

The `<macro>` tag allows the embedding of one paragraph into another within the Seven Spells of Destruction engine. This is useful for reusing shared text or logic across multiple locations without duplicating content.

---

## Purpose

The `<macro>` tag acts as a reference to another paragraph ID, and includes its rendered content and navigation links as if it were part of the original.

---

## Syntax

```html
<macro ID>
```

Where `ID` is the numeric ID of the paragraph to include.

---

## Behaviour

- Loads the paragraph identified by the provided ID.
- Renders its contents directly in place within the calling paragraph.
- Merges its `navigation_links` into the parent paragraph's list.
- Useful for consistent flavour text, notices, warnings, shared directions, or branching logic used in many locations.

---

## Example

```html
<macro 1245>
```

Includes the full rendered text of paragraph ID 1245 in place.

---

## Technical Notes

- This tag is resolved asynchronously and executes the embedded paragraph rendering logic.
- Navigation links are appended to the current paragraph’s navigation structure.
- Avoid circular macros (e.g., paragraph A includes B, which includes A) — while recursion limits are in place, such constructs are discouraged and may result in truncation or failure to render.

---

## Permissions

- Players do not perceive that a macro is in use — it's purely a backend authoring convenience.
- The `macro` tag supports any paragraph, even those gated by race, flags, or combat results.

---

## Summary

Use `<macro>` to improve reusability and keep your story logic DRY (Don’t Repeat Yourself). It makes authoring large linked narratives more maintainable by embedding reusable logic or flavour content where needed.
