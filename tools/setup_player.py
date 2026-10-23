"""
Crea gli asset di input e il Blueprint del giocatore (Passo 2).

Da eseguire nell'editor di Unreal: Output Log -> menu a tendina "Cmd" -> "Python", poi
    py "<percorso del progetto>/tools/setup_player.py"
oppure File -> Execute Python Script.

Crea (o aggiorna, se esistono già):
    /Game/Ricovero/Input/IA_Move, IA_Look, IA_Sprint, IA_Crouch
    /Game/Ricovero/Blueprints/Player/BP_RcvCharacter (figlio di ARcvCharacter)
"""

import unreal

INPUT_DIR = "/Game/Ricovero/Input"
PLAYER_DIR = "/Game/Ricovero/Blueprints/Player"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def load_or_create(name, folder, asset_class, factory):
    path = f"{folder}/{name}"
    if eal.does_asset_exist(path):
        return eal.load_asset(path)
    asset = asset_tools.create_asset(name, folder, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"Impossibile creare {path}")
    unreal.log(f"[Ricovero] Creato {path}")
    return asset


def make_key(name):
    key = unreal.Key()
    key.set_editor_property("key_name", name)
    return key


def input_action(name, value_type):
    action = load_or_create(name, INPUT_DIR, unreal.InputAction, unreal.InputAction_Factory())
    action.set_editor_property("value_type", value_type)
    eal.save_loaded_asset(action)
    return action


# --- Input Actions ---------------------------------------------------------

ia_move = input_action("IA_Move", unreal.InputActionValueType.AXIS2D)
ia_look = input_action("IA_Look", unreal.InputActionValueType.AXIS2D)
ia_sprint = input_action("IA_Sprint", unreal.InputActionValueType.BOOLEAN)
ia_crouch = input_action("IA_Crouch", unreal.InputActionValueType.BOOLEAN)

# I tasti (WASD, mouse, gamepad) si costruiscono in C++ a runtime:
# vedi ARcvCharacter::BuildMappingContext. Qui servono solo le Input Action.

# --- Blueprint del giocatore -----------------------------------------------

bp_path = f"{PLAYER_DIR}/BP_RcvCharacter"
if eal.does_asset_exist(bp_path):
    bp = eal.load_asset(bp_path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.RcvCharacter)
    bp = load_or_create("BP_RcvCharacter", PLAYER_DIR, unreal.Blueprint, factory)

cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(bp_path))
cdo.set_editor_property("move_action", ia_move)
cdo.set_editor_property("look_action", ia_look)
cdo.set_editor_property("sprint_action", ia_sprint)
cdo.set_editor_property("crouch_action", ia_crouch)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)

unreal.log("[Ricovero] Setup giocatore completato: premi Play in una mappa qualsiasi.")
