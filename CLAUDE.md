# Ricovero — regole del progetto

Horror psicologico in prima persona, Unreal Engine 5, circa 20 minuti di gioco.
Piano approvato: https://claude.ai/artifact/CB2CXwv7h1fwX6xMoyrU4B

## Versione e strumenti

- **Unreal Engine 5.8** dal Epic Games Launcher, ultima hotfix 5.8.x. Si aggiorna solo tra una milestone e l'altra.
- Visual Studio 2022 con "Game development with C++" (o Rider).
- Git + Git LFS. Gli asset binari in `Content/` vanno in LFS (vedi `.gitattributes`).
- Claude lavora senza editor né GPU: compilazione, piazzamento in scena e test si fanno sul PC di Nick.

## Regole di codice

- Logica in C++ (modulo `Ricovero`). Blueprint solo come figli configurabili di classi C++, per eventi scriptati e UI.
- Prefisso classi C++ `Rcv` (`ARcvCharacter`, `URcvFlashlightComponent`, `IRcvInteractable`).
- Valori di bilanciamento (velocità, stamina, batteria) in DataAsset o proprietà `EditDefaultsOnly`, mai cablati.
- Log con la categoria `LogRcv`; il timer di sessione usa `LogRcvTimer`.
- Comandi di debug con prefisso `Rcv.` (`Rcv.GoToAct`, `Rcv.Battery`, `Rcv.God`), solo nelle build non Shipping.
- Nessun segreto nel codice: il token Sketchfab si legge dalla variabile d'ambiente `SKETCHFAB_TOKEN`.

## Struttura delle cartelle

```
Config/                 ini di progetto (rendering, input, scalabilità)
Source/Ricovero/
  Player/ Flashlight/ Interaction/ Inventory/ AI/ Narrative/ Game/ Debug/ UI/
Content/Ricovero/
  Maps/ Blueprints/ Input/ UI/ Audio/ Materials/ Textures/ Models/ FX/ Data/
Assets_Raw/             download grezzi, ignorati da Git, rigenerabili con tools/
tools/                  download_assets.py, import_assets.py, optimize_assets.py
CREDITS.md              una riga per asset: nome, autore, fonte, link, licenza
assets_wanted.txt       lista degli asset da scaricare
```

Mappe: persistent `L_Hospital` + sublevel `L_Act1_Wake`, `L_Act2_Ward`, `L_Act3_Morgue`, `L_Act4_Escape`,
`L_Hospital_Lighting`, `L_Hospital_Audio`.

## Naming degli asset

| Tipo | Prefisso | Esempio |
| --- | --- | --- |
| Static Mesh | SM_ | SM_HospitalBed_01 |
| Skeletal Mesh | SK_ | SK_Enemy |
| Material / Instance | M_ / MI_ | MI_Tile_Dirty_02 |
| Texture | T_ + suffisso _D _N _ORM _H | T_Tile_Dirty_ORM |
| Blueprint | BP_ | BP_Door_Locked |
| Widget | WBP_ | WBP_PauseMenu |
| Decal | MI_Decal_ | MI_Decal_DriedBlood_01 |
| Sound Cue / MetaSound | SC_ / MS_ | MS_Footstep_Linoleum |
| Niagara | NS_ | NS_DustMotes |
| Mappa | L_ | L_Act1_Wake |

Texture PBR impacchettate in ORM (R = Occlusion, G = Roughness, B = Metallic).

## Linee guida grafiche

- Illuminazione completamente dinamica: Lumen (ray tracing hardware dove disponibile), Virtual Shadow Maps, MegaLights, Nanite.
- Materiali PBR completi 2K-4K, usura sempre presente: muffa, ruggine, intonaco scrostato, polvere, macchie.
- Decal per sporco, sangue secco e crepe; niente gore eccessivo.
- Volumetric fog, god ray dalle finestre, particelle di polvere.
- Post-process: esposizione automatica lenta, aberrazione cromatica leggera, film grain, vignettatura, DOF, motion blur moderato, grading freddo e desaturato.
- Asset solo realistici e sporchi, licenza CC0 o CC-BY, pochi e riutilizzati con varianti.

## Target di performance

- 60 fps a 1440p in Epic su RTX 4070 / 5060 Ti con TSR o DLSS Quality (16,6 ms).
- 30 fps a 1080p in Alto su RTX 2070 / RX 6600 XT.
- Misura con `stat unit` e Unreal Insights alla fine di ogni milestone.

## Budget di tempo per atto

| Atto | Contenuto | Giocatore normale | Percorso ottimale |
| --- | --- | --- | --- |
| 1 | Risveglio, Stanza 104, primo corridoio | ~3 min | ~2:30 |
| 2 | Reparto degenti, sala operatoria | ~6 min | ~4:30 |
| 3 | Obitorio, ufficio del direttore | ~7 min | ~5 min |
| 4 | Inseguimento, uscita di emergenza, finale | ~4 min | ~3 min |

Totale: ~15 min ottimale, 20-25 min normale. Checkpoint automatico all'inizio di ogni atto.
Ogni oggetto raccoglibile deve avere uno scopo narrativo o di gameplay.
