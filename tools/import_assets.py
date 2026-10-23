"""
Importa in Unreal gli asset scaricati in Assets_Raw/ da tools/download_assets.py.

Da eseguire nell'editor (File -> Esegui script Python). Per ogni asset:
  - modelli (glb/gltf/fbx) -> /Game/Ricovero/Models/<Categoria>/<Nome>/SM_<Nome>,
    con Nanite attivo e collisione semplice a scatola; quelli con scheletro diventano SK_<Nome>;
  - texture PBR -> /Game/Ricovero/Textures/<Categoria>/<Nome>/T_<Nome>_<D|N|ORM|R|AO|H>,
    con sRGB e compressione corretti, più un Material Instance MI_<Nome>.
Crea anche i master material (se mancano):
  M_Surface_Master       BaseColor + Normal + ORM (R=AO, G=Roughness, B=Metallic)
  M_Surface_Master_Split BaseColor + Normal + Roughness + AO, metallic scalare
Gli asset già importati vengono saltati (file .imported nella cartella grezza).
"""

import json
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
RAW = ROOT / "Assets_Raw"
MATERIALS_DIR = "/Game/Ricovero/Materials"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary

MODEL_EXTS = (".glb", ".gltf", ".fbx")
TEXTURE_EXTS = (".jpg", ".jpeg", ".png", ".tga", ".exr")

# Riconoscimento delle mappe: suffissi Poly Haven (rinominati dal downloader o originali) e ambientCG.
MAP_PATTERNS = [
    ("ORM", ("_ORM.", "_arm_")),
    ("N", ("_N.", "_NormalGL.", "_nor_gl_")),
    ("D", ("_D.", "_Color.", "_diff_")),
    ("R", ("_R.", "_Roughness.", "_rough_")),
    ("AO", ("_AO.", "_AmbientOcclusion.", "_ao_")),
    ("H", ("_H.", "_Displacement.", "_disp_")),
]


def classify_texture(filename):
    for suffix, patterns in MAP_PATTERNS:
        if any(p.lower() in filename.lower() for p in patterns):
            return suffix
    return None


def import_files(files, destination, names=None):
    tasks = []
    for i, path in enumerate(files):
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(path))
        task.set_editor_property("destination_path", destination)
        if names:
            task.set_editor_property("destination_name", names[i])
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        tasks.append(task)
    asset_tools.import_asset_tasks(tasks)
    imported = []
    for task in tasks:
        imported.extend(task.get_editor_property("imported_object_paths"))
    return imported


# --- Master material -----------------------------------------------------------

def texture_param(material, name, sampler, x, y, uv, default):
    # Il default deve avere lo stesso tipo del sampler, altrimenti il materiale non compila
    # (la texture di default del motore è sRGB): usiamo una texture vera del primo set importato.
    node = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("texture", default)
    node.set_editor_property("sampler_type", sampler)
    mel.connect_material_expressions(uv, "", node, "UVs")
    return node


def ensure_master(name, split, textures):
    path = f"{MATERIALS_DIR}/{name}"
    if eal.does_asset_exist(path):
        return eal.load_asset(path)

    material = asset_tools.create_asset(name, MATERIALS_DIR, unreal.Material, unreal.MaterialFactoryNew())

    coords = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -900, 0)
    tiling = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -900, 120)
    tiling.set_editor_property("parameter_name", "Tiling")
    tiling.set_editor_property("default_value", 1.0)
    uv = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -650, 40)
    mel.connect_material_expressions(coords, "", uv, "A")
    mel.connect_material_expressions(tiling, "", uv, "B")

    base = texture_param(material, "BaseColor", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, -400, -300, uv, textures["D"])
    mel.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)

    normal = texture_param(material, "Normal", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, -400, 0, uv, textures["N"])
    mel.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)

    if split:
        rough = texture_param(material, "Roughness", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE, -400, 300, uv, textures["R"])
        mel.connect_material_property(rough, "R", unreal.MaterialProperty.MP_ROUGHNESS)
        ao = texture_param(material, "AO", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE, -400, 600, uv, textures.get("AO", textures["R"]))
        mel.connect_material_property(ao, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
        metallic = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -400, 900)
        metallic.set_editor_property("parameter_name", "Metallic")
        mel.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    else:
        orm = texture_param(material, "ORM", unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, -400, 300, uv, textures["ORM"])
        mel.connect_material_property(orm, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
        mel.connect_material_property(orm, "G", unreal.MaterialProperty.MP_ROUGHNESS)
        mel.connect_material_property(orm, "B", unreal.MaterialProperty.MP_METALLIC)

    mel.recompile_material(material)
    eal.save_loaded_asset(material)
    unreal.log(f"[Ricovero] Creato {path}")
    return material


# --- Texture -------------------------------------------------------------------

def configure_texture(texture, suffix):
    if suffix == "D":
        texture.set_editor_property("srgb", True)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    elif suffix == "N":
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    elif suffix == "ORM":
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
    else:
        texture.set_editor_property("srgb", False)
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_GRAYSCALE)
    eal.save_loaded_asset(texture)


def import_texture_set(meta, folder):
    name, category = meta["name"], meta["category"]
    destination = f"/Game/Ricovero/Textures/{category}/{name}"

    maps = {}
    for path in sorted(folder.rglob("*")):
        if path.suffix.lower() in TEXTURE_EXTS:
            suffix = classify_texture(path.name)
            if suffix and suffix not in maps:
                maps[suffix] = path
    if "D" not in maps:
        unreal.log_warning(f"[Ricovero] {name}: nessuna mappa colore trovata, salto")
        return False

    names = [f"T_{name}_{suffix}" for suffix in maps]
    import_files(list(maps.values()), destination, names)

    textures = {}
    for suffix in maps:
        texture = eal.load_asset(f"{destination}/T_{name}_{suffix}")
        if texture:
            configure_texture(texture, suffix)
            textures[suffix] = texture

    if "N" not in textures or ("ORM" not in textures and "R" not in textures):
        unreal.log_warning(f"[Ricovero] {name}: mancano normal o roughness, niente material instance")
        return True
    split = "ORM" not in textures
    master = ensure_master("M_Surface_Master_Split" if split else "M_Surface_Master", split, textures)
    mi_path = f"{destination}/MI_{name}"
    if eal.does_asset_exist(mi_path):
        mi = eal.load_asset(mi_path)
    else:
        mi = asset_tools.create_asset(f"MI_{name}", destination, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, master)

    params = {"D": "BaseColor", "N": "Normal", "ORM": "ORM", "R": "Roughness", "AO": "AO"}
    for suffix, param in params.items():
        if suffix in textures:
            mel.set_material_instance_texture_parameter_value(mi, param, textures[suffix])
    eal.save_loaded_asset(mi)
    return True


# --- Modelli -------------------------------------------------------------------

def is_model(meta):
    return meta.get("kind", "").startswith("modello")


def import_model(meta, folder):
    name, category = meta["name"], meta["category"]
    destination = f"/Game/Ricovero/Models/{category}/{name}"

    candidates = [p for p in folder.rglob("*") if p.suffix.lower() in MODEL_EXTS]
    if not candidates:
        return False
    # Preferisce glb, poi gltf, poi fbx.
    candidates.sort(key=lambda p: MODEL_EXTS.index(p.suffix.lower()))
    import_files([candidates[0]], destination)

    assets = [eal.load_asset(p) for p in eal.list_assets(destination, recursive=True)]
    meshes = [m for m in assets if isinstance(m, (unreal.StaticMesh, unreal.SkeletalMesh))]
    subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    for i, mesh in enumerate(meshes):
        # Le mesh con scheletro (personaggi) restano senza Nanite e senza collisione a scatola:
        # la collisione la danno il Physics Asset e la capsule del personaggio.
        skeletal = isinstance(mesh, unreal.SkeletalMesh)
        if not skeletal:
            nanite = mesh.get_editor_property("nanite_settings")
            nanite.set_editor_property("enabled", True)
            mesh.set_editor_property("nanite_settings", nanite)
            subsystem.remove_collisions(mesh)
            subsystem.add_simple_collisions(mesh, unreal.ScriptCollisionShapeType.BOX)
            eal.save_loaded_asset(mesh)

        prefix = "SK" if skeletal else "SM"
        target = f"{prefix}_{name}" if len(meshes) == 1 else f"{prefix}_{name}_{i + 1:02d}"
        old_path = mesh.get_path_name().split(".")[0]
        new_path = f"{destination}/{target}"
        if old_path != new_path and not eal.does_asset_exist(new_path):
            eal.rename_asset(old_path, new_path)

    unreal.log(f"[Ricovero] {name}: {len(meshes)} mesh importate in {destination}")
    return bool(meshes)


# --- Main ----------------------------------------------------------------------

def main():
    if not RAW.exists():
        unreal.log_warning("[Ricovero] Assets_Raw non esiste: esegui prima tools/download_assets.py")
        return

    imported = skipped = failed = 0
    for meta_file in sorted(RAW.rglob("asset.json")):
        folder = meta_file.parent
        if (folder / ".imported").exists():
            skipped += 1
            continue
        meta = json.loads(meta_file.read_text(encoding="utf-8"))
        try:
            ok = import_model(meta, folder) if is_model(meta) else import_texture_set(meta, folder)
        except Exception as error:
            ok = False
            unreal.log_error(f"[Ricovero] {meta.get('name')}: {error}")
        if ok:
            (folder / ".imported").touch()
            imported += 1
        else:
            failed += 1

    unreal.log(f"[Ricovero] Import completato: {imported} importati, {skipped} già presenti, {failed} falliti")


main()
