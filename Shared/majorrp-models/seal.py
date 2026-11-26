# Seal model files for the MAJOR RP build: zip them and encrypt to public.pem (RSA-OAEP + AES-256-GCM).
# Usage: python seal.py <out.sealed> <models.txt> <file> [file ...]
import io, os, struct, sys, zipfile
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

out, files = sys.argv[1], sys.argv[2:]
pub = serialization.load_pem_public_key(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "public.pem"), "rb").read())
buf = io.BytesIO()
with zipfile.ZipFile(buf, "w", zipfile.ZIP_DEFLATED) as z:
    for f in files:
        z.write(f, os.path.basename(f))
key, nonce = AESGCM.generate_key(256), os.urandom(12)
wrapped = pub.encrypt(key, padding.OAEP(mgf=padding.MGF1(hashes.SHA256()), algorithm=hashes.SHA256(), label=None))
with open(out, "wb") as f:
    f.write(b"MRPS1" + struct.pack("<H", len(wrapped)) + wrapped + nonce + AESGCM(key).encrypt(nonce, buf.getvalue(), None))
print("sealed", len(files), "files ->", out)
