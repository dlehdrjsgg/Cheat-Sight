import sys
import os

OUTPUT_DIR = "output"

def analyze_text(file_path):
    with open(file_path, 'r') as file:
        code = file.read()
    analysis = [f"Analyzing {file_path}"]
    analysis.append(f"Number of lines: {len(code.splitlines())}")
    analysis.append(f"Contains 'malloc': {'malloc' in code}")
    return "\n".join(analysis)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 text_analysis.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    result = analyze_text(file_path)
    output_file = os.path.join(OUTPUT_DIR, os.path.basename(file_path) + "_text.txt")
    with open(output_file, "w") as f:
        f.write(result)
    print(f"Text analysis saved to {output_file}")
