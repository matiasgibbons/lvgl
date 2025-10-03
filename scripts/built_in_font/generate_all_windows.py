#!/usr/bin/env python3
"""
Script adaptado para Windows para regenerar todas las fuentes Montserrat con acentos latinos.
Uso: python generate_all_windows.py
"""

import os
import subprocess

def run_font_gen(size):
    """Genera una fuente de un tamaño específico"""
    print(f"\nGenerating {size} px")
    cmd = ["python", "built_in_font_gen.py", "--size", str(size), 
           "-o", f"lv_font_montserrat_{size}.c", "--bpp", "4"]
    result = subprocess.run(cmd, capture_output=False)
    return result.returncode == 0

# Lista de tamaños a generar
sizes = [8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48]

print("=" * 60)
print("Generando fuentes Montserrat con acentos latinos")
print("Esto puede tomar 10-15 minutos...")
print("=" * 60)

failed = []
for size in sizes:
    if not run_font_gen(size):
        failed.append(size)
        print(f"ERROR: Falló generación de {size}px")

# Generar fuentes especiales
print("\nGenerating 12 px subpx")
subprocess.run(["python", "built_in_font_gen.py", "--size", "12", 
                "-o", "lv_font_montserrat_12_subpx.c", "--bpp", "4", "--subpx"])

print("\nGenerating 28 px compressed")
subprocess.run(["python", "built_in_font_gen.py", "--size", "28", 
                "-o", "lv_font_montserrat_28_compressed.c", "--bpp", "4", "--compressed"])

# Mover archivos a ubicación final (Windows)
print("\nMoviendo archivos a src/font/...")
for file in os.listdir("."):
    if file.startswith("lv_font_montserrat_") and file.endswith(".c"):
        dest = os.path.join("..", "..", "src", "font", file)
        try:
            # En Windows, usamos replace que funciona cross-platform
            import shutil
            shutil.move(file, dest)
            print(f"  ✓ {file}")
        except Exception as e:
            print(f"  ✗ Error moviendo {file}: {e}")

print("\n" + "=" * 60)
if failed:
    print(f"ADVERTENCIA: Fallaron {len(failed)} fuentes: {failed}")
else:
    print("✓ Todas las fuentes generadas exitosamente!")
print("=" * 60)
