"""
Crea la mappa di prova /Game/Ricovero/Maps/Test/L_Test_Systems per i Passi 2 e 3:
pavimento, muri, buio con poca luce, torcia, pile, un documento, una chiave e due porte
(una chiusa a chiave). Sovrascrive la mappa se esiste già.

Da eseguire nell'editor (File -> Execute Python Script) dopo tools/setup_player.py.
"""

import unreal

MAP_PATH = "/Game/Ricovero/Maps/Test/L_Test_Systems"
CUBE = "/Engine/BasicShapes/Cube.Cube"
PLANE = "/Engine/BasicShapes/Plane.Plane"

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Se la mappa è aperta non si può cancellare: prima si passa a una mappa vuota senza nome.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
    if not unreal.EditorAssetLibrary.delete_asset(MAP_PATH):
        raise RuntimeError(f"Impossibile cancellare {MAP_PATH}: chiudila o liberala e riprova")
if not level_editor.new_level(MAP_PATH):
    raise RuntimeError(f"Impossibile creare {MAP_PATH}")


def spawn(cls, location, rotation=(0, 0, 0), label=None):
    actor = actors.spawn_actor_from_class(cls, unreal.Vector(*location), unreal.Rotator(*rotation))
    if label:
        actor.set_actor_label(label)
    return actor


def block(label, location, scale):
    """Cubo da blockout: la mesh base è 100 cm, quindi scale = dimensioni in metri."""
    actor = spawn(unreal.StaticMeshActor, location, label=label)
    comp = actor.static_mesh_component
    comp.set_static_mesh(unreal.EditorAssetLibrary.load_asset(CUBE))
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


# --- Geometria: una stanza 6x6 m e un corridoio 3x16 m, muri alti 3 m -------

floor = spawn(unreal.StaticMeshActor, (500, 0, 0), label="Floor")
floor.static_mesh_component.set_static_mesh(unreal.EditorAssetLibrary.load_asset(PLANE))
floor.set_actor_scale3d(unreal.Vector(30, 12, 1))

block("Ceiling", (500, 0, 305), (30, 12, 0.1))
block("Wall_Back", (-300, 0, 150), (0.2, 12, 3))
block("Wall_Room_N", (0, 300, 150), (6, 0.2, 3))
block("Wall_Room_S", (0, -300, 150), (6, 0.2, 3))
# Parete tra stanza e corridoio, con un vano porta di 1 m al centro.
block("Wall_Door_N", (300, 175, 150), (0.2, 2.5, 3))
block("Wall_Door_S", (300, -175, 150), (0.2, 2.5, 3))
block("Wall_Door_Top", (300, 0, 260), (0.2, 1, 0.8))
block("Wall_Corr_N", (1100, 150, 150), (16, 0.2, 3))
block("Wall_Corr_S", (1100, -150, 150), (16, 0.2, 3))
# Fondo del corridoio con la porta chiusa a chiave.
block("Wall_End_N", (1900, 100, 150), (0.2, 1, 3))
block("Wall_End_S", (1900, -100, 150), (0.2, 1, 3))
block("Wall_End_Top", (1900, 0, 260), (0.2, 1, 0.8))

# --- Luce: quasi buio, per provare la torcia -------------------------------

sky = spawn(unreal.SkyLight, (0, 0, 400), label="SkyLight_Dim")
sky.light_component.set_intensity(0.05)

moon = spawn(unreal.RectLight, (-250, 0, 250), (0, -30, 0), label="Light_Window")
moon.light_component.set_intensity(2.0)
moon.light_component.set_light_color(unreal.LinearColor(0.55, 0.65, 1.0, 1.0))

neon = spawn(unreal.PointLight, (1100, 0, 280), label="Light_Corridor_Neon")
neon.light_component.set_intensity(40.0)
neon.light_component.set_attenuation_radius(600)

ppv = spawn(unreal.PostProcessVolume, (500, 0, 150), label="PostProcess")
ppv.set_editor_property("unbound", True)

spawn(unreal.PlayerStart, (-150, 0, 100), label="PlayerStart")

# --- Oggetti ---------------------------------------------------------------

def pickup(label, location, kind, **props):
    actor = spawn(unreal.RcvPickup, location, label=label)
    actor.set_editor_property("type", kind)
    for key, value in props.items():
        actor.set_editor_property(key, value)
    return actor


block("Cart", (-50, 200, 40), (0.9, 0.5, 0.8))
pickup("Pickup_Flashlight", (-50, 200, 86), unreal.RcvPickupType.FLASHLIGHT)
pickup("Pickup_Battery_1", (150, -200, 10), unreal.RcvPickupType.BATTERY)
pickup("Pickup_Battery_2", (900, 100, 10), unreal.RcvPickupType.BATTERY)
pickup("Pickup_Key_Ward", (1500, -100, 10), unreal.RcvPickupType.KEY, key_id="Key_Ward")

document = unreal.RcvDocument()
document.set_editor_property("id", "Doc_Test")
document.set_editor_property("title", "Cartella clinica — Paziente 67")
document.set_editor_property(
    "body",
    "Paziente sedato alle 23:40. Nessuna reazione agli stimoli.\n"
    "Il dottor R. insiste per proseguire con il protocollo nonostante il parere contrario del personale.\n"
    "Nota a margine, a matita: NON LASCIATELO SOLO DI NOTTE.",
)
pickup("Pickup_Document", (600, 100, 6), unreal.RcvPickupType.DOCUMENT, document=document)

door_room = spawn(unreal.RcvDoor, (300, -50, 0), (0, 90, 0), label="Door_Room")
door_end = spawn(unreal.RcvDoor, (1900, -50, 0), (0, 90, 0), label="Door_Locked")
door_end.set_editor_property("locked", True)
door_end.set_editor_property("required_key", "Key_Ward")

if not level_editor.save_current_level():
    raise RuntimeError(f"Salvataggio di {MAP_PATH} fallito")
unreal.log(f"[Ricovero] Mappa di prova salvata: {MAP_PATH}")
