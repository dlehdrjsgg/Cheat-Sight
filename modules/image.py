import sys
import os
from PIL import Image, ImageDraw

OUTPUT_DIR = "output"

def generate_image(file_path):
    # 간단한 이미지 생성
    output_file = os.path.join(OUTPUT_DIR, os.path.basename(file_path) + "_image.png")
    img = Image.new("RGB", (200, 200), color="white")
    d = ImageDraw.Draw(img)
    d.text((10, 10), "Visualization of " + os.path.basename(file_path), fill="black")
    img.save(output_file)
    print(f"Image saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 image_gen.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    generate_image(file_path)
