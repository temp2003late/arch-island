# Artwork and implementation

The scene is an original generated environment inspired by the supplied concept's atmosphere. It is not a crop of the reference screenshot and contains no UI, labels, numbers or charts. All controls, values, plots and interactive overlays are rendered by Qt.

Generation used the built-in `image_gen` tool, not the CLI/API fallback. No image-generation or network API is used by the application at runtime.

## Assets

- `assets/art/island-night.png` — cinematic nighttime environment.
- `assets/art/island-day.png` — lighting edit preserving the nighttime scene's layout.
- `assets/art/sailboat.png` — transparent sprite, independently animated by network activity.
- `assets/icons/*.svg`, `assets/glow.svg`, `assets/smoke.svg`, `assets/cargo.svg`, `assets/beam.svg`, `assets/wake.svg` — code-authored vector graphics and animated overlays.

## Night environment prompt

Use case: stylized-concept. Asset type: production environment artwork for a native desktop application, NOT a UI mockup.
Create one exceptionally beautiful cinematic realistic Caribbean island at night, wide landscape 16:10 composition, ideally 2400x1500 or larger. The reference image is ONLY a mood/composition reference: create original scenic artwork filling the entire canvas. Absolutely NO application frame, panels, text, labels, icons, charts, borders, logos, or interface elements.
Scene: elevated three-quarter view of a lush volcanic island surrounded by deep sapphire ocean and luminous teal shallows, richly layered jagged volcanic cliffs, tiny detailed palm trees and dense tropical greenery. A small terracotta-roof Caribbean colonial village cascades down the central hillside, hundreds of warm amber windows and lanterns along winding stone steps and waterfront terraces. Highly detailed physical materials, wave foam around craggy rocks, intricate water reflections, distant mountainous silhouettes on a low horizon, atmospheric midnight cobalt clouds, a softly luminous moon in the upper right.
Composition needed for interactive overlay alignment: volcano summit near 33% canvas width and 27% canvas height, naturally sculpted slopes descending into village; a tall elegant ivory lighthouse on the right headland with its lantern at approximately 76% canvas width and 40% height; a detailed wooden harbor dock with visible EMPTY deck area around 70% width and 72% height (leave space to render dynamic cargo); open foreground water in lower left at 22% width and 77% height for an independently animated sailboat, but DO NOT PAINT A SAILBOAT. The island occupies middle 75% width and 65% height. Foreground includes beautiful water and foam.
Volcano is resting with a subtly glowing amber crater and a few fine lava seams, NO large fire plume or billowing smoke (will be animated separately). Lighthouse lantern glows warmly but DO NOT PAINT A LONG BEAM (animated separately).
Lighting is breathtaking, cool moonlit blue rock edges and shimmering turquoise bays contrasted with tiny warm golden lights. Premium cinematic matte painting / realistic miniature world, exceptionally detailed and sophisticated. Match the reference's realism, depth and warmth. Avoid flat vector polygons, low-poly look, cartoons, giant buildings, top-down maps, busy UI, oversaturated neon, brown fog. This is an island environment asset with no text anywhere.

## Day environment prompt

Use case: lighting-weather. Asset type: daytime variant of the EXACT same game environment, intended for pixel-aligned day/night crossfade. Edit the provided original island artwork. Preserve camera, framing, aspect ratio, island silhouette, volcano shape and crater coordinates, ALL village buildings, cliff details, individual palm trees, lighthouse geometry and lantern position, docks, crane, cargo, shoreline and sea waves in EXACTLY the same positions. Change ONLY the illumination and atmosphere from midnight to beautiful warm Caribbean late-afternoon sunlight: rich natural green palms, sunlit ivory buildings and terracotta roofs, deep blue open ocean and luminous clear turquoise shallows, soft warm cream sunbeams from upper right, atmospheric distant blue mountains and wisps of clouds. Replace the moon with subtle hazy sky (no prominent sun disc). Lamps are dim/off in daylight. Preserve a little orange heat in the volcano crater, no extra eruption. Sophisticated cinematic physically textured environmental artwork, NOT flat vector art, not cartoon, not oversaturated. Absolutely no text, UI, borders, icons, or boats. Maintain image dimensions and exact composition so all interactive positions line up between both versions.

## Sailboat prompt

Use case: stylized-concept. Asset type: a single transparent PNG game sprite to composite over the water of a detailed cinematic Caribbean island. One beautiful small classic Caribbean sailing yacht, ivory triangular mainsail and smaller foresail on one tall wooden mast, mahogany hull, subtle golden lantern glow along the deck, delicate detailed rigging. Viewed from elevated three-quarter front-side perspective, bow pointing toward the RIGHT, waterline hull horizontal with slight perspective, boat seen from above just enough to see the deck. Realistic miniature painted physical detail, cool moonlit cream sailcloth, warm brown varnished wood, sophisticated realism. The entire yacht including mast and hull must be visible with a 5% margin. Square canvas, yacht occupies most of height. Genuinely TRANSPARENT background and no scene, no ocean rectangle, no waves, no shadow plane, no text, no people, no labels, no logos. This sprite will be displayed about 115 pixels high over a nighttime ocean illustration so clean readable silhouette and fine material shading are essential. No vector/flat cartoon look.
