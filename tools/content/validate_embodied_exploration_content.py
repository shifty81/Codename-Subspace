#!/usr/bin/env python3
import json, sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'content'
TARGETS=[BASE/'gameplay'/'embodiment',BASE/'interiors',BASE/'planets',BASE/'transitions',BASE/'vertical_slice',BASE/'items',BASE/'loot',BASE/'salvage',BASE/'characters',BASE/'vehicles',BASE/'settlements',BASE/'contracts',BASE/'economy',BASE/'encounters']
objs=[]
for d in TARGETS:
    if not d.exists():
        print(f'[FAIL] missing content directory: {d.relative_to(ROOT)}'); sys.exit(2)
    for p in d.rglob('*.json'):
        try: obj=json.loads(p.read_text(encoding='utf-8'))
        except Exception as e:
            print(f'[FAIL] invalid json {p.relative_to(ROOT)}: {e}'); sys.exit(3)
        if not isinstance(obj,dict) or not obj.get('id'):
            print(f'[FAIL] missing id: {p.relative_to(ROOT)}'); sys.exit(4)
        objs.append((p,obj))
ids={}
for p,o in objs:
    if o['id'] in ids:
        print(f'[FAIL] duplicate id {o["id"]}: {ids[o["id"]]} and {p.relative_to(ROOT)}'); sys.exit(5)
    ids[o['id']]=str(p.relative_to(ROOT))
required=['player.human.standard.v1','interior.ship.pioneer_frigate.v1','interior.station.wayline_k17_hangar.v1','planet.archetype.cinderwake.v1','transition.space_to_planet_landing.v1','transition.planet_to_space_takeoff.v1','vertical_slice.embodied_exploration.v1','expedition.first_descent.v1','content.embodied_exploration.r83_r112.v2','vehicles.surface_exploration.v1','salvage.modules.frontier.v1','contracts.cinderwake.v1']
missing=[x for x in required if x not in ids]
if missing:
    print('[FAIL] required IDs missing: '+', '.join(missing)); sys.exit(6)
# graph checks
pg=next(o for _,o in objs if o['id']=='portal_graph.ship.pioneer_frigate.v1')
layout=next(o for _,o in objs if o['id']=='interior.ship.pioneer_frigate.v1')
rooms={x['id'] for x in layout['rooms']}
adj={x:set() for x in rooms}
for e in pg['portals']:
    a,b=e['a'],e['b']
    if a in rooms and b in rooms: adj[a].add(b); adj[b].add(a)
seen={'cockpit'}; q=['cockpit']
while q:
    n=q.pop(0)
    for x in adj[n]:
        if x not in seen: seen.add(x); q.append(x)
if seen!=rooms:
    print('[FAIL] disconnected starter ship rooms: '+', '.join(sorted(rooms-seen))); sys.exit(7)
planet=next(o for _,o in objs if o['id']=='planet.archetype.cinderwake.v1')
for ref in [planet['biomeRegistry'],planet['resourceRegistry'],planet['landingSites']]:
    if ref not in ids: print(f'[FAIL] planet reference missing: {ref}'); sys.exit(8)
vs=next(o for _,o in objs if o['id']=='vertical_slice.embodied_exploration.v1')
for ref in [vs['shipInterior'],vs['stationInterior'],vs['planet'],*vs['requiredTransitions']]:
    if ref not in ids: print(f'[FAIL] vertical slice reference missing: {ref}'); sys.exit(9)
print(f'[PASS] Embodied exploration content: {len(objs)} JSON assets / {len(ids)} unique IDs')
print(f'[PASS] Pioneer interior connectivity: {len(rooms)} rooms reachable from cockpit')
print('[PASS] Cinderwake planet registries and vertical-slice references resolve')
print('[PASS] R98-R112 surface gameplay, NPC, vehicle, economy and contract content is indexed')
