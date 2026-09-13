#!/usr/bin/env python3
"""Materialize or recover the governed Various Planets runtime texture pack.

The licensed source GLB remains the provenance authority. The materializer can:
1. verify an already-materialized current-project pack,
2. recover a previously materialized hash-verified pack from a nearby/local cache,
3. extract embedded images from an approved local ``various_planets.glb``.

It deliberately does not download the Sketchfab source itself because Sketchfab
downloads require authenticated user authorization. Local recovery is bounded to
well-known project/user locations and every recovered pack is hash-verified
before publication.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import sys
from typing import Any, Iterable

GLB_MAGIC = 0x46546C67
JSON_CHUNK = 0x4E4F534A
BIN_CHUNK = 0x004E4942
EXPECTED_RUNTIME_IMAGES = {
    "image_00.png", "image_03.png", "image_05.jpg", "image_07.png",
    "image_09.jpg", "image_12.png", "image_14.jpg", "image_17.jpg",
    "image_21.jpg", "image_24.png",
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def extension_for(mime: str, data: bytes) -> str:
    mime = (mime or "").lower()
    if "png" in mime or data.startswith(b"\x89PNG\r\n\x1a\n"):
        return ".png"
    if "jpeg" in mime or "jpg" in mime or data.startswith(b"\xff\xd8\xff"):
        return ".jpg"
    if "webp" in mime or data[:4] == b"RIFF" and data[8:12] == b"WEBP":
        return ".webp"
    return ".bin"


def load_glb(path: Path) -> tuple[dict[str, Any], bytes]:
    raw = path.read_bytes()
    if len(raw) < 12:
        raise ValueError("GLB is shorter than its 12-byte header")
    magic, version, declared = struct.unpack_from("<III", raw, 0)
    if magic != GLB_MAGIC:
        raise ValueError("source is not a binary glTF/GLB file")
    if version != 2:
        raise ValueError(f"unsupported GLB version {version}; expected 2")
    if declared > len(raw):
        raise ValueError("GLB declared length exceeds available bytes")

    offset = 12
    doc: dict[str, Any] | None = None
    binary = b""
    while offset + 8 <= declared:
        length, kind = struct.unpack_from("<II", raw, offset)
        offset += 8
        payload = raw[offset:offset + length]
        offset += length
        if kind == JSON_CHUNK:
            doc = json.loads(payload.rstrip(b" \t\r\n\x00").decode("utf-8"))
        elif kind == BIN_CHUNK and not binary:
            binary = payload
    if doc is None:
        raise ValueError("GLB contains no JSON chunk")
    return doc, binary


def embedded_image_bytes(
    doc: dict[str, Any],
    binary: bytes,
    image: dict[str, Any],
    source_dir: Path,
) -> tuple[bytes, str]:
    if "bufferView" in image:
        views = doc.get("bufferViews", [])
        index = int(image["bufferView"])
        if index < 0 or index >= len(views):
            raise ValueError(f"image bufferView {index} is outside bufferViews")
        view = views[index]
        if int(view.get("buffer", 0)) != 0:
            raise ValueError("only the primary embedded GLB buffer is supported")
        start = int(view.get("byteOffset", 0))
        length = int(view.get("byteLength", 0))
        end = start + length
        if start < 0 or length <= 0 or end > len(binary):
            raise ValueError("image bufferView is outside binary GLB chunk")
        return binary[start:end], str(image.get("mimeType", ""))

    uri = str(image.get("uri", ""))
    if uri and not uri.startswith("data:"):
        external = (source_dir / uri).resolve()
        if not external.is_file():
            raise ValueError(f"external image URI is missing: {uri}")
        return external.read_bytes(), str(image.get("mimeType", ""))
    raise ValueError("data-URI images are not accepted by the governed offline materializer")


def _dedupe_paths(values: Iterable[Path]) -> list[Path]:
    seen: set[str] = set()
    out: list[Path] = []
    for candidate in values:
        candidate = candidate.expanduser()
        try:
            key = str(candidate.resolve())
        except OSError:
            key = str(candidate)
        key = os.path.normcase(key)
        if key not in seen:
            seen.add(key)
            out.append(candidate)
    return out


def _nearby_subspace_roots(root: Path) -> list[Path]:
    """Return only immediate sibling project roots; never recurse a whole drive."""
    siblings: list[Path] = []
    parent = root.parent
    try:
        for child in parent.iterdir():
            if not child.is_dir() or child == root:
                continue
            normalized = child.name.lower().replace("-", "").replace("_", "")
            if "subspace" in normalized or "nullharbor" in normalized:
                siblings.append(child)
    except OSError:
        pass
    return siblings


def source_candidates(root: Path, explicit: str | None) -> list[Path]:
    values: list[Path] = []
    if explicit:
        values.append(Path(explicit))

    env = os.environ.get("SUBSPACE_PLANET_PACK_SOURCE", "").strip()
    if env:
        values.append(Path(env))

    values.extend([
        root / "various_planets.glb",
        root / "content/source/licensed/various_planets/various_planets.glb",
        root / "content/source/third_party/various_planets/various_planets.glb",
        root / "assets/source/licensed/various_planets.glb",
        root / "Assets/source/licensed/various_planets.glb",
        root / "GameData/Assets/Source/various_planets.glb",
    ])

    home = Path.home()
    values.extend([
        home / "Downloads/various_planets.glb",
        home / "Desktop/various_planets.glb",
    ])

    # Prior Subspace working copies are a useful local source cache during repo
    # normalization/renames. Keep this bounded to immediate siblings only.
    for sibling in _nearby_subspace_roots(root):
        values.extend([
            sibling / "various_planets.glb",
            sibling / "content/source/licensed/various_planets/various_planets.glb",
            sibling / "content/source/third_party/various_planets/various_planets.glb",
            sibling / "assets/source/licensed/various_planets.glb",
        ])

    return _dedupe_paths(values)


def pack_candidates(root: Path, explicit: str | None) -> list[Path]:
    values: list[Path] = []
    if explicit:
        values.append(Path(explicit))

    env = os.environ.get("SUBSPACE_PLANET_PACK_CACHE", "").strip()
    if env:
        values.append(Path(env))

    # Common local governed-cache positions.
    home = Path.home()
    values.extend([
        home / "Downloads/various_planets_v1",
        home / "Desktop/various_planets_v1",
    ])

    # Reuse a verified pack from an older/sibling Subspace checkout when one is
    # available. This is especially useful after project-root renames.
    for sibling in _nearby_subspace_roots(root):
        values.append(sibling / "content/derived/various_planets_v1")

    return _dedupe_paths(values)


def is_ready(output: Path) -> bool:
    marker = output / "VARIOUS_PLANETS_READY.txt"
    manifest_path = output / "planet_pack_manifest.json"
    if not marker.is_file() or not manifest_path.is_file():
        return False
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        if manifest.get("schema") != "subspace.various-planets-pack.v1":
            return False
        files = manifest.get("images", [])
        if not files:
            return False
        for entry in files:
            rel = str(entry.get("path", ""))
            p = output / rel
            if not rel or not p.is_file() or sha256(p) != entry.get("sha256"):
                return False
        return all((output / "textures" / name).is_file() for name in EXPECTED_RUNTIME_IMAGES)
    except (OSError, ValueError, json.JSONDecodeError):
        return False


def recover_pack(source_pack: Path, output: Path) -> dict[str, Any]:
    if not is_ready(source_pack):
        raise ValueError(f"candidate pack is not hash-verified: {source_pack}")

    staging = output.with_name(output.name + ".staging")
    if staging.exists():
        shutil.rmtree(staging)
    shutil.copytree(source_pack, staging)

    if not is_ready(staging):
        shutil.rmtree(staging, ignore_errors=True)
        raise ValueError("copied pack failed post-copy verification")

    if output.exists():
        shutil.rmtree(output)
    staging.replace(output)
    manifest = json.loads((output / "planet_pack_manifest.json").read_text(encoding="utf-8"))
    return manifest


def materialize(source: Path, output: Path) -> dict[str, Any]:
    doc, binary = load_glb(source)
    images = doc.get("images", [])
    if not images:
        raise ValueError("GLB contains no images")

    staging = output.with_name(output.name + ".staging")
    if staging.exists():
        shutil.rmtree(staging)
    textures = staging / "textures"
    textures.mkdir(parents=True, exist_ok=True)

    entries: list[dict[str, Any]] = []
    for index, image in enumerate(images):
        data, mime = embedded_image_bytes(doc, binary, image, source.parent)
        ext = extension_for(mime, data)
        if ext == ".bin":
            raise ValueError(f"image {index} has unsupported/unknown encoding")
        name = f"image_{index:02d}{ext}"
        target = textures / name
        target.write_bytes(data)
        entries.append({
            "index": index,
            "path": f"textures/{name}",
            "mimeType": mime or ("image/png" if ext == ".png" else "image/jpeg"),
            "bytes": len(data),
            "sha256": sha256(target),
        })

    missing = sorted(name for name in EXPECTED_RUNTIME_IMAGES if not (textures / name).is_file())
    if missing:
        raise ValueError("GLB did not materialize required runtime images: " + ", ".join(missing))

    manifest = {
        "schema": "subspace.various-planets-pack.v1",
        "sourceFile": source.name,
        "sourceSha256": sha256(source),
        "license": "CC-BY-4.0",
        "provider": "Various Planets / feivelyn",
        "imageCount": len(entries),
        "requiredRuntimeImages": sorted(EXPECTED_RUNTIME_IMAGES),
        "images": entries,
    }
    (staging / "planet_pack_manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (staging / "VARIOUS_PLANETS_READY.txt").write_text(
        "Various Planets runtime pack materialized and hash-verified from the governed source GLB.\n",
        encoding="utf-8",
    )
    if output.exists():
        shutil.rmtree(output)
    staging.replace(output)
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True)
    parser.add_argument("--source")
    parser.add_argument("--pack-cache")
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    output = root / "content/derived/various_planets_v1"

    if is_ready(output):
        print(f"[PASS] Various Planets runtime pack verified: {output}")
        return 0
    if args.verify_only:
        print(f"[FAIL] Various Planets runtime pack is incomplete: {output}", file=sys.stderr)
        return 2

    # First prefer a previously materialized, hash-verified pack. This avoids
    # forcing users to retain large licensed GLBs inside every source checkout.
    for candidate in pack_candidates(root, args.pack_cache):
        if candidate == output:
            continue
        if not is_ready(candidate):
            continue
        try:
            manifest = recover_pack(candidate.resolve(), output)
        except Exception as exc:
            print(f"[WARN] Could not recover verified Various Planets pack from {candidate}: {exc}", file=sys.stderr)
            continue
        print(f"[PASS] Recovered verified Various Planets runtime pack from {candidate}")
        if manifest.get("sourceSha256"):
            print(f"[PASS] Original source SHA-256: {manifest['sourceSha256']}")
        return 0

    source = next((p.resolve() for p in source_candidates(root, args.source) if p.is_file()), None)
    if source is None:
        print("[FAIL] Various Planets visual source/cache is unavailable.", file=sys.stderr)
        print("       Checked current project, Downloads/Desktop, and sibling Subspace project roots.", file=sys.stderr)
        print("       Put various_planets.glb in the project root or a governed source folder,", file=sys.stderr)
        print("       set SUBSPACE_PLANET_PACK_SOURCE to the licensed GLB, or set", file=sys.stderr)
        print("       SUBSPACE_PLANET_PACK_CACHE to a verified various_planets_v1 pack.", file=sys.stderr)
        return 3

    try:
        manifest = materialize(source, output)
    except Exception as exc:  # fail closed with a readable gate reason
        print(f"[FAIL] Could not materialize Various Planets pack: {exc}", file=sys.stderr)
        return 4

    print(f"[PASS] Materialized {manifest['imageCount']} Various Planets images from {source}")
    print(f"[PASS] Source SHA-256: {manifest['sourceSha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
