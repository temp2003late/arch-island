# Artwork and implementation

The scene is an original generated environment inspired by the supplied concept's atmosphere. It is not a crop of the reference screenshot and contains no UI, labels, numbers or charts. All controls, values, plots and interactive overlays are rendered by Qt.

Earlier artwork used the built-in `image_gen` tool. The current 4K panoramas were supplied by the owner after generation through their API workflow. No image-generation or network API is used by the application at runtime.

## Current 4K scene · 2026-10-24

- `assets/art/island.png`: supplied daylight master, 3840 × 2160 RGB PNG.
- `assets/art/island_night.png`: supplied night lighting edit, same dimensions.
- `assets/art/scene-mask.png`: code-generated 1920 × 1080 mask; R is water, G is attached palm crowns, B is unused. Geometry is traced in a 1600 × 900 logical world. The original panoramas are embedded unchanged.
- The sailboat uses the existing transparent `assets/art/sailboat.png` sprite, embedded in the executable, with gentle bobbing and roll from the shared scene clock.

The day prompt refined Mediterranean limestone, terracotta, palms, cliffs and clear coastal water, requesting preservation of the composition and excluding UI, text and boats. The night prompt requested illumination-only changes, deep blue ambient light, warm existing windows and lighthouse lantern, sparse stars and no visible moon. The owner supplied the final files; exact API request settings and usage are not recorded in the repository.

Native edge checks measure night offsets of (2, −1) px at the residence, (5, −1) at the lighthouse, (3, 0) at the church, (2, 0) at the bridge, (4, −1) at the crane and (4, 0) at a foreground rock. The GPU shader smoothly varies its sampling correction across the image. This reduces registration errors; it does not establish pixel-identical geometry. Software rendering uses the uncorrected supplied pair.

The previous panoramas below are historical generation records and are no longer loaded or required by the build.

## Historical assets

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

## Arthelm native interface · 2026-10-21

Arch Island is presented as an Arthelm product in development. The interface follows the supplied brand brief and the local Arthelm website's brand tokens: navy `#102338`, deep navy `#081827`, ivory `#f1ede5`, surface `#e8e3d9`, brass `#ceb47d`, muted brass `#b89a65`, and secondary text `#526171`. `qml/Theme.qml` contains the shared native UI palette. Restrained secondary colors distinguish the measured resources; the scene labels use muted colors on dark navy.

The ivory header pairs the Pixel Sail mark and Arthelm wordmark with a quiet system-monitor descriptor; the separate product-name/development badge has been removed. Resource panels use warm light surfaces, narrow rules, and moderate corner radii. Drawers and settings use navy surfaces and brass focus/selection accents. The existing day/night panoramas and animation geometry are preserved.

- `assets/brand/arthelm.webp`: copied unchanged from the owner's local `Arthelm/arthelm-website/public/brand-light.webp`. This is a brand asset, not a newly generated mark.
- `assets/fonts/inter.ttf`: Inter variable font from [Google Fonts](https://github.com/google/fonts/tree/main/ofl/inter).
- `assets/fonts/fraunces.ttf`: Fraunces variable font from [Google Fonts](https://github.com/google/fonts/tree/main/ofl/fraunces).
- The corresponding `*-OFL.txt` files preserve the SIL Open Font License and copyright notices. Both fonts are embedded in the executable, with no runtime network requests.

`docs/arch-island.png` is an actual Qt application capture, using the offscreen software backend; it is not a product illustration or a conceptual mockup.


### Beacon interface reference

`arthelm_beacon.png` is the supplied UI reference for the Arthelm Beacon layout. Its artwork is not embedded in the executable. The implementation adapts its ivory panels, navy navigation, brass accents, serif brand heading, inline metric sparklines and light footer into native QML controls. The original animated island panoramas remain in use; the reference landscape would require new aligned artwork and animation masks. Filesystem occupancy replaces the misleading “Storage Health” caption; no fabricated process count, active-user count or health status is displayed.

## Beacon scene completion · 2026-10-24

Built-in imagegen produced `assets/art/beacon-day.png` and `assets/art/beacon-night.png`. These earlier Beacon variants have now been superseded by the supplied 4K pair. The new artwork adapts the reference residence while preserving the existing shoreline and village layout. QML renders all labels, connectors, signal-route lights and interactions. The summit is excluded from surface displacement; volcanic effects have been removed. The night shader uses the new moon position and no longer applies the old tower registration offset.

Day prompt (inputs: original daylight panorama as edit target, `arthelm_beacon.png` as style reference):

> Use case: precise-object-edit. Create project asset beacon-day.png. Image 1 is the edit target panorama; image 2 is style reference only. Preserve image 1 exact 2:1 framing and spatial positions of all shorelines, docks, lighthouse, village, palms, ocean. Replace ONLY the erupting volcano and its lava/smoke in upper left-center with a rugged rocky peak topped by an elegant warm-lit Mediterranean stone residence like the peak residence in image 2. Absolutely no volcano, lava, smoke plume. Golden late-afternoon lighting. No UI, text, badges, routes, boats or frames. Keep all existing buildings elsewhere pixel aligned. Return full panoramic artwork, not application mockup.

Night prompt (input: generated Beacon daylight panorama):

> Use case: lighting-weather. Edit target: supplied panorama. Create its exact aligned nighttime counterpart for a live application crossfade. Change ONLY illumination and sky to calm deep blue moonlight, subtle stars and warm lit windows, brightest lighthouse lantern and summit residence windows. Preserve every pixel position of ALL architecture, summit residence, cliffs, palms, docks, horizon and ocean. Same 2:1 full panorama framing and dimensions. No volcano, lava, smoke, boats, UI, labels, light routes, text, or border. Keep architecture crisp and sea readable, cinematic refined nighttime scene.

The prompts request alignment; generated textures can differ in fine details. The shared geometric layout and transition are visually inspected, not claimed to be pixel-identical. Existing masks remain conservative around the original shoreline, with an explicit stationary guard around the new summit.

## Complete nighttime interface

The same `monitor.night` state now drives both the landscape and application chrome. `Theme.qml` supplies semantic panel, card, text, muted text, separator and chart colors; metric cards and header icon buttons receive the active mode explicitly. Night panels use `#0d1c2b`, cards `#142638`, text `#e1e7eb` and secondary text `#a6b5c2`. Day mode retains the ivory palette. Automatic mode uses the existing local-time schedule.

`BrandMark` draws the original raster mark with its white matte removed in memory on both hardware and software rendering. The header uses navy ink in day mode and ivory ink at night; the dark navigation rail always uses ivory ink. Its background stays transparent and matches the underlying panel. Dialogs and the processes drawer retain their existing dark surfaces.

Theme colors and header logo ink use the same `Island.lightingBlend` as the day/night panorama. A one-second linear driver feeds an irregular stepped curve with holds, short jumps and small rebounds. A few brief pulses offset the chrome by 1–1.5 pixels and draw restrained cyan/magenta seams and scan fragments. The effect is idle after the switch and does not intercept input. Disabling decorative animation removes the glitch pulses and uses a simple one-second fade. All panels, cards, text, icons and chart accents share the same progress; reversing a transition preserves its current value. Brand ink blends the cached variants in a small premultiplied buffer, preserving edge transparency.
