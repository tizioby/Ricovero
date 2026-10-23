"""
Crea gli asset di input e il Blueprint del giocatore (Passo 2).

Da eseguire nell'editor di Unreal: Output Log -> menu a tendina "Cmd" -> "Python", poi
    py "<percorso del progetto>/tools/setup_player.py"
oppure File -> Execute Python Script.

Crea (o aggiorna, se esistono già):
    /Game/Ricovero/Input/IA_Move, IA_Look, IA_Sprint, IA_Crouch, IMC_Default
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

# --- Mapping Context -------------------------------------------------------
# Ogni voce: (azione, tasto, modificatori). IA_Move usa X = destra, Y = avanti.

SWIZZLE = "swizzle"
NEGATE = "negate"

BINDINGS = [
    (ia_move, "W", [SWIZZLE]),
    (ia_move, "S", [SWIZZLE, NEGATE]),
    (ia_move, "D", []),
    (ia_move, "A", [NEGATE]),
    (ia_move, "Gamepad_Left2D", []),
    (ia_look, "Mouse2D", []),
    (ia_look, "Gamepad_Right2D", []),
    (ia_sprint, "LeftShift", []),
    (ia_sprint, "Gamepad_LeftThumbstick", []),
    (ia_crouch, "LeftControl", []),
    (ia_crouch, "C", []),
    (ia_crouch, "Gamepad_RightThumbstick", []),
]

imc = load_or_create("IMC_Default", INPUT_DIR, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
imc.unmap_all()
for action, key_name, _ in BINDINGS:
    imc.map_key(action, make_key(key_name))


def make_modifier(kind):
    if kind == SWIZZLE:
        return unreal.new_object(unreal.InputModifierSwizzleAxis, imc)  # ordine di default YXZ
    return unreal.new_object(unreal.InputModifierNegate, imc)


def apply_modifiers():
    """I modificatori vanno scritti sulla lista delle mappature del context.
    Il nome della proprietà è cambiato tra le versioni di UE, quindi si provano entrambe."""
    holder = None
    try:
        mappings = list(imc.get_editor_property("mappings"))
    except Exception:
        holder = imc.get_editor_property("default_key_mappings")
        mappings = list(holder.get_editor_property("mappings"))

    for mapping, (_, _, modifiers) in zip(mappings, BINDINGS):
        mapping.set_editor_property("modifiers", [make_modifier(m) for m in modifiers])

    if holder is None:
        imc.set_editor_property("mappings", mappings)
    else:
        holder.set_editor_property("mappings", mappings)
        imc.set_editor_property("default_key_mappings", holder)


try:
    apply_modifiers()
except Exception as error:
    unreal.log_warning(
        "[Ricovero] Modificatori non applicati automaticamente (%s). Apri IMC_Default e aggiungi a mano: "
        "W -> Swizzle Input Axis Values; S -> Swizzle + Negate; A -> Negate." % error
    )

eal.save_loaded_asset(imc)

# --- Blueprint del giocatore -----------------------------------------------

bp_path = f"{PLAYER_DIR}/BP_RcvCharacter"
if eal.does_asset_exist(bp_path):
    bp = eal.load_asset(bp_path)
else:
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", unreal.RcvCharacter)
    bp = load_or_create("BP_RcvCharacter", PLAYER_DIR, unreal.Blueprint, factory)

cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(bp_path))
cdo.set_editor_property("default_mapping_context", imc)
cdo.set_editor_property("move_action", ia_move)
cdo.set_editor_property("look_action", ia_look)
cdo.set_editor_property("sprint_action", ia_sprint)
cdo.set_editor_property("crouch_action", ia_crouch)

unreal.BlueprintEditorLibrary.compile_blueprint(bp)
eal.save_loaded_asset(bp)

unreal.log("[Ricovero] Setup giocatore completato: premi Play in una mappa qualsiasi.")
