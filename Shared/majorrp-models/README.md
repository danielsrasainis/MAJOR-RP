# MAJOR RP built-in models (sealed)

The public repo never holds the raw .dff/.txd/.col files. Each `*.sealed` file here is a zip
(`models.txt` + model files) encrypted to `public.pem`. Only the GitHub Actions build can open them:
it has the matching private key in the repository secret `MAJORRP_MODELS_PRIVATE_KEY`.

At build time `utils/majorrp_pack_models.py` opens all sealed files, picks a new random key,
writes `MTA/models/models.mrp` (AES-256 encrypted pack) and the key header for the client.
The client decrypts the pack in memory after joining the server and replaces the models.

Add models: `python Shared/majorrp-models/seal.py Shared/majorrp-models/<name>.sealed models.txt <files...>`
`models.txt` lines: `<model id> <name>` (uses <name>.dff, and <name>.txd / <name>.col when present).

This stops normal players and model-ripping tools. It does not stop someone who dumps game memory.
