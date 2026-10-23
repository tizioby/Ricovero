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

if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    unreal.EditorAssetLibrary.delete_asset(MAP_PATH)
level_editor.new_level(MAP_PATH)


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


def neon(label, x, state):
    actor = spawn(unreal.RcvNeonLight, (x, 0, WALL_H - 8), label=label)
    actor.set_editor_property("initial_state", state)
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


# --- Pavimento e soffitto ----------------------------------------------------

box("Floor", -520, 3420, -220, 220, -10, 0)
box("Ceiling", -520, 3420, -220, 220, WALL_H, WALL_H + 10)

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

box("Gurney", -400, -200, -40, 40, 0, 90)
box("Cart", -190, -110, 120, 170, 0, 80)
box("Monitor_Stand", -420, -380, -190, -150, 0, 110)
box("Monitor", -425, -375, -190, -160, 110, 150)

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
wheelchair = box("Wheelchair", -100, -40, -30, 30, 0, 90)
wheelchair_mesh = wheelchair.static_mesh_component
wheelchair_mesh.set_mobility(unreal.ComponentMobility.MOVABLE)
wheelchair_mesh.set_simulate_physics(True)
wheelchair_mesh.set_mass_override_in_kg("", 20.0, True)

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

box("Bench", 1150, 1300, 70, 115, 0, 45)
box("Gurney_Corridor", 1850, 2040, -115, -45, 0, 80)

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
n4 = neon("Neon_04", 2200, unreal.RcvNeonState.ON)
n5 = neon("Neon_05", 2550, unreal.RcvNeonState.ON)
n6 = neon("Neon_06", 2900, unreal.RcvNeonState.ON)

# Sagoma del mostro: compare in fondo al corridoio e sparisce quando la luce si spegne.
silhouette = box("Monster_Silhouette", 2770, 2830, -30, 30, 0, 215, mesh=CYLINDER)
silhouette.set_folder_path("Gameplay")

# Fondo del corridoio con la porta del reparto.
wall_x("Corridor_End", 3000, -125, 125, gaps=[(-50, 50, 210)])
door("Door_Ward", 3000)

# --- Evento di fine Atto 1 ----------------------------------------------------

event = spawn(unreal.RcvScriptedTrigger, (2050, 0, 150), label="Event_Act1_LightsOut")
event.set_editor_property("steps", [
    step(0.0, unreal.RcvStepType.SHOW_ACTORS, [silhouette]),
    step(1.2, unreal.RcvStepType.LIGHTS_OFF, [n6, n5, n4], stagger=0.55),
    step(0.15, unreal.RcvStepType.HIDE_ACTORS, [silhouette]),
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

level_editor.save_current_level()
unreal.log(f"[Ricovero] Blockout Atto 1 salvato: {MAP_PATH}")
