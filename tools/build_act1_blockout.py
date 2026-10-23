"""
Blockout dell'Atto 1 in /Game/Ricovero/Maps/L_Hospital (sovrascrive la mappa se esiste).

Percorso: Stanza 67 (risveglio) -> corridoio principale (30 m) -> porta del reparto.
Solo cubi grigi, luci e logica: serve a provare ritmo e durata prima degli asset veri.
Misure in cm. X = avanti lungo il percorso, Y = sinistra/destra, Z = alto.

Da eseguire nell'editor (File -> Esegui script Python) dopo tools/setup_player.py.
"""

import unreal

MAP_PATH = "/Game/Ricovero/Maps/L_Hospital"
CUBE = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube.Cube")
CYLINDER = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cylinder.Cylinder")

WALL_H = 300.0
WALL_T = 20.0

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Se la mappa è aperta non si può cancellare: prima si passa a una mappa vuota senza nome.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    if not unreal.EditorAssetLibrary.delete_asset(MAP_PATH):
        raise RuntimeError(f"Impossibile cancellare {MAP_PATH}: chiudila o liberala e riprova")
if not level_editor.new_level(MAP_PATH):
    raise RuntimeError(f"Impossibile creare {MAP_PATH}")


def spawn(cls, location, yaw=0.0, label=None, pitch=0.0):
    actor = actors.spawn_actor_from_class(cls, unreal.Vector(*location), unreal.Rotator(0.0, pitch, yaw))
    if label:
        actor.set_actor_label(label)
    return actor


def box(label, x0, x1, y0, y1, z0, z1, mesh=CUBE):
    """Cubo da blockout definito dai suoi limiti (cm)."""
    center = ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2)
    actor = spawn(unreal.StaticMeshActor, center, label=label)
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.set_actor_scale3d(unreal.Vector((x1 - x0) / 100, (y1 - y0) / 100, (z1 - z0) / 100))
    actor.set_folder_path("Blockout")
    return actor


def wall_x(label, x, y0, y1, gaps=()):
    """Muro perpendicolare a X, da y0 a y1, con eventuali vani (y_min, y_max, z_top)."""
    cursor = y0
    for i, (g0, g1, top) in enumerate(sorted(gaps)):
        if g0 > cursor:
            box(f"{label}_{i}", x - WALL_T / 2, x + WALL_T / 2, cursor, g0, 0, WALL_H)
        box(f"{label}_{i}_Top", x - WALL_T / 2, x + WALL_T / 2, g0, g1, top, WALL_H)
        cursor = g1
    if cursor < y1:
        box(f"{label}_End", x - WALL_T / 2, x + WALL_T / 2, cursor, y1, 0, WALL_H)


def wall_y(label, y, x0, x1):
    box(label, x0, x1, y - WALL_T / 2, y + WALL_T / 2, 0, WALL_H)


def pickup(label, location, kind, **props):
    actor = spawn(unreal.RcvPickup, location, label=label)
    actor.set_editor_property("type", kind)
    for key, value in props.items():
        actor.set_editor_property(key, value)
    actor.set_folder_path("Gameplay")
    return actor


def door(label, x, yaw=90.0, y=-50.0, **props):
    actor = spawn(unreal.RcvDoor, (x, y, 0), yaw, label=label)
    for key, value in props.items():
        actor.set_editor_property(key, value)
    actor.set_folder_path("Gameplay")
    return actor


def neon(label, x, state, lumens=None):
    actor = spawn(unreal.RcvNeonLight, (x, 0, WALL_H - 8), label=label)
    actor.set_editor_property("initial_state", state)
    if lumens is not None:
        actor.set_editor_property("on_intensity_lumens", lumens)
    actor.set_folder_path("Lights")
    return actor


def step(delay, kind, targets=(), stagger=0.0, lock=False):
    s = unreal.RcvScriptStep()
    s.set_editor_property("delay", delay)
    s.set_editor_property("type", kind)
    s.set_editor_property("targets", list(targets))
    s.set_editor_property("stagger", stagger)
    s.set_editor_property("lock", lock)
    return s


# --- Asset veri (se importati con tools/import_assets.py) ---------------------
# Ogni cubo del blockout resta come collisione invisibile e il modello vero gli viene
# agganciato sopra: la fisica e il gameplay non cambiano, cambia solo l'aspetto.

MODELS = "/Game/Ricovero/Models"
TEXTURES = "/Game/Ricovero/Textures"


def model_meshes(category, name, kind=unreal.StaticMesh):
    """Tutte le mesh importate di un modello (i glTF di Poly Haven possono averne più di una)."""
    folder = f"{MODELS}/{category}/{name}"
    if not unreal.EditorAssetLibrary.does_directory_exist(folder):
        return []
    assets = [unreal.EditorAssetLibrary.load_asset(p) for p in unreal.EditorAssetLibrary.list_assets(folder, recursive=True)]
    return [a for a in assets if isinstance(a, kind)]


def meshes_size(meshes):
    lo = [1e9, 1e9, 1e9]
    hi = [-1e9, -1e9, -1e9]
    for mesh in meshes:
        box = mesh.get_bounding_box()
        for i, axis in enumerate("xyz"):
            lo[i] = min(lo[i], getattr(box.min, axis))
            hi[i] = max(hi[i], getattr(box.max, axis))
    return [hi[i] - lo[i] for i in range(3)]


def place_model(label, category, name, location, yaw=0.0, collision=True, folder="Props"):
    """Mette un modello vero nella scena. Restituisce gli attori creati (uno per mesh)."""
    created = []
    for i, mesh in enumerate(model_meshes(category, name)):
        actor = spawn(unreal.StaticMeshActor, location, yaw=yaw, label=f"{label}_{i + 1:02d}")
        comp = actor.static_mesh_component
        comp.set_static_mesh(mesh)
        if not collision:
            comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        actor.set_folder_path(folder)
        created.append(actor)
    return created


def dress(cube, category, name, z=None):
    """Sostituisce l'aspetto di un cubo del blockout con un modello vero, allineato al lato lungo."""
    meshes = model_meshes(category, name)
    if not meshes:
        return []
    origin, extent = cube.get_actor_bounds(False)
    size = meshes_size(meshes)
    cube_long_x = extent.x >= extent.y
    model_long_x = size[0] >= size[1]
    yaw = 0.0 if cube_long_x == model_long_x else 90.0
    base = (origin.x, origin.y, origin.z - extent.z if z is None else z)
    created = place_model(cube.get_actor_label() + "_Model", category, name, base, yaw=yaw, collision=False)
    comp = cube.static_mesh_component
    comp.set_visibility(False, False)
    comp.set_cast_shadow(False)
    movable = comp.get_editor_property("mobility") == unreal.ComponentMobility.MOVABLE
    for actor in created:
        if movable:
            actor.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
        actor.attach_to_actor(cube, "", unreal.AttachmentRule.KEEP_WORLD, unreal.AttachmentRule.KEEP_WORLD,
                              unreal.AttachmentRule.KEEP_WORLD, False)
    return created


# Materiali delle superfici: proiezione in coordinate mondo, così le texture non si
# stirano sui cubi scalati del blockout. Un master con ORM e uno con Roughness separata.

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary


def ensure_world_master(name, split, textures):
    path = f"/Game/Ricovero/Materials/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)
    material = asset_tools.create_asset(name, "/Game/Ricovero/Materials", unreal.Material, unreal.MaterialFactoryNew())

    def node(cls, x, y):
        return mel.create_material_expression(material, cls, x, y)

    def mask(source, x, y, channels):
        m = node(unreal.MaterialExpressionComponentMask, x, y)
        for ch in "rgba":
            m.set_editor_property(ch, ch in channels)
        mel.connect_material_expressions(source, "", m, "")
        return m

    def mul(a, b, x, y):
        m = node(unreal.MaterialExpressionMultiply, x, y)
        mel.connect_material_expressions(a, "", m, "A")
        mel.connect_material_expressions(b, "", m, "B")
        return m

    def add(a, b, x, y):
        m = node(unreal.MaterialExpressionAdd, x, y)
        mel.connect_material_expressions(a, "", m, "A")
        mel.connect_material_expressions(b, "", m, "B")
        return m

    # Posizione in mondo / dimensione della piastrella (cm).
    world = node(unreal.MaterialExpressionWorldPosition, -2000, 0)
    tile = node(unreal.MaterialExpressionScalarParameter, -2000, 150)
    tile.set_editor_property("parameter_name", "TileSizeCm")
    tile.set_editor_property("default_value", 200.0)
    pos = node(unreal.MaterialExpressionDivide, -1800, 50)
    mel.connect_material_expressions(world, "", pos, "A")
    mel.connect_material_expressions(tile, "", pos, "B")

    # Normale in mondo: sulle facce dei cubi vale esattamente 0 o 1 per asse.
    normal_ws = node(unreal.MaterialExpressionVertexNormalWS, -2000, 400)
    absn = node(unreal.MaterialExpressionAbs, -1800, 400)
    mel.connect_material_expressions(normal_ws, "", absn, "")

    uv_yz = mul(mask(pos, -1600, -100, "gb"), mask(absn, -1600, 300, "r"), -1400, -100)
    uv_xz = mul(mask(pos, -1600, 50, "rb"), mask(absn, -1600, 450, "g"), -1400, 50)
    uv_xy = mul(mask(pos, -1600, 200, "rg"), mask(absn, -1600, 600, "b"), -1400, 200)
    uv = add(add(uv_yz, uv_xz, -1200, 0), uv_xy, -1000, 50)

    def tex(param, sampler, y, default):
        # Default dello stesso tipo del sampler, altrimenti il materiale non compila.
        t = node(unreal.MaterialExpressionTextureSampleParameter2D, -700, y)
        t.set_editor_property("parameter_name", param)
        t.set_editor_property("texture", default)
        t.set_editor_property("sampler_type", sampler)
        mel.connect_material_expressions(uv, "", t, "UVs")
        return t

    base = tex("BaseColor", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, -400, textures["D"])
    mel.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    normal = tex("Normal", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, -100, textures["N"])
    mel.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    if split:
        rough = tex("Roughness", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE, 200, textures["R"])
        mel.connect_material_property(rough, "R", unreal.MaterialProperty.MP_ROUGHNESS)
    else:
        orm = tex("ORM", unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, 200, textures["ORM"])
        mel.connect_material_property(orm, "R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
        mel.connect_material_property(orm, "G", unreal.MaterialProperty.MP_ROUGHNESS)
        mel.connect_material_property(orm, "B", unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def surface_material(name, tile_cm=200.0):
    """Material instance in coordinate mondo per una superficie importata, o None se manca."""
    folder = f"{TEXTURES}/Surfaces/{name}"
    textures = {}
    for suffix in ("D", "N", "ORM", "R"):
        path = f"{folder}/T_{name}_{suffix}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            textures[suffix] = unreal.EditorAssetLibrary.load_asset(path)
    if "D" not in textures or "N" not in textures or ("ORM" not in textures and "R" not in textures):
        return None
    split = "ORM" not in textures
    master = ensure_world_master("M_Surface_World_Split" if split else "M_Surface_World", split, textures)
    mi_path = f"{folder}/MI_{name}_World"
    if unreal.EditorAssetLibrary.does_asset_exist(mi_path):
        mi = unreal.EditorAssetLibrary.load_asset(mi_path)
    else:
        mi = asset_tools.create_asset(f"MI_{name}_World", folder, unreal.MaterialInstanceConstant,
                                      unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, master)
    for suffix, param in {"D": "BaseColor", "N": "Normal", "ORM": "ORM", "R": "Roughness"}.items():
        if suffix in textures:
            mel.set_material_instance_texture_parameter_value(mi, param, textures[suffix])
    mel.set_material_instance_scalar_parameter_value(mi, "TileSizeCm", tile_cm)
    unreal.EditorAssetLibrary.save_loaded_asset(mi)
    return mi


# --- Pavimento e soffitto ----------------------------------------------------

floor = box("Floor", -520, 3420, -220, 220, -10, 0)
ceiling = box("Ceiling", -520, 3420, -220, 220, WALL_H, WALL_H + 10)

# --- Stanza 67 (x -500..0, y -200..200) --------------------------------------

# Parete di fondo con la finestra (120 x 150 cm) da cui entra la luce della luna.
box("Room_Back_L", -510, -490, -200, -60, 0, WALL_H)
box("Room_Back_R", -510, -490, 60, 200, 0, WALL_H)
box("Room_Back_Sill", -510, -490, -60, 60, 0, 100)
box("Room_Back_Lintel", -510, -490, -60, 60, 250, WALL_H)
wall_y("Room_N", 200, -500, 0)
wall_y("Room_S", -200, -500, 0)
# Parete verso il corridoio con la porta (vano 100 x 210 cm).
wall_x("Room_Front", 0, -200, 200, gaps=[(-50, 50, 210)])

gurney = box("Gurney", -400, -200, -40, 40, 0, 90)
cart = box("Cart", -190, -110, 120, 170, 0, 80)
box("Monitor_Stand", -420, -380, -190, -150, 0, 110)
monitor = box("Monitor", -425, -375, -190, -160, 110, 150)

monitor_light = spawn(unreal.PointLight, (-400, -150, 135), label="Light_Monitor")
monitor_light.light_component.set_intensity(80.0)
monitor_light.light_component.set_light_color(unreal.LinearColor(0.4, 1.0, 0.5, 1.0))
monitor_light.light_component.set_attenuation_radius(500)
monitor_light.set_folder_path("Lights")

moon = spawn(unreal.RectLight, (-600, 0, 230), yaw=0.0, pitch=-25.0, label="Light_Moon")
moon.light_component.set_intensity(800.0)
moon.light_component.set_light_color(unreal.LinearColor(0.55, 0.68, 1.0, 1.0))
moon.light_component.set_attenuation_radius(1500)
moon.set_folder_path("Lights")

# Sedia a rotelle (fisica) davanti alla porta: va spinta via.
# Pesante e frenata, non si ribalta e non ci si sale sopra: scivola come su ruote.
wheelchair = box("Wheelchair", -100, -40, -30, 30, 0, 90)
wheelchair_mesh = wheelchair.static_mesh_component
wheelchair_mesh.set_mobility(unreal.ComponentMobility.MOVABLE)
wheelchair_mesh.set_simulate_physics(True)
wheelchair_mesh.set_mass_override_in_kg("", 45.0, True)
wheelchair_mesh.set_linear_damping(2.5)
wheelchair_mesh.set_angular_damping(6.0)
wheelchair_mesh.set_editor_property("can_character_step_up_on", unreal.CanBeCharacterBase.ECB_NO)
body = wheelchair_mesh.get_editor_property("body_instance")
body.set_editor_property("lock_x_rotation", True)
body.set_editor_property("lock_y_rotation", True)
wheelchair_mesh.set_editor_property("body_instance", body)

spawn(unreal.PlayerStart, (-300, 110, 100), yaw=0.0, label="PlayerStart")
start1 = spawn(unreal.RcvActStart, (-300, 110, 100), yaw=0.0, label="ActStart_1")
start1.set_editor_property("act", 1)

pickup("Pickup_Flashlight", (-150, 145, 86), unreal.RcvPickupType.FLASHLIGHT)

door_67 = door("Door_67", 0)

# --- Corridoio principale (x 0..3000, y -125..125) -----------------------------

wall_y("Corridor_N", 125, 0, 3000)
wall_y("Corridor_S", -125, 0, 3000)
# Riempie lo spazio tra stanza e corridoio sopra e sotto.
box("Corridor_Fill_N", 0, 3000, 125, 220, 0, WALL_H)
box("Corridor_Fill_S", 0, 3000, -220, -125, 0, WALL_H)

# Porte laterali chiuse: solo scenografia per ora.
door("Door_Side_112", 800, yaw=0.0, y=112, locked=True)
door("Door_Side_118", 1600, yaw=0.0, y=-112, locked=True)

bench = box("Bench", 1150, 1300, 70, 115, 0, 45)
gurney_corridor = box("Gurney_Corridor", 1850, 2040, -115, -45, 0, 80)

document = unreal.RcvDocument()
document.set_editor_property("id", "Doc_Act1_Chart67")
document.set_editor_property("title", "Cartella clinica - Stanza 67")
document.set_editor_property(
    "body",
    "Paziente: [illeggibile]. Ricoverato il 14/03. Nessun documento d'identita'.\n"
    "Terapia sperimentale autorizzata dal Direttore. Dosaggio aumentato su sua richiesta.\n"
    "23:40 - Paziente sedato. Nessuna reazione agli stimoli.\n"
    "Nota a matita, in fondo alla pagina: NON LASCIATELO SOLO DI NOTTE. LUI TORNA.",
)
pickup("Pickup_Chart67", (1225, 92, 50), unreal.RcvPickupType.DOCUMENT, document=document)
pickup("Pickup_Battery_Act1", (1945, -80, 86), unreal.RcvPickupType.BATTERY)

n1 = neon("Neon_01", 400, unreal.RcvNeonState.OFF)
n2 = neon("Neon_02", 1000, unreal.RcvNeonState.FLICKER)
n3 = neon("Neon_03", 1600, unreal.RcvNeonState.OFF)
# Fondo del corridoio in penombra: abbastanza luce per vedere la sagoma, non di più.
n4 = neon("Neon_04", 2200, unreal.RcvNeonState.ON, lumens=600)
n5 = neon("Neon_05", 2550, unreal.RcvNeonState.FLICKER, lumens=450)
n6 = neon("Neon_06", 2900, unreal.RcvNeonState.ON, lumens=300)

# Sagoma del mostro: compare in fondo al corridoio e sparisce quando la luce si spegne.
monster_meshes = model_meshes("Characters", "BoiledOne", unreal.SkeletalMesh)
if monster_meshes:
    # The Boiled One (landgr4, CC-BY): tutte le parti nello stesso punto, scalate a 2,6 m.
    silhouette = []
    for i, mesh in enumerate(monster_meshes):
        part = spawn(unreal.SkeletalMeshActor, (2800, 0, 0), yaw=180.0, label=f"Monster_BoiledOne_{i + 1:02d}")
        part.skeletal_mesh_component.set_skinned_asset_and_update(mesh)
        part.set_folder_path("Gameplay")
        silhouette.append(part)
    lo, hi = 1e9, -1e9
    for part in silhouette:
        origin, extent = part.get_actor_bounds(False)
        lo, hi = min(lo, origin.z - extent.z), max(hi, origin.z + extent.z)
    if hi > lo:
        factor = 260.0 / (hi - lo)
        for part in silhouette:
            part.set_actor_scale3d(unreal.Vector(factor, factor, factor))
            loc = part.get_actor_location()
            part.set_actor_location(unreal.Vector(loc.x, loc.y, -lo * factor), False, False)
else:
    silhouette = [box("Monster_Silhouette", 2770, 2830, -30, 30, 0, 215, mesh=CYLINDER)]
    silhouette[0].set_folder_path("Gameplay")

# Fondo del corridoio con la porta del reparto.
wall_x("Corridor_End", 3000, -125, 125, gaps=[(-50, 50, 210)])
door("Door_Ward", 3000)

# --- Evento di fine Atto 1 ----------------------------------------------------

event = spawn(unreal.RcvScriptedTrigger, (2050, 0, 150), label="Event_Act1_LightsOut")
event.set_editor_property("steps", [
    # La sagoma resta visibile poco più di mezzo secondo: si spegne la luce alle sue spalle e sparisce.
    step(0.0, unreal.RcvStepType.SHOW_ACTORS, silhouette),
    step(0.5, unreal.RcvStepType.LIGHTS_OFF, [n6]),
    step(0.1, unreal.RcvStepType.HIDE_ACTORS, silhouette),
    step(0.3, unreal.RcvStepType.LIGHTS_OFF, [n5, n4], stagger=0.5),
    step(0.8, unreal.RcvStepType.LIGHTS_OFF, [n2]),
    step(1.5, unreal.RcvStepType.DOOR_SLAM, [door_67], lock=True),
])
event.set_folder_path("Gameplay")

# --- Vestibolo del reparto e fine demo ------------------------------------------

wall_y("Ward_N", 125, 3000, 3400)
wall_y("Ward_S", -125, 3000, 3400)
box("Ward_Back", 3400, 3420, -220, 220, 0, WALL_H)

end_trigger = spawn(unreal.RcvActTrigger, (3150, 0, 150), label="ActTrigger_EndOfDemo")
end_trigger.set_editor_property("act", 2)
end_trigger.set_editor_property("ends_game", True)
end_trigger.set_folder_path("Gameplay")

start2 = spawn(unreal.RcvActStart, (3300, 0, 100), yaw=0.0, label="ActStart_2")
start2.set_editor_property("act", 2)

# --- Atmosfera -----------------------------------------------------------------

sky = spawn(unreal.SkyLight, (0, 0, 600), label="SkyLight_Dim")
sky.light_component.set_intensity(0.15)
sky.set_folder_path("Lights")

fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), label="Fog")
fog_comp = fog.component
fog_comp.set_editor_property("fog_density", 0.02)
fog_comp.set_editor_property("enable_volumetric_fog", True)
fog.set_folder_path("Lights")

ppv = spawn(unreal.PostProcessVolume, (1500, 0, 150), label="PostProcess")
ppv.set_editor_property("unbound", True)
settings = ppv.get_editor_property("settings")
# Esposizione lenta: l'occhio si adatta al buio, ma senza rendere tutto chiaro.
for key, value in {
    "auto_exposure_min_brightness": -4.0,
    "auto_exposure_max_brightness": 4.0,
    "auto_exposure_speed_up": 0.6,
    "auto_exposure_speed_down": 0.4,
}.items():
    settings.set_editor_property("override_" + key, True)
    settings.set_editor_property(key, value)
ppv.set_editor_property("settings", settings)
ppv.set_folder_path("Lights")

# --- Arredo con gli asset veri --------------------------------------------------
# Se un asset non è stato importato, resta il cubo grigio del blockout.

dress(gurney, "Furniture", "HospitalBedFrame")
dress(gurney_corridor, "Furniture", "HospitalBedFrame")
dress(cart, "Medical", "RustyTrolley")
dress(wheelchair, "Medical", "Wheelchair")
dress(monitor, "Props", "CrtTv")
if model_meshes("Furniture", "MetalChair"):
    bench.static_mesh_component.set_visibility(False, False)
    place_model("Chair_A", "Furniture", "MetalChair", (1190, 95, 0), yaw=-90.0)
    place_model("Chair_B", "Furniture", "MetalChair", (1265, 95, 0), yaw=-90.0)

# Oggetti di scena: ognuno racconta qualcosa (abbandono, fuga, incuria).
place_model("Rack_Room", "Furniture", "MetalRack", (-300, 168, 0), yaw=0.0)
place_model("MedicalBox_Cart", "Medical", "MedicalBox", (-175, 132, 80), yaw=15.0)
place_model("Crutches_Room", "Medical", "Crutches", (-470, -170, 0), yaw=30.0)
place_model("WetFloor_Corridor", "Props", "WetFloorSign", (650, -70, 0), yaw=40.0)
place_model("TrashCan_Corridor", "Props", "MetalTrashCan", (1420, 100, 0))
place_model("FireAlarm_Corridor", "Props", "FireAlarm", (1000, 113, 150), yaw=-90.0, collision=False)
place_model("Box_Corridor_A", "Props", "CardboardBox", (2380, 95, 0), yaw=10.0)
place_model("Box_Corridor_B", "Props", "CardboardBox", (2440, 90, 0), yaw=-25.0)
place_model("Clipboard_Gurney", "Props", "Clipboard", (1900, -80, 81), yaw=70.0, collision=False)

# Superfici.
surfaces = {
    "Floor": surface_material("FloorLinoleum", 150.0),
    "Ceiling": surface_material("CeilingTiles", 120.0),
    "Room_": surface_material("WallPeelingPaint", 250.0),
    "Corridor_": surface_material("WallCrackedPlaster", 250.0),
    "Ward_": surface_material("WallTilesBroken", 120.0),
}
# Porte: anta in metallo verniciato invece della griglia di prova.
door_material = surface_material("MetalPaintedGreen", 100.0) or surface_material("MetalRustyPainted", 100.0)
if door_material:
    for actor in actors.get_all_level_actors():
        if isinstance(actor, unreal.RcvDoor):
            actor.get_editor_property("leaf").set_material(0, door_material)

for actor in actors.get_all_level_actors():
    if not isinstance(actor, unreal.StaticMeshActor) or str(actor.get_folder_path()) != "Blockout":
        continue
    label = actor.get_actor_label()
    for prefix, material in surfaces.items():
        if material and label.startswith(prefix):
            actor.static_mesh_component.set_material(0, material)
            break

if not level_editor.save_current_level():
    raise RuntimeError(f"Salvataggio di {MAP_PATH} fallito")
unreal.log(f"[Ricovero] Blockout Atto 1 salvato: {MAP_PATH}")
