from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path


def canonical(v):
    x, y, z = v
    return x, z, y


def profile(path: Path):
    vertices = []
    edge_use = Counter()
    with path.open('r', encoding='utf-8', errors='ignore') as fh:
        for line in fh:
            if line.startswith('v '):
                p = line.split()
                if len(p) >= 4:
                    vertices.append(canonical(tuple(map(float, p[1:4]))))
            elif line.startswith('f '):
                face = []
                for token in line.split()[1:]:
                    head = token.split('/', 1)[0]
                    if not head:
                        continue
                    idx = int(head)
                    idx = len(vertices) + idx if idx < 0 else idx - 1
                    if 0 <= idx < len(vertices):
                        face.append(idx)
                if len(face) >= 3:
                    for a, b in zip(face, face[1:] + face[:1]):
                        if a != b:
                            edge_use[tuple(sorted((a, b)))] += 1
    if not vertices:
        return None
    mn = [min(v[i] for v in vertices) for i in range(3)]
    mx = [max(v[i] for v in vertices) for i in range(3)]
    dims = [max(1e-9, mx[i] - mn[i]) for i in range(3)]
    boundary = [e for e, count in edge_use.items() if count == 1]
    faces = ((0, 0, 'port'), (0, 1, 'starboard'),
             (1, 0, 'aft'), (1, 1, 'forward'),
             (2, 0, 'ventral'), (2, 1, 'dorsal'))
    scores = {name: 0 for _, _, name in faces}
    for a, b in boundary:
        mid = [(vertices[a][i] + vertices[b][i]) * 0.5 for i in range(3)]
        for axis, high, name in faces:
            extreme = mx[axis] if high else mn[axis]
            if abs(mid[axis] - extreme) <= dims[axis] * 0.08:
                scores[name] += 1
    face = ''
    confidence = 0.0
    if boundary:
        face, count = max(scores.items(), key=lambda item: item[1])
        confidence = count / len(boundary)
        threshold = 0.08 if face == 'ventral' else 0.12
        if count < 4 or confidence < threshold:
            face, confidence = '', 0.0
    return dims, len(boundary), face, confidence, scores


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('modules_dir', type=Path)
    ap.add_argument('--csv', dest='csv_path', type=Path)
    args = ap.parse_args()
    rows = []
    for p in sorted(args.modules_dir.glob('*.obj')):
        result = profile(p)
        if not result:
            continue
        dims, boundary_count, face, confidence, scores = result
        largest = 'XYZ'[max(range(3), key=lambda i: dims[i])]
        rows.append({
            'module': p.stem,
            'width_x': f'{dims[0]:.6f}',
            'length_y': f'{dims[1]:.6f}',
            'height_z': f'{dims[2]:.6f}',
            'largest_axis': largest,
            'boundary_edges': boundary_count,
            'attachment_face_hint': face,
            'attachment_confidence': f'{confidence:.6f}',
            **{f'open_{k}': v for k, v in scores.items()},
        })
    fields = list(rows[0]) if rows else []
    if args.csv_path:
        args.csv_path.parent.mkdir(parents=True, exist_ok=True)
        with args.csv_path.open('w', encoding='utf-8', newline='') as fh:
            w = csv.DictWriter(fh, fieldnames=fields)
            w.writeheader(); w.writerows(rows)
    hulls = [r for r in rows if '_hull_' in r['module']]
    print(f'modules={len(rows)}')
    print(f'hulls={len(hulls)} hulls_longest_Y={sum(r["largest_axis"] == "Y" for r in hulls)}')
    candidates = [r for r in rows if r['attachment_face_hint']]
    ventral = [r for r in candidates if r['attachment_face_hint'] == 'ventral']
    print(f'open_face_candidates={len(candidates)} ventral_candidates={len(ventral)}')
    for r in ventral:
        print(f'VENTRAL {r["module"]} confidence={r["attachment_confidence"]}')


if __name__ == '__main__':
    main()
