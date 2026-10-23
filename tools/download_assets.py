"""
Scarica gli asset elencati in assets_wanted.txt in Assets_Raw/ e aggiorna CREDITS.md.

Uso (dalla cartella del progetto, con qualsiasi Python 3.9+, anche quello di Unreal):
    python tools/download_assets.py            scarica ciò che manca
    python tools/download_assets.py --dry-run  mostra cosa scaricherebbe

Formato di assets_wanted.txt, una riga per asset:
    fonte | id | categoria | nome
    polyhaven | metal_cabinet | Furniture | MetalCabinet
Fonti: polyhaven (modelli e texture), ambientcg (materiali), sketchfab (solo CC0 / CC-BY).
Il token Sketchfab si legge dalla variabile d'ambiente SKETCHFAB_TOKEN, mai dal codice.

Usa solo API ufficiali e link di download diretti. Gli asset già scaricati
(cartella con il file .done) vengono saltati.
"""

import argparse
import io
import json
import os
import sys
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RAW = ROOT / "Assets_Raw"
WANTED = ROOT / "assets_wanted.txt"
CREDITS = ROOT / "CREDITS.md"
USER_AGENT = "Ricovero-asset-downloader/1.0 (indie game, contact via GitHub tizioby/Ricovero)"

# Risoluzioni in ordine di preferenza: 4K per superfici grandi, 2K basta per i props.
POLYHAVEN_RES = ["4k", "2k", "1k"]
# Mappe PBR da Poly Haven: nome API -> suffisso nel nostro naming.
POLYHAVEN_MAPS = {
    "Diffuse": "D",
    "nor_gl": "N",
    "arm": "ORM",
    "Rough": "R",
    "AO": "AO",
    "Displacement": "H",
}
AMBIENTCG_RES = ["4K-JPG", "2K-JPG"]
SKETCHFAB_LICENSES = {"cc0": "CC0", "by": "CC-BY 4.0"}


def http_json(url, headers=None):
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT, **(headers or {})})
    with urllib.request.urlopen(request, timeout=60) as response:
        return json.load(response)


def http_download(url, dest, headers=None):
    dest.parent.mkdir(parents=True, exist_ok=True)
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT, **(headers or {})})
    with urllib.request.urlopen(request, timeout=300) as response, open(dest, "wb") as out:
        while chunk := response.read(1 << 20):
            out.write(chunk)
    return dest


def pick_resolution(available, preferred):
    for res in preferred:
        if res in available:
            return res
    return next(iter(available), None)


# --- Poly Haven ----------------------------------------------------------------

def download_polyhaven(asset_id, folder):
    info = http_json(f"https://api.polyhaven.com/info/{asset_id}")
    files = http_json(f"https://api.polyhaven.com/files/{asset_id}")
    authors = ", ".join(info.get("authors", {}).keys()) or "Poly Haven"

    # Il tipo dice cos'è l'asset (0 HDRI, 1 texture, 2 modello): anche le texture
    # offrono un glTF, ma è solo un piano di anteprima.
    if info.get("type") == 2:
        # Modello: glTF con le sue texture incluse.
        res = pick_resolution(files["gltf"], POLYHAVEN_RES)
        gltf = files["gltf"][res]["gltf"]
        http_download(gltf["url"], folder / Path(gltf["url"]).name)
        for rel_path, include in gltf.get("include", {}).items():
            http_download(include["url"], folder / rel_path)
        kind = f"modello glTF {res}"
    else:
        # Texture PBR.
        res = None
        for api_name, suffix in POLYHAVEN_MAPS.items():
            if api_name not in files:
                continue
            res = pick_resolution(files[api_name], POLYHAVEN_RES)
            formats = files[api_name][res]
            fmt = "png" if api_name == "Displacement" and "png" in formats else ("jpg" if "jpg" in formats else next(iter(formats)))
            url = formats[fmt]["url"]
            http_download(url, folder / f"{asset_id}_{suffix}.{fmt}")
        kind = f"texture PBR {res}"

    return {
        "title": info.get("name", asset_id),
        "author": authors,
        "link": f"https://polyhaven.com/a/{asset_id}",
        "license": "CC0",
        "kind": kind,
    }


# --- ambientCG -----------------------------------------------------------------

def download_ambientcg(asset_id, folder):
    data = http_json(f"https://ambientcg.com/api/v2/full_json?id={asset_id}&include=downloadData")
    assets = data.get("foundAssets", [])
    if not assets:
        raise RuntimeError("asset non trovato su ambientCG")
    downloads = assets[0]["downloadFolders"]["default"]["downloadFiletypeCategories"]["zip"]["downloads"]
    by_attr = {d["attribute"]: d for d in downloads}
    attr = pick_resolution(by_attr, AMBIENTCG_RES)
    archive = http_download(by_attr[attr]["downloadLink"], folder / f"{asset_id}_{attr}.zip")
    with zipfile.ZipFile(archive) as zf:
        zf.extractall(folder)
    archive.unlink()
    return {
        "title": assets[0].get("displayName", asset_id),
        "author": "ambientCG",
        "link": f"https://ambientcg.com/view?id={asset_id}",
        "license": "CC0",
        "kind": f"materiale PBR {attr}",
    }


# --- Sketchfab -----------------------------------------------------------------

def download_sketchfab(uid, folder):
    token = os.environ.get("SKETCHFAB_TOKEN")
    if not token:
        raise RuntimeError("SKETCHFAB_TOKEN non impostato")
    model = http_json(f"https://api.sketchfab.com/v3/models/{uid}")
    slug = (model.get("license") or {}).get("slug", "")
    if slug not in SKETCHFAB_LICENSES:
        raise RuntimeError(f"licenza '{slug}' non ammessa (solo CC0 e CC-BY)")
    if not model.get("isDownloadable"):
        raise RuntimeError("modello non scaricabile")

    links = http_json(f"https://api.sketchfab.com/v3/models/{uid}/download", headers={"Authorization": f"Token {token}"})
    if "glb" in links:
        http_download(links["glb"]["url"], folder / f"{folder.name}.glb")
        kind = "modello GLB"
    else:
        archive = http_download(links["gltf"]["url"], folder / f"{uid}.zip")
        with zipfile.ZipFile(archive) as zf:
            zf.extractall(folder)
        archive.unlink()
        kind = "modello glTF"

    user = model.get("user") or {}
    return {
        "title": model.get("name", uid),
        "author": user.get("displayName") or user.get("username", "?"),
        "link": model.get("viewerUrl", f"https://sketchfab.com/3d-models/{uid}"),
        "license": SKETCHFAB_LICENSES[slug],
        "kind": kind,
    }


SOURCES = {
    "polyhaven": download_polyhaven,
    "ambientcg": download_ambientcg,
    "sketchfab": download_sketchfab,
}


# --- Lista e crediti -----------------------------------------------------------

def read_wanted():
    entries = []
    for line_no, line in enumerate(WANTED.read_text(encoding="utf-8").splitlines(), 1):
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        parts = [p.strip() for p in line.split("|")]
        if len(parts) != 4 or parts[0] not in SOURCES:
            print(f"  riga {line_no} ignorata (formato: fonte | id | categoria | nome): {line}")
            continue
        entries.append(dict(zip(("source", "id", "category", "name"), parts)))
    return entries


def add_credit(entry, meta):
    text = CREDITS.read_text(encoding="utf-8") if CREDITS.exists() else ""
    if meta["link"] in text:
        return
    row = f"| {entry['name']} ({meta['title']}) | {meta['author']} | {entry['source']} | {meta['link']} | {meta['license']} |\n"
    if not text.endswith("\n"):
        text += "\n"
    CREDITS.write_text(text + row, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    entries = read_wanted()
    failures = 0
    for entry in entries:
        folder = RAW / entry["source"] / entry["category"] / entry["name"]
        done = folder / ".done"
        label = f"{entry['source']}:{entry['id']} -> {folder.relative_to(ROOT)}"
        if done.exists():
            print(f"  già presente  {label}")
            continue
        if args.dry_run:
            print(f"  da scaricare  {label}")
            continue
        print(f"  scarico       {label}")
        try:
            folder.mkdir(parents=True, exist_ok=True)
            meta = SOURCES[entry["source"]](entry["id"], folder)
            (folder / "asset.json").write_text(json.dumps({**entry, **meta}, indent=2, ensure_ascii=False), encoding="utf-8")
            add_credit(entry, meta)
            done.touch()
            print(f"    ok: {meta['kind']}, {meta['license']}, {meta['author']}")
        except Exception as error:  # un asset fallito non blocca gli altri
            failures += 1
            print(f"    ERRORE: {error}")

    print(f"Fatto. {len(entries)} asset in lista, {failures} errori.")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
