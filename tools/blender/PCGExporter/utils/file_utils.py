"""File utilities for PCG Exporter addon."""
import os


def ensure_folders(root_path):
    """Create all necessary export folders."""
    subfolders = [
        "Meshes",
        "LOD",
        "Collision",
        "SnapPoints",
        "Metadata",
        "Thumbnails"
    ]
    for folder in subfolders:
        path = os.path.join(root_path, folder)
        os.makedirs(path, exist_ok=True)
