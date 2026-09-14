import os
import sys
import struct

XOR_KEY = b"GalityEngine2026"

def xor_data(data: bytes, key: bytes) -> bytes:
    key_len = len(key)
    return bytes([b ^ key[i % key_len] for i, b in enumerate(data)])

def pack_assets(assets_dir: str, output_pak: str):
    file_entries = []
    
    for root, _, files in os.walk(assets_dir):
        for file in files:
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, start=".").replace("\\", "/")
            file_entries.append((full_path, rel_path))

    print(f"[Gality Packer] Packing {len(file_entries)} assets into {output_pak}...")

    header_data = bytearray()
    header_data.extend(struct.pack("<I", len(file_entries)))

    blob_data = bytearray()
    current_offset = 0

    for full_path, rel_path in file_entries:
        with open(full_path, "rb") as f:
            raw_content = f.read()
        
        encrypted_content = xor_data(raw_content, XOR_KEY)
        size = len(encrypted_content)
        
        path_bytes = rel_path.encode("utf-8")
        header_data.extend(struct.pack("<H", len(path_bytes)))
        header_data.extend(path_bytes)
        header_data.extend(struct.pack("<I", current_offset))
        header_data.extend(struct.pack("<I", size))

        blob_data.extend(encrypted_content)
        current_offset += size

    with open(output_pak, "wb") as f_out:
        header_bytes = bytes(header_data)
        f_out.write(struct.pack("<I", len(header_bytes)))
        f_out.write(header_bytes)
        f_out.write(bytes(blob_data))

    print(f"[Gality Packer] Pack complete! Output: {output_pak}")

if __name__ == "__main__":
    assets_folder = sys.argv[1] if len(sys.argv) > 1 else "assets"
    pak_file = sys.argv[2] if len(sys.argv) > 2 else "data.pak"
    pack_assets(assets_folder, pak_file)