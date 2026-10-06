# Seamless panorama assets

Generated with the built-in image_gen tool on 2026-10-06. Original artwork is retained as a reference; the application loads these complete 2:1 panoramas without mirrored edge tiles:

- `assets/art/island-night-wide.png` — 1774 × 887, RGB PNG.
- `assets/art/island-day-wide.png` — 1774 × 887, RGB PNG.

The resulting landscape contains additional sky and sea. Overlay coordinates were adjusted to the generated composition rather than assuming pixel-perfect preservation of the source. Daytime was derived from the night panorama to keep the scene aligned.

## Night prompt

Input: `assets/art/island-night.png`.

```text
Use case: precise-object-edit / outpainting. Edit target: attached night island artwork. Produce a seamless wider landscape panorama, aspect ratio 2:1, high-resolution ideally 2048x1024 or larger. Preserve the original island, architecture, volcano, lighthouse, single moon, harbor and all positions and relative scale EXACTLY in the central 80% of output width, full output height. Original image spans x=10% through x=90% of the new canvas. ONLY extend horizontally left and right with 10% extra natural ocean, horizon and night sky on each side. Continue waves and clouds organically, unique asymmetrical details. No mirrored/repeated rocks, no repeated pier, no tiles, no seams, no black bands, no stretch, no new buildings or boats, no text. Keep original crisp fine detail, moonlight blue ocean and amber warm windows unchanged. The original image top and bottom remain unchanged and the original original moon remains fully visible.
```

## Day prompt

Inputs: the generated night panorama (edit target) and `assets/art/island-day.png` (lighting reference).

```text
Use case: lighting-weather. Edit target IMAGE 1: the new wide night island panorama. Supporting reference IMAGE 2: daytime palette and lighting only. Make IMAGE 1 daytime, preserving EXACTLY its 2:1 frame, island size and placement, every building, lighthouse, harbor, volcano, cliffs, shoreline, mountains, sea wave layout, and sky spacing. Change illumination only to clear warm late afternoon daylight like image 2, turquoise shallows, blue ocean, green palms, sunlit stone. Remove night moon and stars, keep realistic white clouds. Warm lava stays active. Keep single coherent panorama edge to edge, all detail crisp; no repeated or mirrored edge content, no seams, no strips, no boats, no text. Do not zoom, reposition, expand or crop image 1. Output high-resolution 2:1 landscape.
```
