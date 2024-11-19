import sys
import os
import cv2
import numpy as np

OUTPUT_DIR = "output"

def generate_video(file_path):
    # 간단한 동영상 생성
    output_file = os.path.join(OUTPUT_DIR, os.path.basename(file_path) + "_video.avi")
    frame = np.zeros((200, 200, 3), dtype=np.uint8)
    fourcc = cv2.VideoWriter_fourcc(*"XVID")
    out = cv2.VideoWriter(output_file, fourcc, 1.0, (200, 200))

    for i in range(10):  # 10 프레임 생성
        frame[:] = (i * 25, i * 25, i * 25)  # 색상 변화
        out.write(frame)

    out.release()
    print(f"Video saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 video_gen.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    generate_video(file_path)
