"""
Sistema un personaggio glTF/GLB con scheletro prima dell'import in Unreal.

Alcuni GLB (es. quelli di Sketchfab) mettono la rotazione Y-up -> Z-up e la posizione su nodi
genitori vuoti: Interchange li ignora per le mesh con scheletro e le parti arrivano storte o
staccate. Qui Blender applica quelle trasformazioni e riesporta un FBX pulito.

Uso (Blender 4.x, senza interfaccia):
    blender -b -P tools/fix_character_blender.py -- <input.glb> <output.fbx>
"""

import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:]
src, dst = args[0], args[1]

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)

# Stacca tutto dai genitori mantenendo la posizione in mondo, poi applica rotazione e scala:
# armature e mesh restano dove si vedono nel viewer, ma con trasformazioni pulite.
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.parent_clear(type="CLEAR_KEEP_TRANSFORM")
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

# La mesh torna figlia della sua armatura (serve all'export FBX), senza spostarsi.
for obj in list(bpy.data.objects):
    armature = obj.find_armature() if obj.type == "MESH" else None
    if armature:
        world = obj.matrix_world.copy()
        obj.parent = armature
        obj.matrix_world = world

for obj in list(bpy.data.objects):
    if obj.type not in {"MESH", "ARMATURE"}:
        bpy.data.objects.remove(obj, do_unlink=True)

bpy.ops.export_scene.fbx(
    filepath=dst,
    object_types={"ARMATURE", "MESH"},
    add_leaf_bones=False,
    apply_scale_options="FBX_SCALE_ALL",
    bake_anim=False,
    path_mode="COPY",
    embed_textures=True,
)
print(f"[Ricovero] Personaggio sistemato: {dst}")
